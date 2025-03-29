noinst_LIBRARIES += libitcTransportSysvMsgQueueTest.a
itc_platform_unittest_LDADD += libitcTransportSysvMsgQueueTest.a
TEST_SUITES_ADD += -Wl,libitcTransportSysvMsgQueueTest.a

noinst_LIBRARIES += libitcTransportSysvMsgQueue.a
itc_platform_unittest_LDADD += libitcTransportSysvMsgQueue.a
TEST_SUITES_ADD += -Wl,libitcTransportSysvMsgQueue.a

libitcTransportSysvMsgQueueTest_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-common/unittest/itcFileSystemIfMock \
				-I$(abs_top_srcdir)/sw/itc-common/unittest/itcCWrapperIfMock \
				-I$(abs_top_srcdir)/sw/itc-common/unittest/itcThreadManagerIfMock \
				-I$(abs_top_srcdir)/sw/itc-api/unittest/itcPlatformIfMock

libitcTransportSysvMsgQueue_a_CPPFLAGS = \
							$(AM_CPPFLAGS) \
							-I$(abs_top_srcdir)/sw/itc-common/if \
							-I$(abs_top_srcdir)/sw/itc-common/inc \
							-I$(abs_top_srcdir)/sw/itc-api/if

if ENABLE_TEST_COVERAGE_YES
libitcTransportSysvMsgQueue_a_CPPFLAGS += -fprofile-arcs -ftest-coverage --coverage -O0 -g
endif

libitcTransportSysvMsgQueueTest_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/itcTransportSysvMsgQueueTest/itcTransportSysvMsgQueueTest.cc

libitcTransportSysvMsgQueue_a_COMMON_SOURCES 	= \
				sw/itc-common/src/itcTransportSysvMsgQueue.cc

###
#
# libitcTransportSysvMsgQueueTest_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcTransportSysvMsgQueueTest_a_SOURCES = $(libitcTransportSysvMsgQueueTest_a_COMMON_SOURCES)
libitcTransportSysvMsgQueue_a_SOURCES = $(libitcTransportSysvMsgQueue_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcTransportSysvMsgQueueTest_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###