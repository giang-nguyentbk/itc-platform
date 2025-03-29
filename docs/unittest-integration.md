## How to install Google Mock and Google Test unittest on Ubuntu Linux
```bash
$ sudo apt update -y
$ sudo apt install libgtest-dev libgmock-dev -y

+ Verify:
$ dpkg -l | grep gtest
$ dpkg -l | grep gmock
```

## How to run tests
```bash
$ make clean && aclocal && autoconf && automake --add-missing && autoreconf -fi && ./configure BUILD_TARGET=target1 BUILD_TYPE=unittest TEST_SOME_UT_SUITES_ONLY=yes && make && ./itc_platform_unittest

+ Or:
$ make clean && aclocal && autoconf && automake --add-missing && autoreconf -fi && ./configure BUILD_TARGET=target1 BUILD_TYPE=unittest TEST_SOME_UT_SUITES_ONLY=yes && make && ./itc_platform_unittest && make clean

+ Or with valgrind:
$ make clean && aclocal && autoconf && automake --add-missing && autoreconf -fi && ./configure BUILD_TARGET=target1 BUILD_TYPE=unittest TEST_SOME_UT_SUITES_ONLY=yes && make && valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all ./itc_platform_unittest

+ Stress test:
$ for i in {1..50}; do ./itc_platform_unittest; done | grep --col -iE "FAILED TEST"
$ for i in {1..50}; do valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all ./itc_platform_unittest; done | grep --col -iE "FAILED TEST"

```

## How to get a file size in bytes
```bash
$ stat -c %s itc_platform_unittest

```

## How to get test coverage
```bash
# Use lcov to get test coverage visualisation.
$ sudo apt install lcov -y
$ sudo apt install xdg-utils -y
    
# Step 1:
+ Add -fprofile-arcs -ftest-coverage --coverage to *_CPPFLAGS
and --coverage to AM_LDFLAGS.
# Step 2:
    + Compile unit test main program.
# Step 3:
    + 

```