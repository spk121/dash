#include "Optlist.h"
#include <array>
#include "output.h"
#include "error.h"
using namespace std;

constexpr int NOPTS = 17;

const array<string, NOPTS> optnames{ {
	"errexit",
	"noglob",
	"ignoreeof",
	"interactive",
	"monitor",
	"noexec",
	"stdin",
	"xtrace",
	"verbose",
	"vi",
	"emacs",
	"noclobber",
	"allexport",
	"notify",
	"nounset",
	"nolog",
	"debug", }
};

constexpr array<char, NOPTS> optletters{ {
	'e',
	'f',
	'I',
	'i',
	'm',
	'n',
	's',
	'x',
	'v',
	'V',
	'E',
	'C',
	'a',
	'b',
	'u',
	'\0',
	'\0',
} };

Optlist::Optlist()
	: _flags{}, flags{}, abbrev{}, is_login_shell{ false }, arg0{}
{
	int i = 0;
	for (auto str : optnames) {
		_flags.push_back(OPT::UNSPECIFIED);
		flags[str] = &_flags[i++];
	}

	i = 0;
	for (auto c : optletters)
		abbrev[c] = &_flags[i++];

}


Optlist::~Optlist()
{
}

bool Optlist::set(char c, OPT val)
{
	try {
		*(abbrev.at(c)) = val;
	}
	catch (out_of_range&) {
		return false;
	}
	return true;
}

bool Optlist::set(string s, OPT val)
{
	try {
		*(flags.at(s)) = val;
	}
	catch (out_of_range&) {
		return false;
	}

	return true;
}

OPT Optlist::get(string s) const
{
	try {
		return *(flags.at(s));
	}
	catch (out_of_range&) {
		return OPT::UNSPECIFIED;
	}
}

OPT Optlist::get(char c) const
{
	try {
		return *(abbrev.at(c));
	}
	catch (out_of_range&) {
		return OPT::UNSPECIFIED;
	}
}

vector<string> Optlist::process_argv(vector<string> argv)
{
	enum class EDITOR { EMACS, VI } last_editor;

	// The first argv is the command's name
	arg0 = argv[0];
	if (arg0 == "-")
		is_login_shell = true;

	auto arg_iter = argv.begin() + 1;
	for (; arg_iter != argv.end(); ++arg_iter)
	{
		OPT val = OPT::UNSPECIFIED;
		string arg_cur = arg_iter[0];

		if (arg_cur == "-" || arg_cur == "--") {
			// Dash or double-dash forces the end of option procesing.
			++arg_iter;
			break;
		}
		else if (arg_cur.size() < 2 || (arg_cur.front() != '-' && arg_cur.front() != '+')) {
			// If a argument is a single letter or doesn't begin
			// with +/-, it is not an option.
			break;
		}

		if (arg_cur.front() == '-')
			val = OPT::ACTIVE;
		else
			val = OPT::INACTIVE;

		// Arguments that begin with +/- and are more than one character
		// are options.
		for (auto char_iter = arg_cur.begin() + 1; char_iter != arg_cur.end(); ++char_iter)
		{
			char char_cur = char_iter[0];
			if (char_cur == 'c')
				has_command_string = true;
			else if (char_cur == 'l')
				is_login_shell = true;
			else if (char_cur == 'o') {
				// When we see a -o or +o, it means that the next
				// argument is a long option name.  But, if there is no next argument,
				// it means that we're printing out the option table.
				if (arg_iter + 1 != argv.end()) {
					if (!set(arg_iter[1], val))
						sh_error("Illegal option -o %s", arg_iter[1]);
					if (arg_iter[1] == "emacs")
						last_editor = EDITOR::EMACS;
					if (arg_iter[1] == "vi")
						last_editor = EDITOR::VI;
					++arg_iter;
				}
				else {
					if (val == OPT::ACTIVE)
						display_options_format_1();
					else
						display_options_format_2();

				}
			}
			else {
				if (!set(char_cur, val))
					sh_error("Illegal option -%c", char_cur);
				if (char_cur == 'V')
					last_editor = EDITOR::VI;
				if (char_cur == 'E')
					last_editor = EDITOR::EMACS;
			}
		}
	}

	if (get("vi") == OPT::ACTIVE && get("emacs") == OPT::ACTIVE) {
		if (last_editor == EDITOR::EMACS)
			set("vi", OPT::INACTIVE);
		else
			set("emacs", OPT::INACTIVE);
	}

	if (has_command_string && (arg_iter == argv.end()))
		sh_error("-c requires an argument");

	if (get("interactive") == OPT::UNSPECIFIED && get("stdin") == OPT::ACTIVE
		&& isatty(0) && isatty(1))
		set("interactive", OPT::ACTIVE);
	if (get("monitor") == OPT::UNSPECIFIED)
		set("monitor", get("interactive"));

	set_unspecified_flags_to_active();

	// The unprocess arguments are either file names or is a command string
	return argv.erase(arg_iter, argv.end());
}

void Optlist::display_options_format_1() const
{
	out1str("Current option settings\n");
	for (auto f : flags) {
		out1fmt("%-16s%s\n", f.first, *(f.second) == OPT::ACTIVE ? "on" : "off");
	}
}

void Optlist::display_options_format_2() const
{
	out1str("Current option settings\n");
	for (auto f : flags) {
		out1fmt("set %s %s\n", *(f.second) == OPT::ACTIVE ? "-o" : "+o", f.first);
	}
}

void
Optlist::set_unspecified_flags_to_active()
{
	replace(_flags.begin(), _flags.end(), OPT::UNSPECIFIED, OPT::ACTIVE);
}

