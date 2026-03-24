Author: Maxwell Kopita

Floating Point Overflow Checker

Description:This program checks for floating-point precision overflow in C++.

It takes two inputs:
- a loop bound
- a loop counter (increment)

The goal is to figure out if adding the loop counter will eventually stop changing the value because of how floating-point numbers are stored (IEEE 754).
If overflow is possible, the program prints a warning and shows the smallest value where the increment no longer affects the number.

How to Build
- Compile the program using g++: g++ -std=c++17 -Wall -Wextra -o fp_checker fp_overflow_checker.cpp

How to Run
- Run the program with two floating-point values: ./fp_checker <loop_bound> <loop_counter>

Example runs:
./fp_checker 1e+08 1.0
./fp_checker 1e+05 2.0
./fp_checker 1.4567 3.14

What the Program Does
Prints the IEEE 754 bit representation of both inputs, Checks if precision overflow can happen.

If overflow is possible: prints a warning, shows the overflow threshold, prints the threshold in binary

If no overflow is possible, it prints:
No overflow!

How It Works (Simple Explanation)
Instead of using math functions like log2() or pow(), the program works directly with the bits of floating-point numbers.

- It reads the exponent and fraction bits from the float
- It figures out when the increment becomes too small to change the value
- It builds the threshold using bit manipulation

This follows the assignment rules and keeps everything in IEEE 754 format.
Requirements
C++17
g++ compiler

Known Bugs / Limitations
- Only supports positive floating-point inputs (negative values are rejected)
- Uses single-precision floats (float), not double precision
- Very large values may become infinity, which stops threshold calculation
- Does not handle invalid text input beyond basic error checking
- Output formatting assumes standard terminal behavior (spacing may look slightly different in some environments)
