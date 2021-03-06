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

#include <sys/types.h>
#include <sys/stat.h>
// #include <sys/param.h>	/* PIPE_BUF */
#include <signal.h>
#include <string.h>
#include <fcntl.h>
// #include <unistd.h>
#include <stdlib.h>

/*
 * Code for dealing with input/output redirection.
 */
#include "system.h"
#include "main.h"
#include "shell.h"
#include "nodes.h"
#include "jobs.h"
#include "options.h"
#include "expand.h"
#include "redir.h"
#include "output.h"
#include "memalloc.h"
#include "error.h"
#include "Opt_list.h"


#define REALLY_CLOSED -3	/* fd that was closed and still is */
#define EMPTY -2		/* marks an unused slot in redirtab */
#define CLOSED -1		/* fd opened for redir needs to be closed */

#ifndef PIPE_BUF
# define PIPESIZE 4096		/* amount of buffering in a pipe */
#else
# define PIPESIZE PIPE_BUF
#endif


struct redirtab *redirlist;

static int openredirect(union node *);
static void dupredirect(union node *, int);
static int openhere(union node *);


/*
 * Process a list of redirection commands.  If the REDIR_PUSH flag is set,
 * old file descriptors are stashed away so that the redirection can be
 * undone by calling popredir.  If the REDIR_BACKQ flag is set, then the
 * standard output, and the standard error if it becomes a duplicate of
 * stdout, is saved in memory.
 */

void
redirect(union node *redir, int flags)
{
	union node *n;
	struct redirtab *sv;
	int i;
	int fd;
	int newfd;
	int *p;

	if (!redir)
		return;
	sv = NULL;
	intoff();
	if (likely(flags & REDIR_PUSH))
		sv = redirlist;
	n = redir;
	do {
		newfd = openredirect(n);
		if (newfd < -1)
			continue;

		fd = n->nfile.fd;

		if (sv) {
			p = &sv->renamed[fd];
			i = *p;

			if (likely(i == EMPTY)) {
				i = CLOSED;
				if (fd != newfd) {
					i = savefd(fd, fd);
					fd = -1;
				}
			}

			if (i == newfd)
				/* Can only happen if i == newfd == CLOSED */
				i = REALLY_CLOSED;

			*p = i;
		}

		if (fd == newfd)
			continue;

		dupredirect(n, newfd);
	} while ((n = n->nfile.next));
	inton();
	if (flags & REDIR_SAVEFD2 && sv->renamed[2] >= 0)
		preverrout.fd = sv->renamed[2];
}


static int
openredirect(union node *redir)
{
	struct stat64 sb;
	char *fname;
	int f;

	switch (redir->nfile.type) {
	case NFROM:
		fname = redir->nfile.expfname;
		if ((f = open64(fname, O_RDONLY)) < 0)
			goto eopen;
		break;
	case NFROMTO:
		fname = redir->nfile.expfname;
		if ((f = open64(fname, O_RDWR|O_CREAT, 0666)) < 0)
			goto ecreate;
		break;
	case NTO:
		/* Take care of noclobber mode. */
		if (optlist["noclobber"]) {
			fname = redir->nfile.expfname;
			if (stat64(fname, &sb) < 0) {
				if ((f = open64(fname, O_WRONLY|O_CREAT|O_EXCL, 0666)) < 0)
					goto ecreate;
			} else if (!S_ISREG(sb.st_mode)) {
				if ((f = open64(fname, O_WRONLY, 0666)) < 0)
					goto ecreate;
				if (fstat64(f, &sb) < 0 && S_ISREG(sb.st_mode)) {
					close(f);
					errno = EEXIST;
					goto ecreate;
				}
			} else {
				errno = EEXIST;
				goto ecreate;
			}
			break;
		}
		/* FALLTHROUGH */
	case NCLOBBER:
		fname = redir->nfile.expfname;
		if ((f = open64(fname, O_WRONLY|O_CREAT|O_TRUNC, 0666)) < 0)
			goto ecreate;
		break;
	case NAPPEND:
		fname = redir->nfile.expfname;
		if ((f = open64(fname, O_WRONLY|O_CREAT|O_APPEND, 0666)) < 0)
			goto ecreate;
		break;
	case NTOFD:
	case NFROMFD:
		f = redir->ndup.dupfd;
		if (f == redir->nfile.fd)
			f = -2;
		break;
	default:
#ifdef DEBUG
		abort();
#endif
		/* Fall through to eliminate warning. */
	case NHERE:
	case NXHERE:
		f = openhere(redir);
		break;
	}

	return f;
ecreate:
	sh_error("cannot create %s: %s", fname, errmsg(errno, E_CREAT));
eopen:
	sh_error("cannot open %s: %s", fname, errmsg(errno, E_OPEN));
}


static void
dupredirect(union node *redir, int f)
{
	int fd = redir->nfile.fd;
	int err = 0;

	if (redir->nfile.type == NTOFD || redir->nfile.type == NFROMFD) {
		/* if not ">&-" */
		if (f >= 0) {
			if (dup2(f, fd) < 0) {
				err = errno;
				goto err;
			}
			return;
		}
		f = fd;
	} else if (dup2(f, fd) < 0)
		err = errno;

	close(f);
	if (err < 0)
		goto err;

	return;

err:
	sh_error("%d: %s", f, strerror(err));
}


/*
 * Handle here documents.  Normally we fork off a process to write the
 * data to a pipe.  If the document is short, we can stuff the data in
 * the pipe without forking.
 */

static int
openhere(union node *redir)
{
	char *p;
	int pip[2];
	size_t len = 0;

	if (pipe(pip) < 0)
		sh_error("Pipe call failed");

	p = redir->nhere.doc->narg.text;
	if (redir->type == NXHERE) {
		expandarg(redir->nhere.doc, NULL, EXP_QUOTED);
		p = stackblock();
	}

	len = strlen(p);
	if (len <= PIPESIZE) {
		xwrite(pip[1], p, len);
		goto out;
	}

	if (forkshell((struct job *)NULL, (union node *)NULL, FORK_NOJOB) == 0) {
		close(pip[0]);
		signal(SIGINT, SIG_IGN);
#ifndef _MSC_VER
		signal(SIGQUIT, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
		signal(SIGTSTP, SIG_IGN);
		signal(SIGPIPE, SIG_DFL);
#endif
		xwrite(pip[1], p, len);
		_exit(0);
	}
out:
	close(pip[1]);
	return pip[0];
}



/*
 * Undo the effects of the last redirection.
 */

void
popredir(int drop)
{
	struct redirtab *rp;
	int i;

	intoff();
	rp = redirlist;
	for (i = 0 ; i < 10 ; i++) {
		switch (rp->renamed[i]) {
		case CLOSED:
			if (!drop)
				close(i);
			break;
		case EMPTY:
		case REALLY_CLOSED:
			break;
		default:
			if (!drop)
				dup2(rp->renamed[i], i);
			close(rp->renamed[i]);
			break;
		}
	}
	redirlist = rp->next;
	ckfree((char *)rp);
	inton();
}

/*
 * Undo all redirections.  Called on error or interrupt.
 */

void
redir_reset()
{
	/*
	 * Discard all saved file descriptors.
	 */
	unwindredir(0);
}


/*
 * Move a file descriptor to > 10.  Invokes sh_error on error unless
 * the original file dscriptor is not open.
 */

int
savefd(int from, int ofd)
{
	int newfd;
	int err;

	newfd = fcntl_dupfd(from, 10);
	err = newfd < 0 ? errno : 0;
	if (err != EBADF) {
		close(ofd);
		if (err)
			sh_error("%d: %s", from, strerror(err));
		else
			fcntl_setfd_cloexec(newfd);
	}

	return newfd;
}


int
redirectsafe(union node *redir, int flags)
{
	int err;
	volatile int saveint;
	struct jmploc *volatile savehandler = handler;
	struct jmploc jmploc;

	saveint = getint();
	if (!(err = setjmp(jmploc.loc) * 2)) {
		handler = &jmploc;
		redirect(redir, flags);
	}
	handler = savehandler;
	if (err && exception_ != EXERROR)
		longjmp(handler->loc, 1);
	restoreint(saveint);
	return err;
}


void unwindredir(struct redirtab *stop)
{
	while (redirlist != stop)
		popredir(0);
}


struct redirtab *pushredir(union node *redir)
{
	struct redirtab *sv;
	struct redirtab *q;
	int i;

	q = redirlist;
	if (!redir)
		goto out;

	sv = (struct redirtab *) ckmalloc(sizeof (struct redirtab));
	sv->next = q;
	redirlist = sv;
	for (i = 0; i < 10; i++)
		sv->renamed[i] = EMPTY;

out:
	return q;
}
