/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * Copyright (c) 1997-2005
 *	Herbert Xu <herbert@gondor.apana.org.au>.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Kenneth Almquist.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <string>
#include <vector>
using namespace std;

#include <signal.h>
// #include <unistd.h>
#include <stdlib.h>

#include "shell.h"
#define DEFINE_OPTIONS
#include "options.h"
#undef DEFINE_OPTIONS
#include "nodes.h"	/* for other header files */
#include "eval.h"
#include "jobs.h"
#include "input.h"
#include "output.h"
#include "trap.h"
#include "var.h"
#include "memalloc.h"
#include "error.h"
#include "mystring.h"
#ifndef SMALL
#include "myhistedit.h"
#endif
#include "show.h"
#include "Opt_list.h"

char *arg0;			/* value of $0 */
struct shparam shellparam;	/* current positional parameters */
vector<string> argptrv{};
// char **argptr;			/* argument list for builtin commands */
// char *optionarg;		/* set by nextopt (like getopt) */
// char *optptr;			/* used by nextopt */
Opt_args opt_args{};

char *minusc;			/* argument to -c option */

static int options(int);
static void minus_o(char *, int);
static void setoption(int, int);
static int getopts(char *, char *, char **);


/*
 * Process the shell command line arguments.
 */

int
procargs(int argc, char **argv)
{
	int i;
	const char *xminusc;
	char **xargv;
	int login;
	int xargv_len = 0;

	xargv = argv;
	login = xargv[0] && xargv[0][0] == '-';
	arg0 = xargv[0];
	if (argc > 0)
		xargv++;

	optlist.set_all_to_unspecified();

	while (xargv[xargv_len] != nullptr)
		xargv_len++;
	vector<string>(xargv, xargv + xargv_len).swap(argptrv);

	argptr = xargv;
	login |= options(1);
	xargv = argptr;
	xminusc = minusc;
	if (*xargv == NULL) {
		if (xminusc)
			sh_error("-c requires an argument");
		optlist["stdin"] = 1;
	}
	if (optlist["interactive"] == Opt_list::UNSPECIFIED && optlist["stdin"] == Opt_list::ENABLED && isatty(0) && isatty(1))
		optlist["interactive"] = Opt_list::ENABLED;
	if (optlist["monitor"] == Opt_list::UNSPECIFIED)
		optlist["monitor"] = optlist["interactive"];
	optlist.set_unspecified_to_disabled();

#if DEBUG == 2
	optlist["debug"] = Opt_list::ENABLED;
#endif
	/* POSIX 1003.2: first arg after -c cmd is $0, remainder $1... */
	if (xminusc) {
		minusc = *xargv++;
		if (*xargv)
			goto setarg0;
	} else if (!optlist["stdin"]) {
		setinputfile(*xargv, 0);
setarg0:
		arg0 = *xargv++;
		commandname = arg0;
	}

	shellparam.p = xargv;
	shellparam.optind = 1;
	shellparam.optoff = -1;
	/* assert(shellparam.malloc == 0 && shellparam.nparam == 0); */
	while (*xargv) {
		shellparam.nparam++;
		xargv++;
	}
	optschanged();

	return login;
}


void
optschanged(void)
{
#ifdef DEBUG
	opentrace();
#endif
	// setinteractive(optlist["interactive"]);
#ifndef SMALL
	// histedit();
#endif
#ifndef _MSC_VER
	setjobctl(optlist["monitor"]);
#endif
}

/*
 * Process shell options.  The global variable argptr contains a pointer
 * to the argument list; we advance it past the options.
 */

static int
options(int cmdline)
{
	char *p;
	int val;
	int c;
	int login = 0;

	if (cmdline)
		minusc = NULL;
	while ((p = *argptr) != NULL) {
		argptr++;
		if ((c = *p++) == '-') {
			val = 1;
                        if (p[0] == '\0' || (p[0] == '-' && p[1] == '\0')) {
                                if (!cmdline) {
                                        /* "-" means turn off -x and -v */
                                        if (p[0] == '\0')
                                                optlist["xtrace"] = optlist["verbose"] = 0;
                                        /* "--" means reset params */
                                        else if (*argptr == NULL)
						setparam(argptr);
                                }
				break;	  /* "-" or  "--" terminates options */
			}
		} else if (c == '+') {
			val = 0;
		} else {
			argptr--;
			break;
		}
		while ((c = *p++) != '\0') {
			if (c == 'c' && cmdline) {
				minusc = p;	/* command is after shell args*/
			} else if (c == 'l' && cmdline) {
				login = 1;
			} else if (c == 'o') {
				minus_o(*argptr, val);
				if (*argptr)
					argptr++;
			} else {
				setoption(c, val);
			}
		}
	}

	return login;
}

#if 0
static void
minus_o(char *name, int val)
{
	int i;

	if (name == NULL) {
		if (val) {
			out1str("Current option settings\n");
			for (i = 0; i < NOPTS; i++)
				out1fmt("%-16s%s\n", optnames[i],
					optlist[i] ? "on" : "off");
		} else {
			for (i = 0; i < NOPTS; i++)
				out1fmt("set %s %s\n",
					optlist[i] ? "-o" : "+o",
					optnames[i]);

		}
	} else {
		for (i = 0; i < NOPTS; i++)
			if (strequal(name, optnames[i])) {
				optlist[i] = val;
				return;
			}
		sh_error("Illegal option -o %s", name);
	}
}


static void
setoption(int flag, int val)
{
	int i;

	for (i = 0; i < NOPTS; i++)
		if (optletters[i] == flag) {
			optlist[i] = val;
			if (val) {
				/* #%$ hack for ksh semantics */
				if (flag == 'V')
					optlist["emacs"] = 0;
				else if (flag == 'E')
					optlist["vi"] = 0;
			}
			return;
		}
	sh_error("Illegal option -%c", flag);
	/* NOTREACHED */
}
#endif


/*
 * Set the shell parameters.
 */

void
setparam(char **argv)
{
	char **newparam;
	char **ap;
	int nparam;

	for (nparam = 0 ; argv[nparam] ; nparam++);
	ap = newparam = (char **)ckmalloc((nparam + 1) * sizeof *ap);
	while (*argv) {
		*ap++ = savestr(*argv++);
	}
	*ap = NULL;
	freeparam(&shellparam);
	shellparam.malloc = 1;
	shellparam.nparam = nparam;
	shellparam.p = newparam;
	shellparam.optind = 1;
	shellparam.optoff = -1;
}


/*
 * Free the list of positional parameters.
 */

void
freeparam(volatile struct shparam *param)
{
	char **ap;

	if (param->malloc) {
		for (ap = param->p ; *ap ; ap++)
			ckfree(*ap);
		ckfree((char*)(param->p));
	}
}



/*
 * The shift builtin command.
 */

int
shiftcmd(int argc, char **argv)
{
	int n;
	char **ap1, **ap2;

	n = 1;
	if (argc > 1)
		n = number(argv[1]);
	if (n > shellparam.nparam)
		sh_error("can't shift that many");
	intoff();
	shellparam.nparam -= n;
	for (ap1 = shellparam.p ; --n >= 0 ; ap1++) {
		if (shellparam.malloc)
			ckfree(*ap1);
	}
	ap2 = shellparam.p;
	while ((*ap2++ = *ap1++) != NULL);
	shellparam.optind = 1;
	shellparam.optoff = -1;
	inton();
	return 0;
}



/*
 * The set command builtin.
 */

int
setcmd(int argc, char **argv)
{
	if (argc == 1)
		return showvars(nullstr, 0, VUNSET);
	intoff();
	options(0);
	optschanged();
	if (*argptr != NULL) {
		setparam(argptr);
	}
	inton();
	return 0;
}


void
getoptsreset(const char *value)
{
	shellparam.optind = number(value);
	if (shellparam.optind == 0)
		shellparam.optind = 1;
	shellparam.optoff = -1;
}

/*
 * The getopts builtin.  Shellparam.optnext points to the next argument
 * to be processed.  Shellparam.optptr points to the next character to
 * be processed in the current argument.  If shellparam.optnext is NULL,
 * then it's the first time getopts has been called.
 */

int
getoptscmd(int argc, char **argv)
{
	char **optbase;

	if (argc < 3)
		sh_error("Usage: getopts optstring var [arg]");
	else if (argc == 3) {
		optbase = shellparam.p;
		if ((unsigned)shellparam.optind > shellparam.nparam + 1) {
			shellparam.optind = 1;
			shellparam.optoff = -1;
		}
	}
	else {
		optbase = &argv[3];
		if ((unsigned)shellparam.optind > argc - 2) {
			shellparam.optind = 1;
			shellparam.optoff = -1;
		}
	}

	return getopts(argv[1], argv[2], optbase);
}

static int
getopts(char *optstr, char *optvar, char **optfirst)
{
	char *p, *q;
	char c = '?';
	int done = 0;
	char s[2];
	char **optnext;
	int ind = shellparam.optind;
	int off = shellparam.optoff;

	shellparam.optind = -1;
	optnext = optfirst + ind - 1;

	if (ind <= 1 || off < 0 || strlen(optnext[-1]) < off)
		p = NULL;
	else
		p = optnext[-1] + off;
	if (p == NULL || *p == '\0') {
		/* Current word is done, advance */
		p = *optnext;
		if (p == NULL || *p != '-' || *++p == '\0') {
atend:
			p = NULL;
			done = 1;
			goto out;
		}
		optnext++;
		if (p[0] == '-' && p[1] == '\0')	/* check for "--" */
			goto atend;
	}

	c = *p++;
	for (q = optstr; *q != c; ) {
		if (*q == '\0') {
			if (optstr[0] == ':') {
				s[0] = c;
				s[1] = '\0';
				setvar("OPTARG", s, 0);
			} else {
				outfmt(&errout, "Illegal option -%c\n", c);
				(void) unsetvar("OPTARG");
			}
			c = '?';
			goto out;
		}
		if (*++q == ':')
			q++;
	}

	if (*++q == ':') {
		if (*p == '\0' && (p = *optnext) == NULL) {
			if (optstr[0] == ':') {
				s[0] = c;
				s[1] = '\0';
				setvar("OPTARG", s, 0);
				c = ':';
			} else {
				outfmt(&errout, "No arg for -%c option\n", c);
				(void) unsetvar("OPTARG");
				c = '?';
			}
			goto out;
		}

		if (p == *optnext)
			optnext++;
		setvar("OPTARG", p, 0);
		p = NULL;
	} else
		setvar("OPTARG", nullstr, 0);

out:
	ind = optnext - optfirst + 1;
	setvarint("OPTIND", ind, VNOFUNC);
	s[0] = c;
	s[1] = '\0';
	setvar(optvar, s, 0);

	shellparam.optoff = p ? p - *(optnext - 1) : -1;
	shellparam.optind = ind;

	return done;
}

/*
 * XXX - should get rid of.  have all builtins use getopt(3).  the
 * library getopt must have the BSD extension static variable "optreset"
 * otherwise it can't be used within the shell safely.
 *
 * Standard option processing (a la getopt) for builtin routines.  The
 * only argument that is passed to nextopt is the option string; the
 * other arguments are unnecessary.  It return the character, or '\0' on
 * end of input.
 */


/* Checks the current string in the list of strings stored in argptr looking
   for an option that matches one of the options in OPTSTRING.
   All options begin with '-' followed by a single letter option
   from optstring.  If the option in optstring has a ':', 
   */

// externals: optptr, argptr, 
#if 0
int
nextopt(string const& optstring)
// nextopt(const char * optstring)
{
	char *p;
	auto q = optstring.cbegin();
	char c;

	if ((p = optptr) == NULL || *p == '\0') {
		p = *argptr;
		if (p == NULL || *p != '-' || *++p == '\0')
			return '\0';
		argptr++;
		if (p[0] == '-' && p[1] == '\0')	/* check for "--" */
			return '\0';
	}
	c = *p++;
	for ( ; *q != c ; ) {
		if (*q == '\0')
			sh_error("Illegal option -%c", c);
		if (*++q == ':')
			q++;
	}
	if (*++q == ':') {
		if (*p == '\0' && (p = *argptr++) == NULL)
			sh_error("No arg for -%c option", c);
		optionarg = p;
		p = NULL;
	}
	
	optptr = p;
	return c;
}
#endif

