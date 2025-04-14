noinst_LIBRARIES += libitcTransportLSocketRealImpl.a
itc_platform_unittest_LDADD += libitcTransportLSocketRealImpl.a
TEST_SUITES_ADD += -Wl,libitcTransportLSocketRealImpl.a

libitcTransportLSocketRealImpl_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc

# if ENABLE_TEST_COVERAGE_YES
# libitcTransportLSocketRealImpl_a_CPPFLAGS += -fprofile-arcs -ftest-coverage --coverage -O0 -g
# endif

libitcTransportLSocketRealImpl_a_COMMON_SOURCES 	= \
				sw/itc-common/src/itcTransportLSocket.cc

###
#
# libitcTransportLSocketRealImpl_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcTransportLSocketRealImpl_a_SOURCES = $(libitcTransportLSocketRealImpl_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcTransportLSocketRealImpl_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###