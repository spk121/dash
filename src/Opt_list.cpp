#include "Opt_list.h"
#include "output.h"
#include "error.h"

Opt_list optlist{};

const int& Opt_list::operator[](const string& name) const
{
	int i = 0;
	for (const auto& N : optnames)
	{
		if (name == N)
			return optlist[i];
		i++;
	}
	throw out_of_range(name);
}

const int& Opt_list::operator[](const char letter) const
{
	int i = 0;
	for (const auto& C : optletters)
	{
		if (letter == C)
			return optlist[i];
		i++;
	}
	throw out_of_range(string{ letter });
}

int& Opt_list::operator[](const string& name)
{
	int i = 0;
	for (const auto& N : optnames)
	{
		if (name == N)
			return optlist[i];
		i++;
	}
	throw out_of_range(name);
}

int& Opt_list::operator[](const char letter)
{
	int i = 0;
	for (const auto& C : optletters)
	{
		if (letter == C)
			return optlist[i];
		i++;
	}
	throw out_of_range(string{ letter });
}

void Opt_list::set_all_to_unspecified()
{
	for (auto& C : optlist)
		C = Opt_list::UNSPECIFIED;
}

void Opt_list::set_unspecified_to_disabled()
{
	for (auto& C : optlist)
		if (C == Opt_list::UNSPECIFIED)
		C = Opt_list::DISABLED;

}

void Opt_list::display_options()
{
	out1str("Current option settings\n");
	for (int i = 0; i < NOPTS; i++)
		out1fmt("%-16s%s\n", optnames[i],
			optlist[i] ? "on" : "off");
}

void Opt_list::output_options()
{
	for (int i = 0; i < NOPTS; i++)
		out1fmt("set %s %s\n",
			optlist[i] ? "-o" : "+o",
			optnames[i]);
}

void Opt_list::minus_o(char *name, int val)
{
	if (name == NULL) {
		if (val)
			display_options();
		else
			output_options();
	}
	else {
		for (int i = 0; i < NOPTS; i++)
			if (optnames[i] == name) {
				optlist[i] = val;
				return;
			}
		sh_error("Illegal option -o %s", name);
	}
}

void Opt_list::setoption(int flag, int val)
{
	int i;

	for (int i = 0; i < NOPTS; i++)
		if (optletters[i] == flag) {
			optlist[i] = val;
			if (val) {
				/* #%$ hack for ksh semantics */
				if (flag == 'V')					
					operator[]("emacs") = 0;
				else if (flag == 'E')
					operator[]("vi") = 0;
			}
			return;
		}
	sh_error("Illegal option -%c", flag);
	/* NOTREACHED */
}

string Opt_list::makestr()
{
	string p{};
	for (int i = NOPTS - 1; i >= 0; i--) {
		if (optlist[i]) {
			p += optletters[i];
		}
	}
	return p;
}

string Opt_list::serialize()
{
	string out{};

	for (auto C : optlist)
		out.append(1, static_cast<char>(C) + '0');

	return out;

}

void Opt_list::deserialize(string&& in)
{
	int i = 0;
	for (auto C : in)
	{
		optlist[i] = static_cast<int>(in[i] - '0');
		i++;
	}
}
