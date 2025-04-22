noinst_LIBRARIES += libitcLockFreeHashMapTest.a
itc_platform_unittest_LDADD += libitcLockFreeHashMapTest.a
TEST_SUITES_ADD += -Wl,libitcLockFreeHashMapTest.a

libitcLockFreeHashMapTest_a_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc

libitcLockFreeHashMapTest_a_COMMON_SOURCES 	= \
				sw/itc-common/unittest/itcLockFreeHashMapTest/itcLockFreeHashMapTest.cc

###
#
# libitcLockFreeHashMapTest_a_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcLockFreeHashMapTest_a_SOURCES = $(libitcLockFreeHashMapTest_a_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcLockFreeHashMapTest_a_SOURCES += $(itccommon_TARGET1_SOURCES)
# endif
#
###