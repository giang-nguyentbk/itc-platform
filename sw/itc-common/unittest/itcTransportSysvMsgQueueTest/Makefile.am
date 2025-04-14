noinst_LIBRARIES += libitcTransportSysvMsgQueueTest.a
itc_platform_unittest_LDADD += libitcTransportSysvMsgQueueTest.a
TEST_SUITES_ADD += -Wl,libitcTransportSysvMsgQueueTest.a

libitcTransportSysvMsgQueueTest_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc \
				-I$(abs_top_srcdir)/sw/itc-common/unittest/itcFileSystemIfMock \
				-I$(abs_top_srcdir)/sw/itc-common/unittest/itcCWrapperIfMock \
				-I$(abs_top_srcdir)/sw/itc-common/unittest/itcThreadManagerIfMock \
				-I$(abs_top_srcdir)/sw/itc-api/unittest/itcPlatformIfMock


libitcTransportSysvMsgQueueTest_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/itcTransportSysvMsgQueueTest/itcTransportSysvMsgQueueTest.cc

###
#
# libitcTransportSysvMsgQueueTest_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcTransportSysvMsgQueueTest_a_SOURCES = $(libitcTransportSysvMsgQueueTest_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcTransportSysvMsgQueueTest_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###