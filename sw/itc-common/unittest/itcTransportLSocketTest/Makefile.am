noinst_LIBRARIES += libitcTransportLSocketTest.a
itc_platform_unittest_LDADD += libitcTransportLSocketTest.a
TEST_SUITES_ADD += -Wl,libitcTransportLSocketTest.a

libitcTransportLSocketTest_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc \
				-I$(abs_top_srcdir)/sw/itc-common/unittest/itcFileSystemIfMock \
				-I$(abs_top_srcdir)/sw/itc-common/unittest/itcCWrapperIfMock


libitcTransportLSocketTest_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/itcTransportLSocketTest/itcTransportLSocketTest.cc

###
#
# libitcTransportLSocketTest_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcTransportLSocketTest_a_SOURCES = $(libitcTransportLSocketTest_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcTransportLSocketTest_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###