noinst_LIBRARIES += libitcMailboxTest.a
itc_platform_unittest_LDADD += libitcMailboxTest.a
TEST_SUITES_ADD += -Wl,libitcMailboxTest.a

libitcMailboxTest_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc


libitcMailboxTest_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/itcMailboxTest/itcMailboxTest.cc

###
#
# libitcMailboxTest_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcMailboxTest_a_SOURCES = $(libitcMailboxTest_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcMailboxTest_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###