noinst_LIBRARIES += libitcPlatformIfTest.a
itc_platform_unittest_LDADD += libitcPlatformIfTest.a
TEST_SUITES_ADD += -Wl,libitcPlatformIfTest.a

noinst_LIBRARIES += libitcPlatformIf.a
itc_platform_unittest_LDADD += libitcPlatformIf.a
TEST_SUITES_ADD += -Wl,libitcPlatformIf.a

libitcPlatformIfTest_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc \
				-I$(abs_top_srcdir)/sw/itc-common/unittest/itcFileSystemIfMock

libitcPlatformIf_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc \
				-DUNITTEST

if ENABLE_TEST_COVERAGE_YES
libitcPlatformIf_a_CPPFLAGS += -fprofile-arcs -ftest-coverage --coverage -O0 -g
endif

libitcPlatformIfTest_a_COMMON_SOURCES 	= \
				sw/itc-api/unittest/itcPlatformIfTest/itcPlatformIfTest.cc

libitcPlatformIf_a_COMMON_SOURCES 	= \
				sw/itc-api/src/itcPlatform.cc

###
#
# libitcPlatformIfTest_a_TARGET1_SOURCES	= \
#				sw/itc-api/src/...
#
###

libitcPlatformIfTest_a_SOURCES = $(libitcPlatformIfTest_a_COMMON_SOURCES)
libitcPlatformIf_a_SOURCES = $(libitcPlatformIf_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcPlatformIfTest_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###