noinst_LIBRARIES += libitcMutexRealImpl.a
itc_platform_unittest_LDADD += libitcMutexRealImpl.a
TEST_SUITES_ADD += -Wl,libitcMutexRealImpl.a

libitcMutexRealImpl_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc


# if ENABLE_TEST_COVERAGE_YES
# libitcMutexRealImpl_a_CPPFLAGS += -fprofile-arcs -ftest-coverage --coverage -O0 -g
# endif

libitcMutexRealImpl_a_COMMON_SOURCES 	= \
				sw/itc-common/src/itcMutex.cc

###
#
# libitcMutexRealImpl_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcMutexRealImpl_a_SOURCES = $(libitcMutexRealImpl_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcMutexRealImpl_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###