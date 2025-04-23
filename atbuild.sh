#!/bin/bash

# Variables
UNITTEST_PROGRAM="./itc_platform_unittest"
UNITTEST_LOG="unittest/test-results/itc_platform_unittest.log"
VALGRIND_REPORT_XML="unittest/valgrind/memory_leak_report.xml"
TEST_COVERAGE_INFO="unittest/coverage/coverage.info"
TEST_COVERAGE_OUTPUT="unittest/coverage/output"

# architectures["product_name"]="target_architecture"
declare -A architectures
architectures["target1"]="aarch64-linux-gnu"
architectures["target2"]="x86_64-pc-linux-gnu"

# Default values
BUILD_TARGET="target1"
BUILD_TYPE="debug"
UT_REPEAT_COUNT=1
ENABLE_UT_SPECIFIC_TEST="no"
ENABLE_VALGRIND="no"
ENABLE_TEST_COVERAGE="no"
TEST_COVERAGE_RATE="92"
ENABLE_REBUILD="no"
ENABLE_ADDRESS_SANITIZER=""
ENABLE_THREAD_SANITIZER=""

# Parse arguments
for arg in "$@"; do
    case $arg in
        --build-target-*)
            BUILD_TARGET="${arg#*--build-target-}"
            shift
            ;;
        --build-type-*)
            BUILD_TYPE="${arg#*--build-type-}"
            shift
            ;;
        --ut-repeat-count=*)
            UT_REPEAT_COUNT="${arg#*=}"
            shift
            ;;
        --enable-ut-specific-test)
            ENABLE_UT_SPECIFIC_TEST="yes"
            shift
            ;;
        --enable-valgrind)
            ENABLE_VALGRIND="yes"
            shift
            ;;
        --enable-test-coverage)
            ENABLE_TEST_COVERAGE="yes"
            # If enable test coverage, forcibly rebuild because compile flags are needed
            ENABLE_REBUILD="yes"
            shift
            ;;
        --test-coverage-rate=*)
            TEST_COVERAGE_RATE="${arg#*=}"
            shift
            ;;
        --rebuild)
            ENABLE_REBUILD="yes"
            shift
            ;;
        --enable-address-sanitizer)
            ENABLE_ADDRESS_SANITIZER="--enable-address-sanitizer"
            shift
            ;;
        --enable-thread-sanitizer)
            ENABLE_THREAD_SANITIZER="--enable-thread-sanitizer"
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
echo "[+] Target Architecture: ${architectures[$BUILD_TARGET]}"
echo "[+] Build Type: $BUILD_TYPE"
echo "[+] UT Repeat Count: $UT_REPEAT_COUNT"
echo "[+] Enable UT Specific Test: $ENABLE_UT_SPECIFIC_TEST"
echo "[+] Enable Valgrind: $ENABLE_VALGRIND"
echo "[+] Enable Test Coverage: $ENABLE_TEST_COVERAGE"
echo "[+] Test Coverage Rate: $TEST_COVERAGE_RATE"
echo "[+] Enable Rebuild: $ENABLE_REBUILD"
echo "[+] Enable Address Sanitizer: $ENABLE_ADDRESS_SANITIZER"
echo "[+] Enable Thread Sanitizer: $ENABLE_THREAD_SANITIZER"

# Validate conditions: If valgrind or coverage is enabled but UT is disabled, return an error
if  [[ "$BUILD_TARGET" != "target1" ]] &&
    [[ "$BUILD_TARGET" != "target2" ]]; then
    echo "[-] Error: Unknown BUILD_TARGET option: $BUILD_TARGET"
    exit 1
fi
if  [[ "$BUILD_TYPE" != "debug" ]] &&
    [[ "$BUILD_TYPE" != "release" ]] &&
    [[ "$BUILD_TYPE" != "unittest" ]]; then
    echo "[-] Error: Unknown BUILD_TYPE option: $BUILD_TYPE"
    exit 1
fi
if  [[ "$ENABLE_UT_SPECIFIC_TEST" != "no" ]] &&
    [[ "$ENABLE_UT_SPECIFIC_TEST" != "yes" ]]; then
    echo "[-] Error: Unknown ENABLE_UT_SPECIFIC_TEST option: $ENABLE_UT_SPECIFIC_TEST"
    exit 1
fi
if  [[ "$ENABLE_VALGRIND" != "no" ]] &&
    [[ "$ENABLE_VALGRIND" != "yes" ]]; then
    echo "[-] Error: Unknown ENABLE_VALGRIND option: $ENABLE_VALGRIND"
    exit 1
fi
if  [[ "$ENABLE_TEST_COVERAGE" != "no" ]] &&
    [[ "$ENABLE_TEST_COVERAGE" != "yes" ]]; then
    echo "[-] Error: Unknown ENABLE_TEST_COVERAGE option: $ENABLE_TEST_COVERAGE"
    exit 1
fi
if ! [[ "$TEST_COVERAGE_RATE" =~ ^[0-9]+([.][0-9]+)?$ ]]; then
    echo "[-] Error: Invalid TEST_COVERAGE_RATE: $TEST_COVERAGE_RATE (not a floating point or decimal value)"
    exit 1
fi
if  [[ "$ENABLE_REBUILD" != "no" ]] &&
    [[ "$ENABLE_REBUILD" != "yes" ]]; then
    echo "[-] Error: Unknown ENABLE_REBUILD option: $ENABLE_REBUILD"
    exit 1
fi
if  [[ "$ENABLE_ADDRESS_SANITIZER" != "" ]] &&
    [[ "$ENABLE_ADDRESS_SANITIZER" != "--enable-address-sanitizer" ]]; then
    echo "[-] Error: Unknown ENABLE_ADDRESS_SANITIZER option: $ENABLE_ADDRESS_SANITIZER"
    exit 1
fi
if  [[ "$ENABLE_THREAD_SANITIZER" != "" ]] &&
    [[ "$ENABLE_THREAD_SANITIZER" != "--enable-thread-sanitizer" ]]; then
    echo "[-] Error: Unknown ENABLE_THREAD_SANITIZER option: $ENABLE_THREAD_SANITIZER"
    exit 1
fi
if  [[ "$ENABLE_ADDRESS_SANITIZER" == "--enable-address-sanitizer" ]] &&
    [[ "$ENABLE_THREAD_SANITIZER" == "--enable-thread-sanitizer" ]]; then
    echo "[-] Error: Please choose either --enable-address-sanitizer or --enable-thread-sanitizer only!"
    exit 1
fi
if  [[ "$ENABLE_VALGRIND" == "yes" ]] && { [[ "$ENABLE_ADDRESS_SANITIZER" == "--enable-address-sanitizer" ]] ||
    [[ "$ENABLE_THREAD_SANITIZER" == "--enable-thread-sanitizer" ]]; } then
    echo "[-] Error: --enable-valgrind cannot be used with neither --enable-address-sanitizer nor --enable-thread-sanitizer!"
    exit 1
fi
if { [[ "$ENABLE_VALGRIND" == "yes" ]] || [[ "$ENABLE_TEST_COVERAGE" == "yes" ]] || [[ "$ENABLE_ADDRESS_SANITIZER" == "--enable-address-sanitizer" ]] || [[ "$ENABLE_THREAD_SANITIZER" == "--enable-thread-sanitizer" ]]; } &&
   [[ "$BUILD_TYPE" != "unittest" ]] ; then
    echo "[-] Error: --enable-valgrind or --enable-test-coverage requires --build-type-unittest"
    exit 1
fi

TO_BE_REMOVED="./itc_platform_unittest unittest/test-results/* unittest/valgrind/* unittest/coverage/* sw/itc-api/src/*.gcda sw/itc-api/src/*.gcno sw/itc-common/src/*.gcda sw/itc-common/src/*.gcno"

# Build configure.ac options
AUTO_BUILD_COMMAND=""
AUTO_BUILD_COMMAND+="echo '================== CLEANUP START ==================' "
AUTO_BUILD_COMMAND+="&& rm -rf $TO_BE_REMOVED "
if [[ "$ENABLE_REBUILD" == "yes" ]]; then
    AUTO_BUILD_COMMAND+="&& make -k clean 2>/dev/null || true "
fi
AUTO_BUILD_COMMAND+="&& echo '================== CLEANUP DONE ==================' "
AUTO_BUILD_COMMAND+="&& libtoolize && aclocal && autoconf && automake --add-missing && autoreconf -fi && ./configure "
AUTO_BUILD_COMMAND+="BUILD_TARGET=$BUILD_TARGET "
AUTO_BUILD_COMMAND+="--host=${architectures[$BUILD_TARGET]} "
AUTO_BUILD_COMMAND+="BUILD_TYPE=$BUILD_TYPE "
AUTO_BUILD_COMMAND+="ENABLE_UT_SPECIFIC_TEST=$ENABLE_UT_SPECIFIC_TEST "
AUTO_BUILD_COMMAND+="ENABLE_VALGRIND=$ENABLE_VALGRIND "
AUTO_BUILD_COMMAND+="ENABLE_TEST_COVERAGE=$ENABLE_TEST_COVERAGE "
AUTO_BUILD_COMMAND+="$ENABLE_ADDRESS_SANITIZER "
AUTO_BUILD_COMMAND+="$ENABLE_THREAD_SANITIZER "
AUTO_BUILD_COMMAND+="&& echo '================== COMPILATION START ==================' "
AUTO_BUILD_COMMAND+="&& make "
AUTO_BUILD_COMMAND+="&& echo '================== COMPILATION DONE ==================' "

if  [[ "$BUILD_TYPE" == "unittest" ]]; then
    AUTO_BUILD_COMMAND+="&& echo '================== UNIT TEST START ==================' "
    AUTO_BUILD_COMMAND+="&& for i in {1..$UT_REPEAT_COUNT}; do "
    
    if [[ "$ENABLE_VALGRIND" == "yes" ]]; then
        AUTO_BUILD_COMMAND+="valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all --xml=yes --xml-file=$VALGRIND_REPORT_XML "
    fi
    if [[ "$ENABLE_THREAD_SANITIZER" == "--enable-thread-sanitizer" ]]; then
        AUTO_BUILD_COMMAND+="TSAN_OPTIONS=\"verbosity=3\" setarch \`uname -m\` -R "
    fi
    
    AUTO_BUILD_COMMAND+="$UNITTEST_PROGRAM; done > $UNITTEST_LOG 2>&1 || true "
    
    if [[ "$ENABLE_TEST_COVERAGE" == "yes" ]]; then
        INCLUDE_DIRS=("sw/itc-api/if" "sw/itc-common/if" "sw/itc-api/inc" "sw/itc-common/inc" "sw/itc-api/src" "sw/itc-common/src")

        AUTO_BUILD_COMMAND+="&& mkdir -p $TEST_COVERAGE_OUTPUT "

        INCLUDE_FLAGS=""
        for dir in "${INCLUDE_DIRS[@]}"; do
            INCLUDE_FLAGS+="--include $dir "
        done

        # NON_PORTABLE_LCOV_IGNORING_OPTIONS="--ignore-errors empty --ignore-errors unused "
		NON_PORTABLE_LCOV_IGNORING_OPTIONS=""
		# NON_PORTABLE_LCOV_FILTERING_OPTION="$INCLUDE_FLAGS"
		NON_PORTABLE_LCOV_FILTERING_OPTION="$INCLUDE_FLAGS"
        AUTO_BUILD_COMMAND+="&& lcov --capture --directory sw/ $NON_PORTABLE_LCOV_FILTERING_OPTION --output-file $TEST_COVERAGE_INFO $NON_PORTABLE_LCOV_IGNORING_OPTIONS "

        AUTO_BUILD_COMMAND+="&& genhtml $TEST_COVERAGE_INFO --output-directory $TEST_COVERAGE_OUTPUT "
    fi
    
    AUTO_BUILD_COMMAND+="&& echo '============================================================================' "
    AUTO_BUILD_COMMAND+="&& echo '[+] Unit test logs saved in: $UNITTEST_LOG' "
    if [[ "$ENABLE_VALGRIND" == "yes" ]]; then
        AUTO_BUILD_COMMAND+="&& echo '[+] Valgrind report saved in: $VALGRIND_REPORT_XML' "
    fi
    if [[ "$ENABLE_TEST_COVERAGE" == "yes" ]]; then
        AUTO_BUILD_COMMAND+="&& echo '[+] Coverage info saved in: $TEST_COVERAGE_INFO' "
        AUTO_BUILD_COMMAND+="&& echo '[+] Please run the command below to see test coverage report:' "
        AUTO_BUILD_COMMAND+="&& echo '[+] >> xdg-open $TEST_COVERAGE_OUTPUT/index.html' "
        AUTO_BUILD_COMMAND+="&& echo '[+] Or upload $TEST_COVERAGE_INFO file to lcov-viewer, an online tool:' "
        AUTO_BUILD_COMMAND+="&& echo '[+] >> https://lcov-viewer.netlify.app/' "
    fi
    AUTO_BUILD_COMMAND+="&& echo '============================================================================' "
    AUTO_BUILD_COMMAND+="&& echo '================== UNIT TEST DONE ==================' "
    
fi

# Print final configuration command
echo "[+] Running: $AUTO_BUILD_COMMAND"

# Execute the configuration command
eval "$AUTO_BUILD_COMMAND"

# Verify output
if  [[ "$BUILD_TYPE" == "unittest" ]]; then
    if [[ ! -f "$UNITTEST_LOG" || $(grep -E '\[  FAILED  \]' "$UNITTEST_LOG") || ! $(grep -E "PASSED" "$UNITTEST_LOG") ]]; then
        echo -e '[-] UNIT TEST: \t\t\t [FAILED] ❌'
    else
        echo -e '[+] UNIT TEST: \t\t\t [PASSED] ✅'
    fi

    
    if [[ "$ENABLE_VALGRIND" == "yes" ]]; then
        if [[ ! -f "$VALGRIND_REPORT_XML" || $(grep -E '<error>|<fatal_signal>' "$VALGRIND_REPORT_XML") ]]; then
            echo -e '[-] VALGRIND TEST: \t\t [FAILED] ❌'
        else
            echo -e '[+] VALGRIND TEST: \t\t [PASSED] ✅'
        fi
    fi
    
    if [[ "$ENABLE_TEST_COVERAGE" == "yes" ]]; then
        GENHTML_OUTPUT="genhtml $TEST_COVERAGE_INFO --output-directory $TEST_COVERAGE_OUTPUT"
        
        # Extract line and function coverage percentages using grep and sed
        LINES_RATE=$($GENHTML_OUTPUT | grep "lines......:" | sed -E 's/.*: ([0-9]+\.[0-9]+)%.*/\1/')
        FUNCTIONS_RATE=$($GENHTML_OUTPUT | grep "functions......:" | sed -E 's/.*: ([0-9]+\.[0-9]+)%.*/\1/')
        # LINES_RATE=$($GENHTML_OUTPUT | awk '/lines\.+:/ {print $2}' | tr -d '%')
        # FUNCTIONS_RATE=$($GENHTML_OUTPUT | awk '/functions\.+:/ {print $2}' | tr -d '%')
        
        # Handle empty or invalid values
        if [[ -z "$LINES_RATE" || -z "$FUNCTIONS_RATE" ]]; then
            echo "[-] ERROR: Failed to extract coverage rates"
            exit 1
        fi
        
        # echo "Line Coverage     : $LINES_RATE%"
        # echo "Function Coverage : $FUNCTIONS_RATE%"
        
        # Check if both are >= $TEST_COVERAGE_RATE or 92% by default
        LINES_RATE_PASSED=$(echo "$LINES_RATE >= $TEST_COVERAGE_RATE" | bc)
        FUNCTIONS_RATE_PASSED=$(echo "$FUNCTIONS_RATE >= $TEST_COVERAGE_RATE" | bc)
        
        if [[ "$LINES_RATE_PASSED" -eq 1 && "$FUNCTIONS_RATE_PASSED" -eq 1 ]]; then
            echo -e '[+] COVERAGE TEST: \t\t [PASSED] ✅'
        else
            echo -e '[-] COVERAGE TEST: \t\t [FAILED] ❌'
        fi
    fi
    
    if [[ "$ENABLE_ADDRESS_SANITIZER" == "--enable-address-sanitizer" ]]; then
        if [[ ! -f "$UNITTEST_LOG" || $(grep -E 'ERROR: .*Sanitizer:' "$UNITTEST_LOG") ]]; then
            echo -e '[-] ADDRESS-SANITIZER TEST: \t [FAILED] ❌'
        else
            echo -e '[+] ADDRESS-SANITIZER TEST: \t [PASSED] ✅'
        fi
    fi
    
    if [[ "$ENABLE_THREAD_SANITIZER" == "--enable-thread-sanitizer" ]]; then
        if [[ ! -f "$UNITTEST_LOG" || $(grep -E 'WARNING: ThreadSanitizer:' "$UNITTEST_LOG") ]]; then
            echo -e '[-] THREAD-SANITIZER TEST: \t [FAILED] ❌'
        else
            echo -e '[+] THREAD-SANITIZER TEST: \t [PASSED] ✅'
        fi
    fi
fi