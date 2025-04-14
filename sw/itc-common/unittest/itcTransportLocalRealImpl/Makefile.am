noinst_LIBRARIES += libitcTransportLocalRealImpl.a
itc_platform_unittest_LDADD += libitcTransportLocalRealImpl.a
TEST_SUITES_ADD += -Wl,libitcTransportLocalRealImpl.a

libitcTransportLocalRealImpl_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc

# if ENABLE_TEST_COVERAGE_YES
# libitcTransportLocalRealImpl_a_CPPFLAGS += -fprofile-arcs -ftest-coverage --coverage -O0 -g
# endif

libitcTransportLocalRealImpl_a_COMMON_SOURCES 	= \
				sw/itc-common/src/itcTransportLocal.cc

###
#
# libitcTransportLocalRealImpl_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcTransportLocalRealImpl_a_SOURCES = $(libitcTransportLocalRealImpl_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcTransportLocalRealImpl_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###