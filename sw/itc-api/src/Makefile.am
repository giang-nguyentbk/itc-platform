noinst_LTLIBRARIES += libitcapi.la
libitcplatform_la_LIBADD += libitcapi.la

libitcapi_la_CPPFLAGS	= \
				$(AM_CPPFLAGS) \
				-I$(abs_top_srcdir)/sw/itc-common/if \
				-I$(abs_top_srcdir)/sw/itc-common/inc \
				-I$(abs_top_srcdir)/sw/itc-api/if \
				-I$(abs_top_srcdir)/sw/itc-api/inc

itcapi_COMMON_SOURCES 	= \
				sw/itc-api/src/itcImpl.cc

###
#
# itcapi_TARGET1_SOURCES	= \
#				sw/itc-common/src/...
#
###

libitcapi_la_SOURCES = $(itcapi_COMMON_SOURCES)

###
#
# if ENABLE_TARGET1
# 	libitcapi_la_SOURCES += $(itcapi_TARGET1_SOURCES)
# endif
#
###