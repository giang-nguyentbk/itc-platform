noinst_LIBRARIES += libitcCWrapperIfMock.a
itc_platform_unittest_LDADD += libitcCWrapperIfMock.a
TEST_SUITES_ADD += -Wl,libitcCWrapperIfMock.a

libitcCWrapperIfMock_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-common/unittest/itcCWrapperIfMock

libitcCWrapperIfMock_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/itcCWrapperIfMock/itcCWrapperIfMock.cc

###
#
# libitcCWrapperIfMock_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcCWrapperIfMock_a_SOURCES = $(libitcCWrapperIfMock_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcCWrapperIfMock_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###