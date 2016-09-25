#pragma once

#include <array>
#include <string>
using namespace std;

struct Opt_list
{
	static const size_t NOPTS = 17;
	static array<string, NOPTS> optnames;
	static array<char, NOPTS> optletters;
	enum {DISABLED = 0, ENABLED = 1, UNSPECIFIED = 2};

	array<int, NOPTS> optlist;

	Opt_list() : optlist{} {};
	int& operator[](const string& name);
	int& operator[](const char letter);
	const int& operator[](const string& name) const;
	const int& operator[](const char letter) const;
	void set_all_to_unspecified();
	void set_unspecified_to_disabled();

	void display_options();
	void output_options();
	void minus_o(char *name, int val);
	void setoption(int flag, int val);
	string makestr();
	string serialize();
	void deserialize(string&& in);
};

array<string, Opt_list::NOPTS> Opt_list::optnames {
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
	"debug",
};

array<char, Opt_list::NOPTS> Opt_list::optletters {
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
	0,
	0,
};

extern Opt_list optlist;