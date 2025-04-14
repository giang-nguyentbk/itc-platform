noinst_LIBRARIES += libitcSmartContainerTest.a
itc_platform_unittest_LDADD += libitcSmartContainerTest.a
TEST_SUITES_ADD += -Wl,libitcSmartContainerTest.a

libitcSmartContainerTest_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc

libitcSmartContainerTest_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/itcSmartContainerTest/itcSmartContainerTest.cc

###
#
# libitcSmartContainerTest_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcSmartContainerTest_a_SOURCES = $(libitcSmartContainerTest_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcSmartContainerTest_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###