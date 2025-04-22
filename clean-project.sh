#!/bin/bash

TO_BE_REMOVED_IN_ROOT_DIR=".libs .vscode autom4te.cache m4 aclocal.m4 ar-lib compile config.* configure configure~ depcomp install-sh libtool ltmain.sh Makefile Makefile.in missing lib*.la lib*.a"
TO_BE_REMOVED_IN_UNITTEST_DIR="unittest/test-results/* unittest/valgrind/* unittest/coverage/*"
TO_BE_REMOVED_TEST_COVERAGE_EXTRA_FILES="sw/itc-api/src/*.gcda sw/itc-api/src/*.gcno sw/itc-common/src/*.gcda sw/itc-common/src/*.gcno"

TO_BE_REMOVED="$TO_BE_REMOVED_IN_ROOT_DIR $TO_BE_REMOVED_IN_UNITTEST_DIR $TO_BE_REMOVED_TEST_COVERAGE_EXTRA_FILES"

make -k clean 2>/dev/null || true
rm -rf $TO_BE_REMOVED
rm -rf $(find . -type d -name ".deps") 2>/dev/null
rm -rf $(find . -type d -name ".libs") 2>/dev/null
rm -f $(find . -type f -name ".dirstamp") 2>/dev/null