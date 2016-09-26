#pragma once

#include <fstream>


/*
* Debugging stuff.
*/

class Trace {
private:
	std::ofstream tracefile;
	bool debug;
public:
	enable() { debug = true; };
	disable() { debug = false; };
	template<typename T>
	void trput(const T& obj) { if (!debug) tracefile << obj; } 

	trace(const char *fmt, ...);
	tracev(const char *fmt, va_list va);
	void trputs(const char *s);
	static void trstring(char *s);
	void trargs(char **ap);
	void opentrace(void);
};
