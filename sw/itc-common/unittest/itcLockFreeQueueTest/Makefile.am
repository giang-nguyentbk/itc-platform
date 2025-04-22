noinst_LIBRARIES += libitcLockFreeQueueTest.a
itc_platform_unittest_LDADD += libitcLockFreeQueueTest.a
TEST_SUITES_ADD += -Wl,libitcLockFreeQueueTest.a

libitcLockFreeQueueTest_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc

libitcLockFreeQueueTest_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/itcLockFreeQueueTest/itcLockFreeQueueTest.cc

###
#
# libitcLockFreeQueueTest_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcLockFreeQueueTest_a_SOURCES = $(libitcLockFreeQueueTest_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcLockFreeQueueTest_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###