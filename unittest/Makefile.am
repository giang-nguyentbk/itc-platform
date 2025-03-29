bin_PROGRAMS = itc_platform_unittest
itc_platform_unittest_SOURCES = \
                            unittest/main.cc

noinst_LIBRARIES =
itc_platform_unittest_LDADD = 
itc_platform_unittest_LDFLAGS =
itc_platform_unittest_CPPFLAGS =

if ENABLE_TEST_COVERAGE_YES
itc_platform_unittest_LDFLAGS += --coverage
endif
if ENABLE_VALGRIND_YES
itc_platform_unittest_CPPFLAGS += -g
endif

TEST_SUITES_ADD = -Wl,--whole-archive,
if ENABLE_UNITTEST_PARTIAL
include unittest/Makefile.partial.suite.includes.am
else
# ENABLE_UNITTEST_ALL
include unittest/Makefile.all.suite.includes.am
endif
TEST_SUITES_ADD += -Wl,--no-whole-archive

itc_platform_unittest_LDFLAGS += -lgtest -lgtest_main -lgmock \
                -lpthread -Wl,--export-dynamic \
                $(TEST_SUITES_ADD)
