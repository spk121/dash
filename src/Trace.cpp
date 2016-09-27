#include "Trace.h"
#include "parser.h"

void Trace::trace(const char *fmt, ...)
{
	va_list va;

	if (debug != 1)
		return;
	va_start(va, fmt);
	(void)vfprintf(tracefile, fmt, va);
	va_end(va);
}

void Trace::tracev(const char *fmt, va_list va)
{
	if (debug != 1)
		return;
	(void)vfprintf(tracefile, fmt, va);
}

static string trstring(const string& in)
{
	string out{};
	out += "\"";
	for (const signed char c : in)
	{
		switch (c) {
		case '\n': 
			out += "\\n";
			break;
		case '\t':
			out += "\\t";
			break;
		case 'r':
			out += "\\r";
			break;
		case '"':
			out += "\"";
			break;
		case '\\':
			out += "\\";
			break;
		case CTLESC:
			out += "\\e";
			break;
		case CTLVAR:
			out += "\\v";
			break;
		case CTLBACKQ:
			out += "\\q";
			break;
		default:
			if (c >= ' ' && c <= '~')
				out += c;
			else {
				out += "\\";
				out += c >> 6 & 03;
				out += c >> 3 & 07;
				out += c & 07;
			}
			break;
		}
	}
	out += "\"";
	return out;
}


void
trargs(char **ap)
{
	if (debug != 1)
		return;
	for (const char* s : ap)
		tracefile << trstring(s);
		trstring(*ap++);
		if (*ap)
			putc(' ', tracefile);
		else
			putc('\n', tracefile);
	}
}


void
opentrace(void)
{
	char s[100];
#ifdef O_APPEND
	int flags;
#endif

	if (debug != 1) {
		if (tracefile)
			fflush(tracefile);
		/* leave open because libedit might be using it */
		return;
	}
#ifdef not_this_way
	{
		char *p;
		if ((p = getenv(homestr)) == NULL) {
			if (geteuid() == 0)
				p = "/";
			else
				p = "/tmp";
		}
		scopy(p, s);
		strcat(s, "/trace");
	}
#else
	scopy("./trace", s);
#endif /* not_this_way */
	if (tracefile) {
#ifndef __KLIBC__
		if (!freopen(s, "a", tracefile)) {
#else
		if (!(!fclose(tracefile) && (tracefile = fopen(s, "a")))) {
#endif /* __KLIBC__ */
			fprintf(stderr, "Can't re-open %s\n", s);
			debug = 0;
			return;
		}
		}
	else {
		if ((tracefile = fopen(s, "a")) == NULL) {
			fprintf(stderr, "Can't open %s\n", s);
			debug = 0;
			return;
		}
	}
#ifdef O_APPEND
	if ((flags = fcntl(fileno(tracefile), F_GETFL, 0)) >= 0)
		fcntl(fileno(tracefile), F_SETFL, flags | O_APPEND);
#endif
#if !defined(__KLIBC__) && !defined(_MSC_VER)
	setlinebuf(tracefile);
#endif /* __KLIBC__ */
	fputs("\nTracing started.\n", tracefile);
	}
#endif /* DEBUG */
