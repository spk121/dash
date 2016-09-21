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

#ifndef SYSTEM_H
#define SYSTEM_H

#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef _MSC_VER
#include <Windows.h>
#include <process.h>
#include <io.h>
#include <malloc.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <dirent.h>
#include <fcntl.h>
#include <paths.h>
#endif

 // _PATH_BSHELL
 // linux: "/dev/tty" in <path.h>
 // MSC: missing
#ifdef _MSC_VER
#define _PATH_BSHELL "/bin/sh"
#endif

// _PATH_DEVNULL
#ifdef _MSC_VER
#define _PATH_DEVNULL "/dev/null"
#endif

// _PATH_TTY
// linux: "/dev/tty" in <paths.h>
// MSC: missing
#ifdef _MSC_VER
#define _PATH_TTY "CON"
#endif

#ifdef _MSC_VER
#define DIR_SEPARATOR '\\'
#else
#define DIR_SEPARATOR '/'
#endif

#ifdef _MSC_VER
#define SEARCHPATH_SEPARATOR ';'
#else
#define SEARCHPATH_SEPARATOR ':'
#endif

// alloca:
// LSB: missing
// glibc: void *alloca(size_t size), in <stdlib.h>
// MSC: void *alloca(size_t size), in <malloc.h>

static inline void barrier()
{
#ifdef _MSC_VER
	MemoryBarrier();
#else
	__asm__ __volatile__("": : : "memory");
#endif
}

// environ:
//   glibc:  char ** environ, in <unistd.h>
//   msc:    extern char ** environ, in <stdlib.h>

// error
//
#define error printf

// fcntl: perform various operations on files
// LSB: int fcntl(int __fd, int __cmd, ...), in <fcntl.h>
// MSC: missing

// Duplicate filedescriptor to a number >= MIN
static inline int fcntl_dupfd(int fd, int min)
{
	#ifdef _MSC_VER
	return fd;
	#else
	return fcntl(fd, F_DUPFD, min);
	#endif
}
//
static inline int fcntl_getfl(int a, int b)
{
	#ifdef _MSC_VER
	return 0;
	#else
	return fcntl (a, F_GETFL, b);
	#endif
}

static inline int fcntl_setfl(int a, int b)
{
	#ifdef _MSC_VER
	return fcntl(a, F_SETFL, b);
	#else
	return 0;
	#endif
}
// Set file descriptor to close when an exec in invoked
static inline int fcntl_setfd_cloexec(int fd)
{
	#ifdef _MSC_VER
	return 0;
	#else
	return fcntl(fd, F_SETFD, FD_CLOEXEC);
	#endif
}

// fstat64:
// LSB: int fstat64(int __fd, struct stat64 * __buf), in <sys/stat.h>
// MSC: int _fstat64(int fd, struct __stat64 *buffer), in <sys/stat.h> & <sys/types.h>
#ifdef _MSC_VER
#define fstat64 _fstat64
#endif

// getegid,
// LSB: gid_t getegid(void) in <unistd.h> 
// MSC: missing
#ifdef _MSC_VER
static inline int getegid(void) { return 1000; }
#endif

// geteuid, returns effective user ID of the process
// LSB:  uid_t geteuid(void), in <unistd.h>
// MSC:  missing
#ifdef _MSC_VER
int geteuid(void);
#endif

// getgid,
// LSB: gid_t getgid(void) in <unistd.h> 
// MSC: missing
#ifdef _MSC_VER
static inline int getgid(void) { return 1000; }
#endif

// getpgrp
// LSB: pid_t getpgrp(void) in <unistd.h> 
// MSC: missing
#ifdef _MSC_VER
static inline int getpgrp(void) { return 1000; }
#endif

// getpid
// glibc: pid_t getpid(void), in <unistd.h>
// msc:   int _getpid(void), in <process.h>
#ifdef _MSC_VER
typedef DWORD sys_process_id_t;
#define SYS_PROCESS_ID_FORMAT "%lu"
#else
typedef pid_t sys_process_id_t;
#define SYS_PROCESS_ID_FORMAT "%d"
#endif
sys_process_id_t sys_get_process_id();

// getppid: return process ID of parent
// LSB:  pid_t getppid(void), in <unistd.h>
// MSC:  missing
#ifdef _MSC_VER
static inline int getppid(void)
{
	return 0;
}
#endif

// getuid
// LSB:  uid_t getuid(void), in <unistd.h>
// MSC:  missing
#ifdef _MSC_VER
static inline int getuid(void) { return 1000; }
#endif

// gid_t
#ifdef _MSC_VER
typedef int gid_t;
#endif

// kill
// LSB: int kill(pid_t __pid, int __sig), in <signal.h>
// msc:  missing
#ifdef _MSC_VER
static inline int kill(int a, int b) {
	return 0;
}
#endif

// O_NONBLOCK
// LSB: octal 04000 in <fcntl.h>
// MSC:missing
#ifdef _MSC_VER
#define O_NONBLOCK 0x0800
#endif
// pid_t: the type returned by getpid()
// glibc: int, in <sys/types.h>
// msc:   undefined, but, getpid() returns an int
#ifdef _MSC_VER
typedef  int pid_t;
#endif

// PATH_MAX
#ifdef _MSC_VER
#define PATH_MAX MAX_PATH
#endif

// pipe:
// LSB: int pipe(int __pipedes [2]), in <unistd.h>
// MSC: int _pipe(int *pfds, unsigned int psize, int textmode), in <io.h>
#ifdef _MSC_VER
int pipe(int p[2]);
#endif

// open64: returns a file descriptor that can open a large file
// LSB: int open64(const char *, int, ...), in <fcntl.h>
// MSC: int _open(const char *filename, int oflag[, int pmode]), in <io.h>
#ifdef _MSC_VER
#define open64 _open
#endif

// S_ISDIR:
// LSB: macro, in <sys/stat.h>
// MSC: missing
#ifdef _MSC_VER
#define S_ISDIR(m) (((m)&_S_IFMT) == _S_IFDIR)
#endif

// S_ISREG:
// LSB: macro, in <sys/stat.h>
// MSC: missing
#ifdef _MSC_VER
#define S_ISREG(m) (((m)&_S_IFMT) == _S_IFREG)
#endif

// setpgid
// LSB: int setpgid(pid_t __pid, pid_t __pgid) in <unistd.h> 
// MSC: missing
#ifdef _MSC_VER
static inline int setpgid(int pid, int pgid) { return 0;  }
#endif

// setsignal


// sig_atomic_t:
// glibc: int, in <signal.h>
// msc: int, in <signal.h>

static inline void sigclearmask(void)
{
	// This is supposed to clear the list of signals that are blocked.

#ifdef _MSC_VER
#else
	sigsetmask(0);
#endif
}

// signal:
// glibc: sighandler_t signal(int signum, sighandler_t func), in <signal.h>
//        sighandler_t is void (*)(int)
// msc: void (__cdecl *signal(int sig, void (__cdecl *func) (int[, int])))(int), in <signal.h>
#ifdef _MSC_VER
typedef void(__cdecl *sighandler_t) (int);
#endif

// SIGCONT
#ifdef _MSC_VER
#define SIGCONT 19 /* continue a stopped process */
#endif

// SIGPIPE
#ifdef _MSC_VER
#define SIGPIPE 13 /* write on a pipe with no one to read it */
#endif

// SIGQUIT
#ifdef _MSC_VER
#define SIGQUIT 3       /* quit */
#endif

// SIGTTIN
#ifdef _MSC_VER
#define SIGTTIN 21      /* to readers pgrp upon background tty read */
#endif

// SIGTSTP
#ifdef _MSC_VER
#define SIGTSTP 18      /* stop signal from tty */
#endif

// SIGTTOU
#ifdef _MSC_VER
#define SIGTTOU 22      /* like TTIN for output if (tp->t_local<OSTOP) */
#endif

// SIGTTIN
#ifdef _MSC_VER
#define SIGTTIN 21      /* to readers pgrp upon background tty read */
#endif

// ssize_t
// glibc:
// MSC: missing
#ifdef _MSC_VER
typedef __int64 ssize_t;
#endif

#ifndef SSIZE_MAX
#define SSIZE_MAX ((ssize_t)((size_t)-1 >> 1))
#endif

// struct stat64:
// LSB: in <sys/stat.h>
// MSC: missing.  _stat64 is in <sys/types.h>
#ifdef _MSC_VER
#define stat64 _stat64
#endif

// stpcpy
// LSB: char * stpcpy(char *dest, const char *src) in #include <string.h> 
// MSC: missing
#ifdef _MSC_VER
char *stpcpy(char *dest, const char *src);
#endif

// strchrnul
// LSB: never been in LSB
// glibc: char *strchrnul(const char *s, int c), in <string.h>
// MSC: missing
#ifdef _MSC_VER
char *strchrnul(const char *s, int c);
#endif

// strsignal
// LSB: char *strsignal(int __sig), in <string.h>
// MSC: missing
#ifdef _MSCVER
char *strsignal(int sig);
#endif

// tcgetpgrp
// LSB: pid_t tcgetpgrp(int __fd) in <unistd.h> 
// MSC: missing
#ifdef _MSC_VER
static inline int tcgetpgrp(int fd) { return 1000; }
#endif

// times
// LSB: clock_t times(struct tms * __buffer) in <sys/times.h> 
// MSC: missing
#ifdef _MSC_VER
static inline long times(struct tms *_buf) { return 0; }
#endif

// struct tms:
// LSB: in <sys/times.h>
// MSC: missing
#ifdef _MSC_VER
struct tms {
	long tms_utime;
	long tms_stime;
	long tms_cutime;
	long tms_cstime;
};
#endif

#ifndef HAVE_MEMPCPY
void *mempcpy(void *, const void *, size_t);
#endif

#ifndef HAVE_STRCHRNUL
char *strchrnul(const char *, int);
#endif

#ifndef HAVE_STRSIGNAL
char *strsignal(int);
#endif

#ifndef HAVE_STRTOD
static inline double strtod(const char *nptr, char **endptr)
{
	*endptr = (char *)nptr;
	return 0;
}
#endif

#ifndef HAVE_STRTOIMAX
#define strtoimax strtoll
#endif

#ifndef HAVE_STRTOUMAX
#define strtoumax strtoull
#endif

#ifndef HAVE_KILLPG
static inline int killpg(pid_t pid, int signal)
{
#ifdef DEBUG
	if (pid < 0)
		abort();
#endif
#ifdef _MSC_VER
	// FIXME: implement me
	abort();
#else
	return kill(-pid, signal);
#endif
}
#endif

#ifndef HAVE_SYSCONF
#define _SC_CLK_TCK 2
long sysconf(int);
#endif

#endif /* SYSTEM_H */
