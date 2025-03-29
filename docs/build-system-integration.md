## How to install GNU-Autotool build system on Ubuntu Linux
```bash
$ sudo apt update -y
$ sudo apt install autoconf automake libtool pkg-config -y

+ Verify:
$ libtoolize --version
$ aclocal --version
$ automake --version
$ autoreconf --version
```

## How to build the project
```bash
# Necessary files
# 1. configure.ac
# 2. Makefile.am
# 3. Source and header files

# If any change in configure.ac or Makefile.am, need to rerun these:
$ libtoolize
$ aclocal
$ automake --add-missing
$ autoreconf -fi

# If want to build other target
$ ./configure BUILD_TARGET=target1
$ make
$ ./main

# For example, rebuild target2
$ make clean
$ ./configure BUILD_TARGET=target2
$ make
$ ./main

# Clean object files
$ make clean
```

## How to combine conditions in configure.ac
```bash
AM_CONDITIONAL([COMBINED_CONDITION_AND],
               [test "$cond_1" != "no" -a "$cond_2" != "no"])
AM_CONDITIONAL([COMBINED_CONDITION_OR],
               [test "$cond_1" != "no" -o "$cond_2" != "no"])
```