#!/bin/bash

file="unittest/test-results/itc_platform_unittest.log"

awk '
/\[BENCHMARK\].*took/ {
    test_name = $2            # e.g., test1, test2, test3
    time = $(NF-1)            # e.g., 508
    sum[test_name] += time
    count[test_name]++
}
END {
    for (test in sum)
        printf "%s average: %.2f ns\n", test, sum[test] / count[test]
}
' "$file"
