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

#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
// #include <unistd.h>
#include <fcntl.h>

#include "shell.h"
#include "main.h"
#include "options.h"
#include "output.h"
#include "parser.h"
#include "nodes.h"
#include "expand.h"
#include "eval.h"
#include "jobs.h"
#include "input.h"
#include "trap.h"
#include "var.h"
#include "show.h"
#include "memalloc.h"
#include "error.h"
#include "mystring.h"
#include "exec.h"
#include "cd.h"
#include "redir.h"
#include "Opt_list.h"

sys_process_id_t rootpid;
int shlvl;
#ifdef __GLIBC__
int *dash_errno;
#endif

static void read_profile(const char *);
static char *find_dot_file(char *);
static int cmdloop(int);
int main(int, char **);

/*
 * Main routine.  We initialize things, parse the arguments, execute
 * profiles if we're a login shell, and then call cmdloop to execute
 * commands.  The setjmp call sets up the location to jump to when an
 * exception occurs.  When an exception occurs the variable "state"
 * is used to figure out how far we had gotten.
 */

int
main(int argc, char **argv)
{
	char *shinit;
	volatile int state;
	struct jmploc jmploc;
	struct stackmark smark;
	int login;

#ifdef __GLIBC__
	dash_errno = __errno_location();
#endif

	state = 0;
	if (unlikely(setjmp(jmploc.loc))) {
		int e;
		int s;

		/* Module reset */
		eval_reset();
		expand_reset();
		input_reset();
		redir_reset();
		var_reset();

		e = exception_;

		s = state;
		if (e == EXEXIT || s == 0 || optlist["interactive"] == Opt_list::DISABLED || shlvl)
			//exitshell();
			;

		if (e == EXINT) {
			out2c('\n');
#ifdef FLUSHERR
			flushout(out2);
#endif
		}
		popstackmark(&smark);
		forceinton();				/* enable interrupts */
		if (s == 1)
			goto state1;
		else if (s == 2)
			goto state2;
		else if (s == 3)
			goto state3;
		else
			goto state4;
	}
	handler = &jmploc;
#ifdef DEBUG
	opentrace();
	trputs("Shell args:  ");  trargs(argv);
#endif
	rootpid = sys_get_process_id();

	/* Module initialization */
	input_init();
	trap_init();
	var_init();

	setstackmark(&smark);
	login = procargs(argc, argv);
	if (login) {
		state = 1;
		read_profile("/etc/profile");
state1:
		state = 2;
		read_profile("$HOME/.profile");
	}
state2:
	state = 3;
	if (
#ifndef linux
		getuid() == geteuid() && getgid() == getegid() &&
#endif
		optlist["interactive"]
	) {
		if ((shinit = lookupvar("ENV")) != NULL && *shinit != '\0') {
			read_profile(shinit);
		}
	}
	popstackmark(&smark);
state3:
	state = 4;
	if (minusc)
		evalstring(minusc, optlist["stdin"] ? 0 : EV_EXIT);

	if (optlist["stdin"] || minusc == NULL) {
state4:	/* XXX ??? - why isn't this before the "if" statement */
		cmdloop(1);
	}
	exitshell();
	/* NOTREACHED */
}


/*
 * Read and execute commands.  "Top" is nonzero for the top level command
 * loop; it turns on prompting if the shell is interactive.
 */

static int
cmdloop(int top)
{
	union node *n;
	struct stackmark smark;
	int inter;
	int status = 0;
	int numeof = 0;

	TRACE(("cmdloop(%d) called\n", top));
	for (;;) {
		int skip;

		setstackmark(&smark);
		if (jobctl)
			showjobs(out2, SHOW_CHANGED);
		inter = 0;
		if (optlist["interactive"] && top) {
			inter++;
		}
		n = parsecmd(inter);
		/* showtree(n); DEBUG */
		if (n == NEOF) {
			if (!top || numeof >= 50)
				break;
			if (!stoppedjobs()) {
				if (!optlist["ignoreeof"])
					break;
				out2str("\nUse \"exit\" to leave shell.\n");
			}
			numeof++;
		} else if (optlist["noexec"] == 0) {
			job_warning = (job_warning == 2) ? 1 : 0;
			numeof = 0;
			evaltree(n, 0);
			status = exitstatus;
		}
		popstackmark(&smark);

		skip = evalskip;
		if (skip) {
			evalskip &= ~(SKIPFUNC | SKIPFUNCDEF);
			break;
		}
	}

	return status;
}



/*
 * Read /etc/profile or .profile.  Return on error.
 */

static void
read_profile(const char *name)
{
	name = expandstr(name);
	if (setinputfile(name, INPUT_PUSH_FILE | INPUT_NOFILE_OK) < 0)
		return;

	cmdloop(0);
	popfile();
}



/*
 * Read a file containing shell functions.
 */

void
readcmdfile(char *name)
{
	setinputfile(name, INPUT_PUSH_FILE);
	cmdloop(0);
	popfile();
}



/*
 * Take commands from a file.  To be compatible we should do a path
 * search for the file, which is necessary to find sub-commands.
 */


static char *
find_dot_file(char *basename)
{
	char *fullname;
	const char *path = pathval();
	struct stat statb;

	/* don't try this for absolute or relative paths */
	if (strchr(basename, '/'))
		return basename;

	while ((fullname = padvance(&path, basename)) != NULL) {
		if ((stat(fullname, &statb) == 0) && S_ISREG(statb.st_mode)) {
			/*
			 * Don't bother freeing here, since it will
			 * be freed by the caller.
			 */
			return fullname;
		}
		stunalloc(fullname);
	}

	/* not found in the PATH */
	sh_error("%s: not found", basename);
	/* NOTREACHED */
}

int
dotcmd(int argc, char **argv)
{
	int status = 0;

	nextopt(nullstr);
	argv = argptr;

	if (*argv) {
		char *fullname;

		fullname = find_dot_file(*argv);
		setinputfile(fullname, INPUT_PUSH_FILE);
		commandname = fullname;
		status = cmdloop(0);
		popfile();
	}

	return status;
}


int
exitcmd(int argc, char **argv)
{
	if (stoppedjobs())
		return 0;

	if (argc > 1) {
		int status = number(argv[1]);

		exitstatus = status;
		if (savestatus >= 0)
			savestatus = status;
	}

	exraise(EXEXIT);
	/* NOTREACHED */
}
