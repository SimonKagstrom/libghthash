AUTOMAKE_OPTIONS = gnu
lib_LTLIBRARIES = libghthash.la

libghthash_la_SOURCES = hash_table.c hash_functions.c
include_HEADERS = ght_hash_table.h
noinst_HEADERS =

libghthash_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

EXTRA_DIST = Makefile.win
