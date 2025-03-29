noinst_LIBRARIES += libitcTransportLSocketTest.a
itc_platform_unittest_LDADD += libitcTransportLSocketTest.a
TEST_SUITES_ADD += -Wl,libitcTransportLSocketTest.a

noinst_LIBRARIES += libitcTransportLSocket.a
itc_platform_unittest_LDADD += libitcTransportLSocket.a
TEST_SUITES_ADD += -Wl,libitcTransportLSocket.a

libitcTransportLSocketTest_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-common/unittest/itcFileSystemIfMock \
				-I$(abs_top_srcdir)/sw/itc-common/unittest/itcCWrapperIfMock

libitcTransportLSocket_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if

if ENABLE_TEST_COVERAGE_YES
libitcTransportLSocket_a_CPPFLAGS += -fprofile-arcs -ftest-coverage --coverage -O0 -g
endif

libitcTransportLSocketTest_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/itcTransportLSocketTest/itcTransportLSocketTest.cc

libitcTransportLSocket_a_COMMON_SOURCES 	= \
				sw/itc-common/src/itcTransportLSocket.cc

###
#
# libitcTransportLSocketTest_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcTransportLSocketTest_a_SOURCES = $(libitcTransportLSocketTest_a_COMMON_SOURCES)
libitcTransportLSocket_a_SOURCES = $(libitcTransportLSocket_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcTransportLSocketTest_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###