noinst_LIBRARIES += libitcMutexTest.a
itc_platform_unittest_LDADD += libitcMutexTest.a
TEST_SUITES_ADD += -Wl,libitcMutexTest.a

libitcMutexTest_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc


libitcMutexTest_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/itcMutexTest/itcMutexTest.cc

###
#
# libitcMutexTest_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcMutexTest_a_SOURCES = $(libitcMutexTest_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcMutexTest_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###