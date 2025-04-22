noinst_LIBRARIES += libitcMailboxRealImpl.a
itc_platform_unittest_LDADD += libitcMailboxRealImpl.a
TEST_SUITES_ADD += -Wl,libitcMailboxRealImpl.a

libitcMailboxRealImpl_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc

# if ENABLE_TEST_COVERAGE_YES
# libitcMailboxRealImpl_a_CPPFLAGS += -fprofile-arcs -ftest-coverage --coverage -O0 -g
# endif

libitcMailboxRealImpl_a_COMMON_SOURCES 	= \
				sw/itc-common/src/itcMailbox.cc

###
#
# libitcMailboxRealImpl_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcMailboxRealImpl_a_SOURCES = $(libitcMailboxRealImpl_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcMailboxRealImpl_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###