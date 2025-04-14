noinst_LIBRARIES += libitcFileSystemTest.a
itc_platform_unittest_LDADD += libitcFileSystemTest.a
TEST_SUITES_ADD += -Wl,libitcFileSystemTest.a

libitcFileSystemTest_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc \
				-I$(abs_top_srcdir)/sw/itc-common/unittest/itcCWrapperIfMock

libitcFileSystemTest_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/itcFileSystemTest/itcFileSystemTest.cc

###
#
# libitcFileSystemTest_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcFileSystemTest_a_SOURCES = $(libitcFileSystemTest_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcFileSystemTest_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###