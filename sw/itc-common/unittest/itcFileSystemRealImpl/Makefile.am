noinst_LIBRARIES += libitcFileSystemRealImpl.a
itc_platform_unittest_LDADD += libitcFileSystemRealImpl.a
TEST_SUITES_ADD += -Wl,libitcFileSystemRealImpl.a

libitcFileSystemRealImpl_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-DUNITTEST \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc

# if ENABLE_TEST_COVERAGE_YES
# libitcFileSystemRealImpl_a_CPPFLAGS += -fprofile-arcs -ftest-coverage --coverage -O0 -g
# endif

libitcFileSystemRealImpl_a_COMMON_SOURCES 	= \
				sw/itc-common/src/itcFileSystem.cc

###
#
# libitcFileSystemRealImpl_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcFileSystemRealImpl_a_SOURCES = $(libitcFileSystemRealImpl_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcFileSystemRealImpl_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###