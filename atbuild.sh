# UNITTEST BUILD
# make clean && libtoolize && aclocal && autoconf && automake --add-missing && autoreconf -fi && ./configure BUILD_TARGET=target1 BUILD_TYPE=unittest UNITTEST_TYPE=$1 && make && valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all --xml=yes --xml-file=unittest/valgrind/memory_leak_report.xml ./itc_platform_unittest
# make clean && aclocal && autoconf && automake --add-missing && autoreconf -fi && ./configure BUILD_TARGET=target1 BUILD_TYPE=unittest UNITTEST_TYPE=$1 && make && ./itc_platform_unittest
# make clean && aclocal && autoconf && automake --add-missing && autoreconf -fi && ./configure BUILD_TARGET=target1 BUILD_TYPE=unittest UNITTEST_TYPE=$1 && make && valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all ./itc_platform_unittest > unittest/test-results/itc_platform_unittest.log 2>&1

# for i in {1..50}; do ./itc_platform_unittest; done > unittest/test-results/itc_platform_unittest.log 2>&1

# make clean && rm -rf unittest/test-results/itc_platform_unittest.log

#make clean && aclocal && autoconf && automake --add-missing && autoreconf -fi && ./configure BUILD_TARGET=target1 BUILD_TYPE=unittest UNITTEST_TYPE=$1 --host=arm-linux-gnueabi && make && valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all ./itc_platform_unittest
# make clean && aclocal && autoconf && automake --add-missing && autoreconf -fi && ./configure BUILD_TARGET=target1 BUILD_TYPE=unittest UNITTEST_TYPE=$1 --host=i686-pc-linux-gnu && make && valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all ./itc_platform_unittest
# make clean && aclocal && autoconf && automake --add-missing && autoreconf -fi && ./configure BUILD_TARGET=target1 BUILD_TYPE=unittest UNITTEST_TYPE=$1 --host=x86_64-pc-linux-gnu && make && valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all ./itc_platform_unittest
# make clean && aclocal && autoconf && automake --add-missing && autoreconf -fi && ./configure BUILD_TARGET=target1 BUILD_TYPE=unittest UNITTEST_TYPE=$1 --host=armv7-eabi && make && valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all ./itc_platform_unittest
# make clean && aclocal && autoconf && automake --add-missing && autoreconf -fi && ./configure BUILD_TARGET=target1 BUILD_TYPE=unittest UNITTEST_TYPE=$1 --host=aarch64-linux-gnu && make && valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all ./itc_platform_unittest

# PRODUCTION BUILD DEBUG - TARGET1
# make clean && aclocal && autoconf && automake --add-missing && autoreconf -fi && ./configure BUILD_TARGET=target1 BUILD_TYPE=debug && make && make clean

# PRODUCTION BUILD RELEASE - TARGET1
# make clean && aclocal && autoconf && automake --add-missing && autoreconf -fi && ./configure BUILD_TARGET=target1 BUILD_TYPE=release && make && make clean


#!/bin/bash

# Clean up
rm -rf unittest/test-results/* unittest/valgrind/* unittest/coverage/* sw/itc-api/src/*.gcda sw/itc-api/src/*.gcno sw/itc-common/src/*.gcda sw/itc-common/src/*.gcno

# Default values
BUILD_TARGET="target1"
BUILD_TYPE="debug"
ENABLE_UT="no"
ENABLE_VALGRIND="no"
ENABLE_TEST_COVERAGE="no"
AUTO_BUILD_COMMAND="libtoolize && aclocal && autoconf && automake --add-missing && autoreconf -fi && ./configure "

# Parse arguments
for arg in "$@"; do
    case $arg in
        --build-target=*)
            BUILD_TARGET="${arg#*=}"
            shift
            ;;
        --build-type=*)
            BUILD_TYPE="${arg#*=}"
            shift
            ;;
        --enable-ut=*)
            ENABLE_UT="${arg#*=}"
            shift
            ;;
        --enable-valgrind)
            ENABLE_VALGRIND="yes"
            shift
            ;;
        --enable-test-coverage)
            ENABLE_TEST_COVERAGE="yes"
            shift
            ;;
        *)
            echo "[-] Unknown option: $arg"
            exit 1
            ;;
    esac
done

# Print parsed values
echo "[+] Build Target: $BUILD_TARGET"
echo "[+] Enable UT: $ENABLE_UT"
echo "[+] Enable Valgrind: $ENABLE_VALGRIND"
echo "[+] Enable Test Coverage: $ENABLE_TEST_COVERAGE"

# Validate conditions: If valgrind or coverage is enabled but UT is disabled, return an error
if [[ "$ENABLE_UT" != "no" ]] && [[ "$ENABLE_UT" != "all" ]] && [[ "$ENABLE_UT" != "partial" ]]; then
    echo "[-] Error: Unknown ENABLE_TEST_COVERAGE option $ENABLE_TEST_COVERAGE"
    exit 1
fi
if [[ "$ENABLE_TEST_COVERAGE" != "no" ]] && [[ "$ENABLE_TEST_COVERAGE" != "yes" ]]; then
    echo "[-] Error: Unknown ENABLE_TEST_COVERAGE option $ENABLE_TEST_COVERAGE"
    exit 1
fi
if [[ "$ENABLE_VALGRIND" != "no" ]] && [[ "$ENABLE_VALGRIND" != "yes" ]]; then
    echo "[-] Error: Unknown ENABLE_VALGRIND option $ENABLE_VALGRIND"
    exit 1
fi
if [[ "$ENABLE_VALGRIND" == "yes" ]] && [[ "$ENABLE_UT" == "no" ]]; then
    echo "[-] Error: --enable-valgrind=yes requires --enable-ut=yes."
    exit 1
fi
if [[ "$ENABLE_TEST_COVERAGE" == "yes" ]] && [[ "$ENABLE_UT" == "no" ]]; then
    echo "[-] Error: --enable-test-coverage=yes requires --enable-ut=yes."
    exit 1
fi

# Build configure.ac options
AUTO_BUILD_COMMAND+="BUILD_TARGET=$BUILD_TARGET "
AUTO_BUILD_COMMAND+="BUILD_TYPE=$BUILD_TYPE "
if [[ "$ENABLE_UT" == "all" ]]; then
    echo "[+] Enable unit test: all."
    AUTO_BUILD_COMMAND+="ENABLE_UNITTEST=all "
fi
if [[ "$ENABLE_UT" == "partial" ]]; then
    echo "[+] Enable unit test: partial."
    AUTO_BUILD_COMMAND+="ENABLE_UNITTEST=partial "
fi
if [[ "$ENABLE_VALGRIND" == "yes" ]]; then
    echo "[+] Enable valgrind: yes."
    AUTO_BUILD_COMMAND+="ENABLE_VALGRIND=yes "
fi
if [[ "$ENABLE_TEST_COVERAGE" == "yes" ]]; then
    echo "[+] Enable test coverage: $ENABLE_TEST_COVERAGE."
    AUTO_BUILD_COMMAND+="ENABLE_TEST_COVERAGE=yes "
fi

AUTO_BUILD_COMMAND+="&& make "

if [[ "$ENABLE_VALGRIND" == "yes" ]]; then
    echo "[+] Enable valgrind: yes."
    AUTO_BUILD_COMMAND+="&& valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all --xml=yes --xml-file=unittest/valgrind/memory_leak_report.xml ./itc_platform_unittest "
else
    AUTO_BUILD_COMMAND+="&& ./itc_platform_unittest "
fi

AUTO_BUILD_COMMAND+="> unittest/test-results/itc_platform_unittest.log 2>&1 "

if [[ "$ENABLE_TEST_COVERAGE" == "yes" ]]; then
    # Define directories to include
    INCLUDE_DIRS=("sw/itc-api/if" "sw/itc-common/if" "sw/itc-api/inc" "sw/itc-common/inc" "sw/itc-api/src" "sw/itc-common/src")

	# Define directories to exclude
    EXCLUDE_DIRS=("sw/itc-api/unittest" "sw/itc-common/unittest" "/usr/include" "/usr/local/include")

    # Define output directory
    OUTPUT_DIR="unittest/coverage"
    AUTO_BUILD_COMMAND+="&& mkdir -p '$OUTPUT_DIR' "

    # Convert arrays to space-separated strings
    INCLUDE_FLAGS=""
    for dir in "${INCLUDE_DIRS[@]}"; do
        INCLUDE_FLAGS+="--include '$dir/*' "
    done

	# Convert arrays to space-separated strings
    EXCLUDE_FLAGS=""
    for dir in "${EXCLUDE_DIRS[@]}"; do
        EXCLUDE_FLAGS+="--exclude '$dir/*' "
    done

	# NON_PORTABLE_LCOV_IGNORING_OPTIONS="--ignore-errors empty --ignore-errors unused "
	NON_PORTABLE_LCOV_IGNORING_OPTIONS=""
	# NON_PORTABLE_LCOV_FILTERING_OPTION="$INCLUDE_FLAGS"
	NON_PORTABLE_LCOV_FILTERING_OPTION="$EXCLUDE_FLAGS"
    # Run lcov to capture coverage
    AUTO_BUILD_COMMAND+="&& lcov --capture --directory sw/ $NON_PORTABLE_LCOV_FILTERING_OPTION --output-file '$OUTPUT_DIR/coverage.info' $NON_PORTABLE_LCOV_IGNORING_OPTIONS "
    
    # Generate html visualisation
    AUTO_BUILD_COMMAND+="&& genhtml '$OUTPUT_DIR/coverage.info' --output-directory '$OUTPUT_DIR/output' "
fi


# Guide line
if [[ "$ENABLE_UT" != "no" ]]; then
    AUTO_BUILD_COMMAND+="&& echo '============================================================================' "
    AUTO_BUILD_COMMAND+="&& echo '| Unit test logs saved in: unittest/test-results/itc_platform_unittest.log |' "
    AUTO_BUILD_COMMAND+="&& echo '============================================================================' "
fi

if [[ "$ENABLE_VALGRIND" == "yes" ]]; then
    AUTO_BUILD_COMMAND+="&& echo '============================================================================' "
    AUTO_BUILD_COMMAND+="&& echo '| Valgrind report saved in: unittest/valgrind/memory_leak_report.xml       |' "
    AUTO_BUILD_COMMAND+="&& echo '============================================================================' "
fi

if [[ "$ENABLE_TEST_COVERAGE" == "yes" ]]; then
    AUTO_BUILD_COMMAND+="&& echo '============================================================================' "
    AUTO_BUILD_COMMAND+="&& echo '| Coverage info saved in: unittest/coverage/coverage.info                  |' "
    AUTO_BUILD_COMMAND+="&& echo '============================================================================' "
    AUTO_BUILD_COMMAND+="&& echo '| RUN BELOW COMMANDS TO SEE TEST COVERAGE REPORT                           |' "
    AUTO_BUILD_COMMAND+="&& echo '| >> xdg-open unittest/coverage/output/index.html                          |' "
    AUTO_BUILD_COMMAND+="&& echo '============================================================================' "
    AUTO_BUILD_COMMAND+="&& echo '| OR UPLOAD unittest/coverage/coverage.info FILE                           |' "
    AUTO_BUILD_COMMAND+="&& echo '| TO LCOV VIEWER ONLINE TOOL                                               |' "
    AUTO_BUILD_COMMAND+="&& echo '| >> https://lcov-viewer.netlify.app/                                      |' "
    AUTO_BUILD_COMMAND+="&& echo '============================================================================' "
    AUTO_BUILD_COMMAND+="&& lcov -q -l unittest/coverage/coverage.info "
fi

# Print final configuration command
echo "[+] Running: $AUTO_BUILD_COMMAND"

# Execute the configuration command
eval "$AUTO_BUILD_COMMAND"
