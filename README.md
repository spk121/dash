# dash
A modified version of the Debian Almquest Shell (dash).

* What is this project
This is Mike Gran's hack of the Debian Almquest Shell (dash).

I was looking for some other project to try to practice more modern C++ idioms.
I started off playing with this shell code because it seems to have many hand-coded
data structures and algorithms that already exist as pre-made structures and algorithms
in C++17.

But since refactoring is a chore, I wanted to first make it build in Visual Studio
so I could use its rather nice refactoring abilities.

That lead me down a rabbit hole.  I'd never tried to port native BSD libc or GNU libc
code to native MSVC.  It is all very different: processes, threads, console model,
environment variables.  And something like a shell is heavily dependent on this
low-level functionality.

So suddenly this little exercise became somewhat more interesting, and now I'm
playing with it.

Will I finish it?  I don't know.  I've never finished anything before, so chances
are low.

* Status
It builds and runs on GNU/Linux.  It builds and can be launched on Windows 7/10.
But on Windows, virtually all functionality is broken.

* TODO
** Environment Variables
Vanilla dash has a hand-coded singly-linked list to store the environment variables.
It has a manual memory management scheme.

POSIX expects the environ variables names to be in a 7-bit portable character set,
with the values in 8-bit-clean character set of unspecified encoding.

Windows has environment variables encoded in wchar_t.

Plan: store env vars in a map<wstring,...>.  For POSIX, convert
locale encoding to wchar_t using codecvt.
