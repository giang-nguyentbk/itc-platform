noinst_LIBRARIES += libitcThreadManagerIfMock.a
itc_platform_unittest_LDADD += libitcThreadManagerIfMock.a
TEST_SUITES_ADD += -Wl,libitcThreadManagerIfMock.a

libitcThreadManagerIfMock_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc
				-I$(abs_top_srcdir)/sw/itc-common/unittest/mock/itcThreadManagerIfMock

libitcThreadManagerIfMock_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/mock/itcThreadManagerIfMock/itcThreadManagerIfMock.cc

###
#
# libitcThreadManagerIfMock_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcThreadManagerIfMock_a_SOURCES = $(libitcThreadManagerIfMock_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcThreadManagerIfMock_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###