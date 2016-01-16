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
 *
 *	@(#)alias.h	8.2 (Berkeley) 5/4/95
 */

#ifndef ALIAS_H
#define ALIAS_H

#define ALIASINUSE	1
#define ALIASDEAD	2

class alias {
 private:
	int flag;
 public:
	alias *next;
	char *name;
	char *val;

	alias(char *_name, char *_val)
		: flag{0}, next{nullptr}, name{_name}, val{_val} {};
	int in_use() { return flag & ALIASINUSE; }
	void set_in_use() { flag |= ALIASINUSE; }
	void set_not_in_use() { flag &= ~ALIASINUSE; }
	int is_dead() {return flag & ALIASDEAD; }
	void set_not_dead() { flag &= ~ALIASDEAD; }
	void set_dead() { flag |= ALIASDEAD; }
};

struct alias *lookupalias(const char *, int);
int aliascmd(int, char **);
int unaliascmd(int, char **);
void rmaliases(void);
int unalias(const char *);
void printalias(const struct alias *);


#endif /* ALIAS_H */
