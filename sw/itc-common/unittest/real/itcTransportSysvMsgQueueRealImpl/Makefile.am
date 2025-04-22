noinst_LIBRARIES += libitcTransportSysvMsgQueueRealImpl.a
itc_platform_unittest_LDADD += libitcTransportSysvMsgQueueRealImpl.a
TEST_SUITES_ADD += -Wl,libitcTransportSysvMsgQueueRealImpl.a

libitcTransportSysvMsgQueueRealImpl_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc

# if ENABLE_TEST_COVERAGE_YES
# libitcTransportSysvMsgQueueRealImpl_a_CPPFLAGS += -fprofile-arcs -ftest-coverage --coverage -O0 -g
# endif

libitcTransportSysvMsgQueueRealImpl_a_COMMON_SOURCES 	= \
				sw/itc-common/src/itcTransportSysvMsgQueue.cc

###
#
# libitcTransportSysvMsgQueueRealImpl_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcTransportSysvMsgQueueRealImpl_a_SOURCES = $(libitcTransportSysvMsgQueueRealImpl_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcTransportSysvMsgQueueRealImpl_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###