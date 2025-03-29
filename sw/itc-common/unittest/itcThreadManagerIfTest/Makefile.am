noinst_LIBRARIES += libitcThreadManagerIfTest.a
itc_platform_unittest_LDADD += libitcThreadManagerIfTest.a
TEST_SUITES_ADD += -Wl,libitcThreadManagerIfTest.a

noinst_LIBRARIES += libitcThreadManagerIf.a
itc_platform_unittest_LDADD += libitcThreadManagerIf.a
TEST_SUITES_ADD += -Wl,libitcThreadManagerIf.a

libitcThreadManagerIfTest_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-common/unittest/itcCWrapperIfMock

libitcThreadManagerIf_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-DUNITTEST

if ENABLE_TEST_COVERAGE_YES
libitcThreadManagerIf_a_CPPFLAGS += -fprofile-arcs -ftest-coverage --coverage -O0 -g
endif

libitcThreadManagerIfTest_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/itcThreadManagerIfTest/itcThreadManagerIfTest.cc

libitcThreadManagerIf_a_COMMON_SOURCES 	= \
				sw/itc-common/src/itcThreadManager.cc

###
#
# libitcThreadManagerIfTest_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcThreadManagerIfTest_a_SOURCES = $(libitcThreadManagerIfTest_a_COMMON_SOURCES)
libitcThreadManagerIf_a_SOURCES = $(libitcThreadManagerIf_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcThreadManagerIfTest_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###