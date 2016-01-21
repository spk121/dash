/*-
 * Copyright (c) 1993
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

#include <map>
#include <string>
using namespace std;

#include <stdlib.h>
#include "shell.h"
#include "input.h"
#include "output.h"
#include "error.h"
#include "memalloc.h"
#include "mystring.h"
#include "alias.h"
#include "options.h"	/* XXX for argptr (should remove?) */

#define ATABSIZE 39

static map<string, alias> atab {};
// static struct alias *atab[ATABSIZE];

static void setalias(const char *, const char *);
static struct alias *freealias(struct alias *);
static struct alias **__lookupalias(const char *);

static
void
setalias(const char *name, const char *val)
{
	string nstr {name};

	INTOFF;
	if (atab.count(nstr) == 1) {
		auto& ap = atab[nstr];
		if (!ap.in_use()) {
			ckfree(ap.val);
		}
		ap.val	= savestr(val);
		ap.set_not_dead();
	} else {
		/* not found */
		auto& ap = atab[nstr];
		ap = alias {(char *)name, (char *)val};
		// ap.name = savestr(name);
		// ap.val = savestr(val);
		// ap.next = nullptr;
		// ap.set_not_dead();
		// ap.set_not_in_use();
	}
	INTON;
}

int
unalias(const char *name)
{
	string nstr {name};

	if (atab.count(nstr) == 1) {
		INTOFF;
		atab.erase(nstr);
		INTON;
		return 0;
	}
	return 1;
}

void
rmaliases(void)
{
	INTOFF;
	atab.clear();
	INTON;
}

struct alias *
lookupalias(const char *name, int check)
{
	string nstr {name};
	if (atab.count(nstr) == 1) {
		alias ap = atab.at(nstr);
		if (check && ap.in_use())
			return nullptr;
		alias* a2 = new alias{ap};
		return a2;
	}
	return nullptr;
}

/*
 * TODO - sort output
 */
int
aliascmd(int argc, char **argv)
{
	char *n, *v;
	int ret = 0;
	struct alias *ap;

	if (argc == 1) {
		for(auto &key_value : atab)
			printalias(&(key_value.second));
		return (0);
	}
	while ((n = *++argv) != NULL) {
		if ((v = strchr(n+1, '=')) == NULL) { /* n+1: funny ksh stuff */
			if (atab.count(string{n}) == 0) {
				outfmt(out2, "%s: %s not found\n", "alias", n);
				ret = 1;
			} else
				printalias(&atab[string{n}]);
		} else {
			*v++ = '\0';
			setalias(n, v);
		}
	}

	return (ret);
}

int
unaliascmd(int argc, char **argv)
{
	int i;

	while ((i = nextopt("a")) != '\0') {
		if (i == 'a') {
			rmaliases();
			return (0);
		}
	}
	for (i = 0; *argptr; argptr++) {
		if (unalias(*argptr)) {
			outfmt(out2, "%s: %s not found\n", "unalias", *argptr);
			i = 1;
		}
	}

	return (i);
}

static struct alias *
freealias(struct alias *ap) {
	struct alias *next;

	if (ap->in_use()) {
		ap->set_dead();
		return ap;
	}

	next = ap->next;
	ckfree(ap->name);
	ckfree(ap->val);
	ckfree(ap);
	return next;
}

void
printalias(const struct alias *ap) {
	out1fmt("%s=%s\n", ap->name, single_quote(ap->val));
}

#if 0
static struct alias **
__lookupalias(const char *name) {
	unsigned int hashval;
	struct alias **app;
	const char *p;
	unsigned int ch;

	p = name;

	ch = (unsigned char)*p;
	hashval = ch << 4;
	while (ch) {
		hashval += ch;
		ch = (unsigned char)*++p;
	}
	app = &atab[hashval % ATABSIZE];

	for (; *app; app = &(*app)->next) {
		if (strequal(name, (*app)->name)) {
			break;
		}
	}

	return app;
}

#endif
