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
 *
 *	@(#)memalloc.h	8.2 (Berkeley) 5/4/95
 */

#ifndef MEMALLOC_H
#define MEMALLOC_H

#include <stddef.h>
#include <stdlib.h>

struct stackmark {
	struct stack_block *stackp;
	char *stacknxt;
	size_t stacknleft;
};


extern char *stacknxt;
extern size_t stacknleft;
extern char *sstrend;

typedef void * pointer;

pointer ckmalloc(size_t);
pointer ckrealloc(pointer, size_t);
char *savestr(const char *);
pointer stalloc(size_t);
void stunalloc(pointer);
void pushstackmark(struct stackmark *mark, size_t len);
void setstackmark(struct stackmark *);
void popstackmark(struct stackmark *);
void growstackblock(void);
void *growstackstr(void);
char *makestrspace(size_t, char *);
char *stnputs(const char *, size_t, char *);
char *stputs(const char *, char *);


static inline void grabstackblock(size_t len)
{
	stalloc(len);
}

static inline char *_STPUTC(int c, char *p) {
	if (p == sstrend)
		p = (char *)growstackstr();
	*p++ = c;
	return p;
}

static inline char* stackblock()
{
    return stacknxt;
}

static inline size_t stackblocksize()
{
    return stacknleft;
}

static inline void startstackstr(char **p)
{
*p = stackblock();
}

static inline void stputc(int c, char **p)
{
    *p = _STPUTC(c, *p);
}

static inline void checkstrspace(size_t n, char **p)
{
    char *q = *p;
    size_t l = n;
    size_t m = sstrend - q;
    if (l > m)
        *p = makestrspace(l, q);
}

static inline void ustputc(int c, char **p)
{
    **p = c;
    *p += 1;
}

static inline void stackstrnull(char **p)
{
    if (*p == sstrend) {
        *p = (char *)growstackstr();
        **p = '\0';
    }
    else {
        **p = '\0';
    }
}

static inline void stunputc(char **p)
{
    *p = *p - 1;
}

static inline void stadjust(int amount, char **p)
{
    *p += amount;
}

static inline char* grabstackstr(char *p)
{
    return (char *) stalloc(p - (char *)stackblock());
}

static inline void* stackstrend()
{
    return (void *) sstrend;
}

static inline void ckfree(char *p)
{
    free(p);
}


#endif /* MEMALLOC_H */
