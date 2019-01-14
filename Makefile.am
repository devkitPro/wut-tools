# Makefile.am -- Process this file with automake to produce Makefile.in

common_CPPFLAGS = -I$(top_srcdir)/src/common
excmd_CPPFLAGS = -I${top_srcdir}/libraries/excmd/src
fmt_CPPFLAGS = -I${top_srcdir}/libraries/fmt/include

excmd_files = libraries/excmd/src/excmd_exception.h  \
	libraries/excmd/src/excmd.h  \
	libraries/excmd/src/excmd_meta.h  \
	libraries/excmd/src/excmd_str.h  \
	libraries/excmd/src/excmd_value_parser.h

fmt_files = libraries/fmt/include/fmt/core.h  \
	libraries/fmt/include/fmt/format.h  \
	libraries/fmt/include/fmt/format-inl.h  \
	libraries/fmt/include/fmt/ostream.h \
	libraries/fmt/include/fmt/posix.h  \
	libraries/fmt/include/fmt/printf.h  \
	libraries/fmt/include/fmt/ranges.h  \
	libraries/fmt/include/fmt/time.h \
	libraries/fmt/src/format.cc \
	libraries/fmt/src/posix.cc


bin_PROGRAMS = elf2rpl readrpl rplexportgen rplimportgen udplogserver

elf2rpl_SOURCES	= $(excmd_files) $(fmt_files) src/elf2rpl/main.cpp

elf2rpl_CPPFLAGS = @ZLIB_CFLAGS@ $(common_CPPFLAGS) ${excmd_CPPFLAGS} ${fmt_CPPFLAGS}
elf2rpl_LDADD = @ZLIB_LIBS@

readrpl_SOURCES = $(fmt_files) src/readrpl/generate_exports_def.cpp  \
	src/readrpl/generate_exports_def.h  \
	src/readrpl/main.cpp  src/readrpl/print.cpp  \
	src/readrpl/print.h  src/readrpl/readrpl.h  \
	src/readrpl/verify.cpp src/readrpl/verify.h

readrpl_CPPFLAGS = @ZLIB_CFLAGS@ $(common_CPPFLAGS) ${excmd_CPPFLAGS} ${fmt_CPPFLAGS}
readrpl_LDADD = @ZLIB_LIBS@

rplexportgen_SOURCES = src/rplexportgen/rplexportgen.cpp

rplexportgen_CPPFLAGS = @ZLIB_CFLAGS@ $(common_CPPFLAGS) ${excmd_CPPFLAGS} ${fmt_CPPFLAGS}
rplexportgen_LDADD = @ZLIB_LIBS@

rplimportgen_SOURCES = src/rplimportgen/rplimportgen.cpp

rplimportgen_CPPFLAGS = @ZLIB_CFLAGS@ $(common_CPPFLAGS) ${excmd_CPPFLAGS} ${fmt_CPPFLAGS}
rplimportgen_LDADD = @ZLIB_LIBS@

udplogserver_SOURCES = src/udplogserver/main.cpp
udplogserver_LDADD = @NET_LIBS@

EXTRA_DIST = autogen.sh src/common/be_val.h  src/common/elf.h  src/common/type_traits.h  src/common/utils.h
