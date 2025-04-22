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

AC_ARG_VAR(BUILD_TYPE, [choose: debug release unittest])
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
AM_CONDITIONAL([BUILD_TYPE_UNITTEST], [check_build_type unittest])

ASAN_FLAGS=""
TSAN_FLAGS=""
if test "x$BUILD_TYPE" = "xunittest"; then
    AC_ARG_VAR(ENABLE_UT_SPECIFIC_TEST, [choose: yes])
    AC_MSG_NOTICE([............checking ENABLE_UT_SPECIFIC_TEST: $ENABLE_UT_SPECIFIC_TEST............])
    check_enable_ut_specific_test()
    {
        for t in "$@"
        do
            test "x$ENABLE_UT_SPECIFIC_TEST" = "x$t" && return 0
        done
        return 1
    }
    AM_CONDITIONAL([ENABLE_UT_SPECIFIC_TEST_YES], [check_enable_ut_specific_test yes])

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

    AC_ARG_ENABLE([address-sanitizer],
        [AS_HELP_STRING([--enable-address-sanitizer], [Enable Address-Sanitizer (default: no)])],
        [enable_address_sanitizer=$enableval],
        [enable_address_sanitizer=no]
    )

    AC_ARG_ENABLE([thread-sanitizer],
        [AS_HELP_STRING([--enable-thread-sanitizer], [Enable Thread-Sanitizer (default: no)])],
        [enable_thread_sanitizer=$enableval],
        [enable_thread_sanitizer=no]
    )

    if test "$enable_address_sanitizer" = yes; then
        ASAN_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g -O1 -fno-optimize-sibling-calls -fno-ipa-icf"
        AM_CONDITIONAL([ENABLE_ADDRESS_SANITIZER_YES], [true])
    else
        AM_CONDITIONAL([ENABLE_ADDRESS_SANITIZER_YES], [false])
    fi

    if test "$enable_thread_sanitizer" = yes; then
        TSAN_FLAGS="-fsanitize=thread -g -O1 -fno-omit-frame-pointer -fPIE"
        AM_CONDITIONAL([ENABLE_THREAD_SANITIZER_YES], [true])
    else
        AM_CONDITIONAL([ENABLE_THREAD_SANITIZER_YES], [false])
    fi
else
AM_CONDITIONAL([ENABLE_UT_SPECIFIC_TEST_YES], [false])
AM_CONDITIONAL([ENABLE_VALGRIND_YES], [false])
AM_CONDITIONAL([ENABLE_TEST_COVERAGE_YES], [false])
AM_CONDITIONAL([ENABLE_ADDRESS_SANITIZER_YES], [false])
AM_CONDITIONAL([ENABLE_THREAD_SANITIZER_YES], [false])
fi

AM_CFLAGS="$AM_CFLAGS $ASAN_FLAGS $TSAN_FLAGS -std=c23 -Wall -Werror -Wno-unused-parameter -Wextra -pedantic"
AC_SUBST([AM_CFLAGS])

AM_CXXFLAGS="$AM_CXXFLAGS $ASAN_FLAGS $TSAN_FLAGS -std=c++20 -Wall -Werror -Wno-unused-parameter -Wextra -pedantic"
AC_SUBST([AM_CXXFLAGS])

ARFLAGS=cr
AC_SUBST([ARFLAGS])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_MACRO_DIRS([m4])

AC_OUTPUT