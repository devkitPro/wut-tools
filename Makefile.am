# Makefile.am -- Process this file with automake to produce Makefile.in
ACLOCAL_AMFLAGS = -I m4

common_CPPFLAGS = -I$(top_srcdir)/src/common
excmd_CPPFLAGS = -I${top_srcdir}/libraries/excmd/src
fmt_CPPFLAGS = -I${top_srcdir}/libraries/fmt/include

excmd_files = libraries/excmd/src/excmd_exception.h  \
	libraries/excmd/src/excmd.h  \
	libraries/excmd/src/excmd_meta.h  \
	libraries/excmd/src/excmd_str.h  \
	libraries/excmd/src/excmd_value_parser.h


bin_PROGRAMS = elf2rpl readrpl rplexportgen rplimportgen wuhbtool udplogserver

noinst_LTLIBRARIES      = libfmt.la

libfmt_la_SOURCES = libraries/fmt/include/fmt/core.h  \
	libraries/fmt/include/fmt/format.h  \
	libraries/fmt/include/fmt/format-inl.h  \
	libraries/fmt/include/fmt/ostream.h \
	libraries/fmt/include/fmt/posix.h  \
	libraries/fmt/include/fmt/printf.h  \
	libraries/fmt/include/fmt/ranges.h  \
	libraries/fmt/include/fmt/time.h \
	libraries/fmt/src/format.cc \
	libraries/fmt/src/posix.cc

libfmt_la_CPPFLAGS	= $(fmt_CPPFLAGS)

elf2rpl_SOURCES	= $(excmd_files) src/elf2rpl/main.cpp

elf2rpl_CPPFLAGS = @ZLIB_CFLAGS@ $(common_CPPFLAGS) ${excmd_CPPFLAGS} ${fmt_CPPFLAGS}
elf2rpl_LDADD = libfmt.la @ZLIB_LIBS@

readrpl_SOURCES = src/readrpl/generate_exports_def.cpp  \
	src/readrpl/generate_exports_def.h  \
	src/readrpl/main.cpp  src/readrpl/print.cpp  \
	src/readrpl/print.h  src/readrpl/readrpl.h  \
	src/readrpl/verify.cpp src/readrpl/verify.h

readrpl_CPPFLAGS = @ZLIB_CFLAGS@ $(common_CPPFLAGS) ${excmd_CPPFLAGS} ${fmt_CPPFLAGS}
readrpl_LDADD = libfmt.la @ZLIB_LIBS@

rplexportgen_SOURCES = src/rplexportgen/rplexportgen.cpp

rplexportgen_CPPFLAGS = @ZLIB_CFLAGS@ $(common_CPPFLAGS) ${excmd_CPPFLAGS} ${fmt_CPPFLAGS}
rplexportgen_LDADD = @ZLIB_LIBS@

rplimportgen_SOURCES = src/rplimportgen/rplimportgen.cpp

rplimportgen_CPPFLAGS = @ZLIB_CFLAGS@ $(common_CPPFLAGS) ${excmd_CPPFLAGS} ${fmt_CPPFLAGS}
rplimportgen_LDADD = @ZLIB_LIBS@

wuhbtool_SOURCES = $(excmd_files) \
	src/wuhbtool/main.cpp \
	src/wuhbtool/entities/BufferFileEntry.cpp \
	src/wuhbtool/entities/BufferFileEntry.h \
	src/wuhbtool/entities/DirectoryEntry.cpp \
	src/wuhbtool/entities/DirectoryEntry.h \
	src/wuhbtool/entities/FileEntry.cpp \
	src/wuhbtool/entities/FileEntry.h \
	src/wuhbtool/entities/NodeEntry.cpp \
	src/wuhbtool/entities/NodeEntry.h \
	src/wuhbtool/entities/OSFileEntry.cpp \
	src/wuhbtool/entities/OSFileEntry.h \
	src/wuhbtool/entities/RootEntry.cpp \
	src/wuhbtool/entities/RootEntry.h \
	src/wuhbtool/services/RomFSService.cpp \
	src/wuhbtool/services/RomFSService.h \
	src/wuhbtool/services/RomFSStructs.h \
	src/wuhbtool/services/TgaGzService.cpp \
	src/wuhbtool/services/TgaGzService.h \
	src/wuhbtool/utils/filepath.cpp \
	src/wuhbtool/utils/filepath.h \
	src/wuhbtool/utils/types.h \
	src/wuhbtool/utils/utils.h

wuhbtool_CPPFLAGS = @ZLIB_CFLAGS@ $(common_CPPFLAGS) ${excmd_CPPFLAGS}
wuhbtool_LDADD = @ZLIB_LIBS@ @FREEIMAGE_LIBS@

udplogserver_SOURCES = src/udplogserver/main.cpp
udplogserver_LDADD = @NET_LIBS@

EXTRA_DIST = autogen.sh src/common/be_val.h  src/common/elf.h  src/common/rplwrap.h src/common/type_traits.h  src/common/utils.h LICENSE.md
