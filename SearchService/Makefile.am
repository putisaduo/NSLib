# --------------------------------
# pion automake configuration file
# --------------------------------

top_build_prefix = ../../pion-5.0.6
top_builddir = ../../pion-5.0.6
top_srcdir = ../../pion-5.0.6
AM_CPPFLAGS = -I../../pion-5.0.6/include -I../ -I../src/

pion_pluginsdir = @PION_PLUGINS_DIRECTORY@
pion_plugins_LTLIBRARIES = SearchService.la

SearchService_la_CXXFLAGS = -O3 -shared -std=c++11 $(AM_CXXFLAGS)
SearchService_la_SOURCES = Encrypt.cpp Encrypt.h Headers.h SearchService.cpp SearchService.hpp StringParam.h URL.cpp URL.h Worker.cpp Worker.h
SearchService_la_LDFLAGS = -no-undefined -module -avoid-version -L../src -lNSLib
SearchService_la_LIBADD = ../../pion-5.0.6/src/libpion.la @PION_EXTERNAL_LIBS@
SearchService_la_DEPENDENCIES = ../../pion-5.0.6/src/libpion.la


EXTRA_DIST = *.vcxproj *.vcxproj.filters
