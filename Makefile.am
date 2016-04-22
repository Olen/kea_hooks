# SUBDIRS = . tests

# AM_CPPFLAGS  = -I$(top_builddir)/src/lib -I$(top_srcdir)/src/lib
AM_CPPFLAGS  = -I/usr/include/kea
AM_CPPFLAGS += $(BOOST_INCLUDES)
AM_CXXFLAGS  = $(KEA_CXXFLAGS)

ACLOCAL_AMFLAGS = -I m4

# Some versions of GCC warn about some versions of Boost regarding
# missing initializer for members in its posix_time.
# https://svn.boost.org/trac/boost/ticket/3477
# But older GCC compilers don't have the flag.
AM_CXXFLAGS += $(WARNING_NO_MISSING_FIELD_INITIALIZERS_CFLAG)
AM_CXXFLAGS += -std=c++11 

# Define rule to build logging source files from message file
options_to_options_messages.h options_to_options_messages.cc: s-messages
s-messages: options_to_options_messages.mes
	kea-msg-compiler options_to_options_messages.mes
# 	$(top_builddir)/src/lib/log/compiler/kea-msg-compiler options_to_options_messages.mes
	touch $@

# Tell automake that the message files are built as part of the build process
# (so that they are built before the main library is built).
BUILT_SOURCES = options_to_options_messages.h options_to_options_messages.cc

EXTRA_DIST =
# EXTRA_DIST = libdhcp_user_chk.dox
# Ensure that the message file is included in the distribution
EXTRA_DIST += options_to_options_messages.mes

# Get rid of generated message files on a clean
CLEANFILES = *.gcno *.gcda options_to_options_messages.h options_to_options_messages.cc s-messages

# convenience archive

noinst_LTLIBRARIES = libduc.la

libduc_la_SOURCES  =
libduc_la_SOURCES += load_unload.cc
libduc_la_SOURCES += pkt4_receive.cc
libduc_la_SOURCES += pkt4_send.cc
libduc_la_SOURCES += options_to_options.h
libduc_la_SOURCES += options_to_options_log.cc options_to_options_log.h

# libduc_la_SOURCES += subnet_select_co.cc
# libduc_la_SOURCES += user.cc user.h
# libduc_la_SOURCES += user_chk.h
# libduc_la_SOURCES += user_chk_log.cc user_chk_log.h
# libduc_la_SOURCES += user_data_source.h
# libduc_la_SOURCES += user_file.cc user_file.h
# libduc_la_SOURCES += user_registry.cc user_registry.h
libduc_la_SOURCES += version.cc

nodist_libduc_la_SOURCES = options_to_options_messages.h options_to_options_messages.cc

libduc_la_CXXFLAGS = $(AM_CXXFLAGS)
libduc_la_CPPFLAGS = $(AM_CPPFLAGS)


if USE_CLANGPP
# Disable unused parameter warning caused by some of the
# Boost headers when compiling with clang.
libduc_la_CXXFLAGS += -Wno-unused-parameter
endif

noinst_LTLIBRARIES += options_to_options.la

options_to_options_la_SOURCES  =
options_to_options_la_LDFLAGS  = $(AM_LDFLAGS)
options_to_options_la_LDFLAGS  += -avoid-version -export-dynamic -module
# -rpath /nowhere is a hack to trigger libtool to not create a
# convenience archive, resulting in shared modules
options_to_options_la_LDFLAGS  += -rpath /nowhere
options_to_options_la_LIBADD  = libduc.la
#options_to_options_la_LIBADD  += $(top_builddir)/src/lib/hooks/libkea-hooks.la
#options_to_options_la_LIBADD  += $(top_builddir)/src/lib/log/libkea-log.la
#options_to_options_la_LIBADD  += $(top_builddir)/src/lib/util/threads/libkea-threads.la
#options_to_options_la_LIBADD  += $(top_builddir)/src/lib/util/libkea-util.la
#options_to_options_la_LIBADD  += $(top_builddir)/src/lib/exceptions/libkea-exceptions.la
options_to_options_la_LIBADD  += $(LOG4CPLUS_LIBS)
