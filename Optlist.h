#pragma once

#include <string>
#include <map>
#include <vector>
#include <algorithm>

using namespace std;

enum class OPT {ACTIVE, INACTIVE, UNSPECIFIED};

class Optlist
{
public:
	bool is_login_shell;
	bool has_command_string;
	string arg0;

	Optlist();
	~Optlist();

	bool set(char c, OPT val);
	bool set(string s, OPT val);
	OPT Optlist::get(string s) const;
	OPT Optlist::get(char c) const;
	vector<string> process_argv(vector<string>);

private:
	vector<OPT> _flags;
	map <string, OPT*> flags;
	map <char, OPT*> abbrev;

	void display_options_format_1() const;
	void display_options_format_2() const;
	void set_unspecified_flags_to_active();
};

