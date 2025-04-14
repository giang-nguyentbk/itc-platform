noinst_LIBRARIES += libitcThreadPoolTest.a
itc_platform_unittest_LDADD += libitcThreadPoolTest.a
TEST_SUITES_ADD += -Wl,libitcThreadPoolTest.a

libitcThreadPoolTest_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc

libitcThreadPoolTest_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/itcThreadPoolTest/itcThreadPoolTest.cc

###
#
# libitcThreadPoolTest_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcThreadPoolTest_a_SOURCES = $(libitcThreadPoolTest_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcThreadPoolTest_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###