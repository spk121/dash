AM_YFLAGS = -d

# COMMON_CFLAGS = -x c++ -std=c++14 -Wall -g -O0
COMMON_CFLAGS = -Wall -g -O0
COMMON_CPPFLAGS = \
	-include $(top_builddir)/config.h \
	-DBSD=1 -DSHELL \
	-DIFS_BROKEN

AM_CFLAGS = $(COMMON_CFLAGS)
AM_CPPFLAGS = $(COMMON_CPPFLAGS)
AM_CFLAGS_FOR_BUILD = $(COMMON_CFLAGS) 
AM_CPPFLAGS_FOR_BUILD = $(COMMON_CPPFLAGS)

COMPILE_FOR_BUILD = \
	$(CC_FOR_BUILD) $(DEFAULT_INCLUDES) $(AM_CPPFLAGS_FOR_BUILD) \
	$(CPPFLAGS_FOR_BUILD) \
	$(AM_CFLAGS_FOR_BUILD) $(CFLAGS_FOR_BUILD) 

bin_PROGRAMS = dash

dash_CXXFILES = \
	histedit.cpp mail.cpp main.cpp memalloc.cpp miscbltin.cpp \
	mystring.cpp options.cpp parser.cpp redir.cpp show.cpp trap.cpp output.cpp \
	bltin/printf.cpp system.cpp bltin/test.cpp bltin/times.cpp var.c
dash_CXXFILES += \
	alias.cpp arith_yacc.cpp arith_yylex.cpp cd.cpp eval.cpp error.cpp \
	exec.cpp expand.cpp \
	input.cpp jobs.cpp
dash_SOURCES = \
	$(dash_CXXFILES) \
	alias.h arith_yacc.h bltin/bltin.h cd.h error.h eval.h exec.h \
	expand.h \
	init.h input.h jobs.h machdep.h mail.h main.h memalloc.h miscbltin.h \
	myhistedit.h mystring.h options.h output.h parser.h redir.h shell.h \
	show.h system.h trap.h var.h
dash_LDADD = builtins.o init.o nodes.o signames.o syntax.o

HELPERS = mkinit mksyntax mknodes mksignames

BUILT_SOURCES = builtins.h builtins.cpp init.cpp nodes.h nodes.cpp \
	signames.cpp \
	syntax.h token.h token_vars.h
CLEANFILES = \
	$(BUILT_SOURCES) $(patsubst %.o,%.c,$(dash_LDADD)) \
	$(HELPERS) builtins.def

man_MANS = dash.1
EXTRA_DIST = \
	$(man_MANS) \
	mktokens mkbuiltins builtins.def.in mkinit.c \
	mknodes.c nodetypes nodes.c.pat mksyntax.c mksignames.c

token.h token_vars.h: mktokens
	sh $^

builtins.def: builtins.def.in $(top_builddir)/config.h
	$(COMPILE) -E -x c -o $@ $<

builtins.cpp builtins.h: mkbuiltins builtins.def
	sh $^

init.cpp: mkinit $(dash_CFILES) $(dask_CXXFILES)
	./$^

nodes.cpp nodes.h: mknodes nodetypes nodes.c.pat
	./$^

syntax.cpp syntax.h: mksyntax
	./$^

signames.cpp: mksignames
	./$^

mksyntax: token.h

$(HELPERS): %: %.c
	$(COMPILE_FOR_BUILD) -o $@ $<
