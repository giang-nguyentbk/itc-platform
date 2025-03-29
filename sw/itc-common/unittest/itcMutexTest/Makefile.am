noinst_LIBRARIES += libitcMutexTest.a
itc_platform_unittest_LDADD += libitcMutexTest.a
TEST_SUITES_ADD += -Wl,libitcMutexTest.a

noinst_LIBRARIES += libitcMutex.a
itc_platform_unittest_LDADD += libitcMutex.a
TEST_SUITES_ADD += -Wl,libitcMutex.a

libitcMutexTest_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if

libitcMutex_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if

if ENABLE_TEST_COVERAGE_YES
libitcMutex_a_CPPFLAGS += -fprofile-arcs -ftest-coverage --coverage -O0 -g
endif

libitcMutexTest_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/itcMutexTest/itcMutexTest.cc

libitcMutex_a_COMMON_SOURCES 	= \
				sw/itc-common/src/itcMutex.cc

###
#
# libitcMutexTest_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcMutexTest_a_SOURCES = $(libitcMutexTest_a_COMMON_SOURCES)
libitcMutex_a_SOURCES = $(libitcMutex_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcMutexTest_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###