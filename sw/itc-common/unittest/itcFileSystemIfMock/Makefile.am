noinst_LIBRARIES += libitcFileSystemIfMock.a
itc_platform_unittest_LDADD += libitcFileSystemIfMock.a
TEST_SUITES_ADD += -Wl,libitcFileSystemIfMock.a

libitcFileSystemIfMock_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-common/unittest/itcFileSystemIfMock

libitcFileSystemIfMock_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/itcFileSystemIfMock/itcFileSystemIfMock.cc

###
#
# libitcFileSystemIfMock_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcFileSystemIfMock_a_SOURCES = $(libitcFileSystemIfMock_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcFileSystemIfMock_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###