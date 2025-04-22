noinst_LIBRARIES += libitcConcurrentContainerTest.a
itc_platform_unittest_LDADD += libitcConcurrentContainerTest.a
TEST_SUITES_ADD += -Wl,libitcConcurrentContainerTest.a

libitcConcurrentContainerTest_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc

libitcConcurrentContainerTest_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/itcConcurrentContainerTest/itcConcurrentContainerTest.cc

###
#
# libitcConcurrentContainerTest_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcConcurrentContainerTest_a_SOURCES = $(libitcConcurrentContainerTest_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcConcurrentContainerTest_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###