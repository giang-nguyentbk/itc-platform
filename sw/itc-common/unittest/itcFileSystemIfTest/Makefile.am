noinst_LIBRARIES += libitcFileSystemIfTest.a
itc_platform_unittest_LDADD += libitcFileSystemIfTest.a
TEST_SUITES_ADD += -Wl,libitcFileSystemIfTest.a

noinst_LIBRARIES += libitcFileSystemIf.a
itc_platform_unittest_LDADD += libitcFileSystemIf.a
TEST_SUITES_ADD += -Wl,libitcFileSystemIf.a

libitcFileSystemIfTest_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-common/unittest/itcCWrapperIfMock

libitcFileSystemIf_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-DUNITTEST

if ENABLE_TEST_COVERAGE_YES
libitcFileSystemIf_a_CPPFLAGS += -fprofile-arcs -ftest-coverage --coverage -O0 -g
endif

libitcFileSystemIfTest_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/itcFileSystemIfTest/itcFileSystemIfTest.cc

libitcFileSystemIf_a_COMMON_SOURCES 	= \
				sw/itc-common/src/itcFileSystem.cc

###
#
# libitcFileSystemIfTest_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcFileSystemIfTest_a_SOURCES = $(libitcFileSystemIfTest_a_COMMON_SOURCES)
libitcFileSystemIf_a_SOURCES = $(libitcFileSystemIf_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcFileSystemIfTest_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###