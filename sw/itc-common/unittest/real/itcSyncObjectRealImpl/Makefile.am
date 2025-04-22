noinst_LIBRARIES += libitcSyncObjectRealImpl.a
itc_platform_unittest_LDADD += libitcSyncObjectRealImpl.a
TEST_SUITES_ADD += -Wl,libitcSyncObjectRealImpl.a

libitcSyncObjectRealImpl_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc

# if ENABLE_TEST_COVERAGE_YES
# libitcSyncObjectRealImpl_a_CPPFLAGS += -fprofile-arcs -ftest-coverage --coverage -O0 -g
# endif

libitcSyncObjectRealImpl_a_COMMON_SOURCES 	= \
				sw/itc-common/src/itcSyncObject.cc

###
#
# libitcSyncObjectRealImpl_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcSyncObjectRealImpl_a_SOURCES = $(libitcSyncObjectRealImpl_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcSyncObjectRealImpl_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###