noinst_LIBRARIES += libitcTransportLocalTest.a
itc_platform_unittest_LDADD += libitcTransportLocalTest.a
TEST_SUITES_ADD += -Wl,libitcTransportLocalTest.a

libitcTransportLocalTest_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc


libitcTransportLocalTest_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/itcTransportLocalTest/itcTransportLocalTest.cc

###
#
# libitcTransportLocalTest_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcTransportLocalTest_a_SOURCES = $(libitcTransportLocalTest_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcTransportLocalTest_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###