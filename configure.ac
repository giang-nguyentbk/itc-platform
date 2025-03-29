AC_INIT([itcplatform], [0.1])

AM_INIT_AUTOMAKE([1.9 foreign subdir-objects])

AM_SILENT_RULES([yes])

AM_PROG_AR
LT_INIT
AC_PROG_CXX
AC_PATH_PROG([GCOV], [gcov], [true])
AC_PATH_PROG([LCOV], [lcov], [true])

AC_ARG_VAR(BUILD_TARGET, [choose: target1 target2])
AC_MSG_NOTICE([............checking BUILD_TARGET: $BUILD_TARGET............])
AS_IF([test "x$BUILD_TARGET" = x], [AC_MSG_ERROR([No BUILD_TARGET was given!])])

check_build_target()
{
    for t in "$@"
    do
        test "x$BUILD_TARGET" = "x$t" && return 0
    done
    return 1
}

AM_CONDITIONAL([BUILD_TARGET_TARGET1], [check_build_target target1])
AM_CONDITIONAL([BUILD_TARGET_TARGET2], [check_build_target target2])

AC_ARG_VAR(BUILD_TYPE, [choose: debug release])
AC_MSG_NOTICE([............checking BUILD_TYPE: $BUILD_TYPE............])
AS_IF([test "x$BUILD_TYPE" = x], [AC_MSG_ERROR([No BUILD_TYPE was given!])])
check_build_type()
{
    for t in "$@"
    do
        test "x$BUILD_TYPE" = "x$t" && return 0
    done
    return 1
}
AM_CONDITIONAL([BUILD_TYPE_DEBUG], [check_build_type debug])
AM_CONDITIONAL([BUILD_TYPE_RELEASE], [check_build_type release])

AC_ARG_VAR(ENABLE_UNITTEST, [choose: all partial])
AC_MSG_NOTICE([............checking ENABLE_UNITTEST: $ENABLE_UNITTEST............])
check_enable_unittest()
{
    for t in "$@"
    do
        test "x$ENABLE_UNITTEST" = "x$t" && return 0
    done
    return 1
}
AM_CONDITIONAL([ENABLE_UNITTEST_ALL], [check_enable_unittest all])
AM_CONDITIONAL([ENABLE_UNITTEST_PARTIAL], [check_enable_unittest partial])
AM_CONDITIONAL([ENABLE_UNITTEST_YES],
               [test "$ENABLE_UNITTEST_ALL" != "no" -o "$ENABLE_UNITTEST_PARTIAL" != "no"])

AC_ARG_VAR(ENABLE_VALGRIND, [choose: yes])
AC_MSG_NOTICE([............checking ENABLE_VALGRIND: $ENABLE_VALGRIND............])
check_enable_valgrind()
{
    for t in "$@"
    do
        test "x$ENABLE_VALGRIND" = "x$t" && return 0
    done
    return 1
}
AM_CONDITIONAL([ENABLE_VALGRIND_YES], [check_enable_valgrind yes])

AC_ARG_VAR(ENABLE_TEST_COVERAGE, [choose: yes])
AC_MSG_NOTICE([............checking ENABLE_TEST_COVERAGE: $ENABLE_TEST_COVERAGE............])
check_enable_test_coverage()
{
    for t in "$@"
    do
        test "x$ENABLE_TEST_COVERAGE" = "x$t" && return 0
    done
    return 1
}
AM_CONDITIONAL([ENABLE_TEST_COVERAGE_YES], [check_enable_test_coverage yes])

AM_CFLAGS="$AM_CFLAGS -std=c23 -Wall -Werror -Wno-unused-parameter -Wextra -pedantic"
AC_SUBST([AM_CFLAGS])

AM_CXXFLAGS="$AM_CXXFLAGS -std=c++20 -Wall -Werror -Wno-unused-parameter -Wextra -pedantic"
AC_SUBST([AM_CXXFLAGS])

ARFLAGS=cr
AC_SUBST([ARFLAGS])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_MACRO_DIRS([m4])

AC_OUTPUT