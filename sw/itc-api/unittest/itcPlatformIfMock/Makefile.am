noinst_LIBRARIES += libitcPlatformIfMock.a
itc_platform_unittest_LDADD += libitcPlatformIfMock.a
TEST_SUITES_ADD += -Wl,libitcPlatformIfMock.a

libitcPlatformIfMock_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc \
				-I$(abs_top_srcdir)/sw/itc-api/unittest/itcPlatformIfMock

libitcPlatformIfMock_a_COMMON_SOURCES 	= \
				sw/itc-api/unittest/itcPlatformIfMock/itcPlatformIfMock.cc

###
#
# libitcPlatformIfMock_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcPlatformIfMock_a_SOURCES = $(libitcPlatformIfMock_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcPlatformIfMock_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###