/*
 * Copyright (c) 2004
 *	Herbert Xu <herbert@gondor.apana.org.au>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
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

#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>


#include "error.h"
#include "output.h"
#include "system.h"

 // geteuid, returns effective user ID of the process
 // LSB:  uid_t geteuid(void), in <unistd.h>
 // MSC:  missing
#ifdef _MSC_VER
int geteuid(void)
{
	return 1000;
}
#endif

// pipe:
// LSB: int pipe(int __pipedes [2]), in <unistd.h>
// MSC: int _pipe(int *pfds, unsigned int psize, int textmode), in <io.h>
#ifdef _MSC_VER
int pipe(int p[2])
{
	return _pipe(p, 256, _O_BINARY);
}
#endif

#ifdef _MSC_VER
char *stpcpy(char *dest, const char *src)
{
	char *d = dest;
	const char *s = src;
	do
		*d++ = *s;
	while (*s++ != '\0');
	return d - 1;
}
#endif

#ifdef _MSC_VER
// From GLIBC
// LGPL 2.1
/* Find the first occurrence of C in S or the final NUL byte.  */
char *strchrnul(const char *s, int c_in)
{
	const unsigned char *char_ptr;
	const unsigned long int *longword_ptr;
	unsigned long int longword, magic_bits, charmask;
	unsigned char c;

	c = (unsigned char)c_in;

	/* Handle the first few characters by reading one character at a time.
	Do this until CHAR_PTR is aligned on a longword boundary.  */
	for (char_ptr = (const unsigned char *)s;
	((unsigned long int) char_ptr & (sizeof(longword) - 1)) != 0;
		++char_ptr)
		if (*char_ptr == c || *char_ptr == '\0')
			return (char *)char_ptr;

	/* All these elucidatory comments refer to 4-byte longwords,
	but the theory applies equally well to 8-byte longwords.  */

	longword_ptr = (unsigned long int *) char_ptr;

	/* Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
	the "holes."  Note that there is a hole just to the left of
	each byte, with an extra at the end:

	bits:  01111110 11111110 11111110 11111111
	bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD

	The 1-bits make sure that carries propagate to the next 0-bit.
	The 0-bits provide holes for carries to fall into.  */
	switch (sizeof(longword))
	{
	case 4: magic_bits = 0x7efefeffL; break;
	case 8: magic_bits = ((0x7efefefeL << 16) << 16) | 0xfefefeffL; break;
	default:
		abort();
	}

	/* Set up a longword, each of whose bytes is C.  */
	charmask = c | (c << 8);
	charmask |= charmask << 16;
	if (sizeof(longword) > 4)
		/* Do the shift in two steps to avoid a warning if long has 32 bits.  */
		charmask |= (charmask << 16) << 16;
	if (sizeof(longword) > 8)
		abort();

	/* Instead of the traditional loop which tests each character,
	we will test a longword at a time.  The tricky part is testing
	if *any of the four* bytes in the longword in question are zero.  */
	for (;;)
	{
		/* We tentatively exit the loop if adding MAGIC_BITS to
		LONGWORD fails to change any of the hole bits of LONGWORD.

		1) Is this safe?  Will it catch all the zero bytes?
		Suppose there is a byte with all zeros.  Any carry bits
		propagating from its left will fall into the hole at its
		least significant bit and stop.  Since there will be no
		carry from its most significant bit, the LSB of the
		byte to the left will be unchanged, and the zero will be
		detected.

		2) Is this worthwhile?  Will it ignore everything except
		zero bytes?  Suppose every byte of LONGWORD has a bit set
		somewhere.  There will be a carry into bit 8.  If bit 8
		is set, this will carry into bit 16.  If bit 8 is clear,
		one of bits 9-15 must be set, so there will be a carry
		into bit 16.  Similarly, there will be a carry into bit
		24.  If one of bits 24-30 is set, there will be a carry
		into bit 31, so all of the hole bits will be changed.

		The one misfire occurs when bits 24-30 are clear and bit
		31 is set; in this case, the hole at bit 31 is not
		changed.  If we had access to the processor carry flag,
		we could close this loophole by putting the fourth hole
		at bit 32!

		So it ignores everything except 128's, when they're aligned
		properly.

		3) But wait!  Aren't we looking for C as well as zero?
		Good point.  So what we do is XOR LONGWORD with a longword,
		each of whose bytes is C.  This turns each byte that is C
		into a zero.  */

		longword = *longword_ptr++;

		/* Add MAGIC_BITS to LONGWORD.  */
		if ((((longword + magic_bits)

			/* Set those bits that were unchanged by the addition.  */
			^ ~longword)

			/* Look at only the hole bits.  If any of the hole bits
			are unchanged, most likely one of the bytes was a
			zero.  */
			& ~magic_bits) != 0 ||

			/* That caught zeroes.  Now test for C.  */
			((((longword ^ charmask) + magic_bits) ^ ~(longword ^ charmask))
				& ~magic_bits) != 0)
		{
			/* Which of the bytes was C or zero?
			If none of them were, it was a misfire; continue the search.  */

			const unsigned char *cp = (const unsigned char *)(longword_ptr - 1);

			if (*cp == c || *cp == '\0')
				return (char *)cp;
			if (*++cp == c || *cp == '\0')
				return (char *)cp;
			if (*++cp == c || *cp == '\0')
				return (char *)cp;
			if (*++cp == c || *cp == '\0')
				return (char *)cp;
			if (sizeof(longword) > 4)
			{
				if (*++cp == c || *cp == '\0')
					return (char *)cp;
				if (*++cp == c || *cp == '\0')
					return (char *)cp;
				if (*++cp == c || *cp == '\0')
					return (char *)cp;
				if (*++cp == c || *cp == '\0')
					return (char *)cp;
			}
		}
	}

	/* This should never happen.  */
	return NULL;
}

#endif

#ifdef _MSC_VER
#include <signal.h>
#define BUFFERSIZ	100
static char *_sys_siglist[NSIG + 1] =
{
	NULL, // 0
	NULL, // 1
	"Interrupt", // 2 = SIGINT
	NULL,
	"Illegal instruction", // 4 = SIGILL
	NULL, // 5
	"Aborted", // 6 = SIGABRT_COMPAT
	NULL,
	"Floating point exception", // 8 = SIGFPE
	NULL,
	NULL, // 10
	"Segmentation fault", // 11 = SIGSEGV
	NULL,
	NULL,
	"Terminated", // 15 = SIGTERM
	NULL,
	NULL,
	NULL,
	NULL,
	NULL, // 20
	"Break", // 21 = SIGBREAK
	"Aborted", // 22 = SIGABRT
	NULL, // 23 = NSIG
};

static char static_buf[BUFFERSIZ];
/* Return a string describing the meaning of the signal number SIGNUM.  */
char *
strsignal(int signum)
{
	const char *desc;

	if (signum < 0 || signum >= NSIG
		|| (desc = _sys_siglist[signum]) == NULL)
	{
		int len;
			len = _snprintf(static_buf, BUFFERSIZ - 1, "Unknown signal %d",
				signum);
		if (len >= BUFFERSIZ)
			return NULL;
		
		static_buf[len] = '\0';
		return static_buf;
	}

	return (char *)desc;
}
#endif

#ifndef HAVE_MEMPCPY
void *mempcpy(void *dest, const void *src, size_t n)
{
	return (void *)((char *)memcpy(dest, src, n) + n);
}
#endif

#ifndef HAVE_SYSCONF
long sysconf(int name)
{
	sh_error("no sysconf for: %d", name);
	return 0;
}
#endif

