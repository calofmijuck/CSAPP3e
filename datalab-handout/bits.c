/*
 * CS:APP Data Lab
 *
 * <Please put your name and userid here>
 * Name: Sungchan Yi (2017-18570)
 * UserId: calofmijuck
 *
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:

  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code
  must conform to the following style:

  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>

  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.


  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function.
     The max operator count is checked by dlc. Note that '=' is not
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 *
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce
 *      the correct answers.
 */


#endif
/* Copyright (C) 1991-2014 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses ISO/IEC 10646 (2nd ed., published 2011-03-15) /
   Unicode 6.0.  */
/* We do not support C11 <threads.h>.  */
/*
 * bitNor - ~(x|y) using only ~ and &
 *   Example: bitNor(0x6, 0x5) = 0xFFFFFFF8
 *   Legal ops: ~ &
 *   Max ops: 8
 *   Rating: 1
 */
int bitNor(int x, int y) {
    // By De Morgan's Law, ~(x or y) <=> ~x and ~y
    return ~x & ~y;
}
/*
 * bitOr - x|y using only ~ and &
 *   Example: bitOr(6, 5) = 7
 *   Legal ops: ~ &
 *   Max ops: 8
 *   Rating: 1
 */
int bitOr(int x, int y) {
    // x or y <=> ~(~x and ~y)
    return ~(~x & ~y);
}
/*
 * bitXor - x^y using only ~ and &
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
    // x xor y <=> (x and ~y) or (~x and y)
    // <=> ~(~(x and ~y) and ~(~x and y))
    return ~(~(x & ~y) & ~(~x & y));
}
/*
 * allEvenBits - return 1 if all even-numbered bits in word set to 1
 *   Examples allEvenBits(0xFFFFFFFE) = 0, allEvenBits(0x55555555) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allEvenBits(int x) {
    // tmp = x & 0x55555555 should be 0x55555555 if all even-numbered bits are 1
    // tmp ^ (tmp << 1) should be 0xFFFFFFFF
    // adding 1 would cause it to be 0. Now use logical NOT
    int r = (0x55 << 8) | 0x55;
    int tmp = x & ((r << 16) | r);
    return !((tmp ^ (tmp << 1)) + 1);
}
/*
 * leastBitPos - return a mask that marks the position of the
 *               least significant 1 bit. If x == 0, return 0
 *   Example: leastBitPos(96) = 0x20
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int leastBitPos(int x) {
    // x & (-x) returns the least bit position
    return x & (~x + 1);
}
/*
 * fitsBits - return 1 if x can be represented as an
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
    // shift 32 - n bits to the left and then shift 32 - n bits
    // to the right. If x were to fit in n-bit 2's complement integer,
    // x would remain unchanged.
    int k = 33 + ~n;
    return !(x ^ ((x << k) >> k));
}
/*
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) {
    // if x is positive return x >> n
    // if x is negative...  add pow(2, n) - 1 to x
    // and then return x >> n
    int neg = x >> 31; // if x < 0, neg = 0xFFFFFFFF = -1
    int k = (neg & 1) << n;
    return (x + k + neg) >> n;
}
/*
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
    // check for 0x30 <= x <= 0x39.
    // If x is out of bounds, overflow will occur and it will be detected.
    // When overflow occurs, return 0
    int sign = 1 << 31;
    int high = sign & (x + ~(sign | 0x39)) >> 31;
    int low = sign & (x + ~0x2F) >> 31;
    return !(high | low);
}
/*
 * isLessOrEqual - if x <= y  then return 1, else return 0
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
    int minusX = ~x + 1;
    int yminusx = y + minusX;
    int flag1 = (yminusx >> 31) & 1; // 0 if x <= y

    // Cases with overflow
    int sign = 1 << 31;
    int signx = x & sign;
    int signy = y & sign;
    int flag2 = ((signx ^ signy) >> 31) & 1; // 1 if x, y have different signs
    // must return 1 when overflow does not occur, or when x, y have different signs and x is negative
    return (!flag1 & !flag2) | (flag2 & (signx >> 31));
}
/*
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int logicalShift(int x, int n) {
    // (x >> n) & (top n bits are 0 and rest 1)
    int msb = ((1 << 31) >> n) << 1;
    return (x >> n) & ~msb;
}
/*
 * multFiveEighths - multiplies by 5/8 rounding toward 0.
 *   Should exactly duplicate effect of C expression (x*5/8),
 *   including overflow behavior.
 *   Examples: multFiveEighths(77) = 48
 *             multFiveEighths(-22) = -13
 *             multFiveEighths(1073741824) = 13421728 (overflow)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 3
 */
int multFiveEighths(int x) {
    int neg = x >> 31; // check for negative
    int k = (neg & 1) << 3; // rounds toward 0
    int x5 = (x << 2) + x; // times 5
    return (x5 + neg + k) >> 3; // divide by 8
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
    // Divide the 32 bits repeatedly in half and conquer!
    // Mask and add vertically
    int step1 = 0x55;
    int step2 = 0x33;
    int step3 = 0x0F;
    int step4 = 0xFF;
    int step5;
    int res;

    // step1 = 0x55555555
    step1 = (step1 << 8) | step1;
    step1 = (step1 << 16) | step1;
    // step2 = 0x33333333
    step2 = (step2 << 8) | step2;
    step2 = (step2 << 16) | step2;
    // step3 = 0x0F0F0F0F
    step3 = (step3 << 8) | step3;
    step3 = (step3 << 16) | step3;
    // step5 = 0x0000FFFF
    step5 = (step4 << 8) | step4;
    // step4 = 0x00FF00FF
    step4 = (step4 << 16) | step4;

    // Now divide and conquer, merge
    res = (x & step1) + ((x >> 1) & step1);
    res = ((res >> 2) & step2) + (res & step2);
    res = ((res >> 4) + res) & step3;
    res = ((res >> 8) + res) & step4;
    res = ((res >> 16) + res) & step5;
    return res;
}
/*
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4
 */
int bang(int x) {
    int lsb = x & (~x + 1); // least significant bit
    int ret = (~lsb + 1) >> 31; // ret is 0 iff x is 0, else it is 1
    return ~ret & 1;
}
/*
 * bitParity - returns 1 if x contains an odd number of 0's
 *   Examples: bitParity(5) = 0, bitParity(7) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 4
 */
int bitParity(int x) {
    // A ^ B = C then C has the same parity of number of 1 bits as (number of 1 bits in A) + (number of 1 bits in B)
    x = x ^ (x >> 16); // lower 16 bits have same parity
    x = x ^ (x >> 8); // lower 8 bits have same parity
    x = x ^ (x >> 4); // lower 4 bits have same parity
    x = x ^ (x >> 2); // lower 2 bits have same parity
    x = x ^ (x >> 1); // lower bit has the same parity
    return x & 1; // & 1 for truncating all bits except for the lsb
}
/*
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) {
    int exponent = (uf >> 23) & 0xFF; // exponent
    int mantissa = (uf & 0x7FFFFF) | 0x800000; // mantissa, append a 1 to the left
    int e = exponent - 127;

    // for NaN
    if(exponent == 0x7F800000) return 0x80000000u;

    // if e < 0 round down to 0, check range for integer range
    if(e < 0) return 0;
    if(e > 30) return 0x80000000u;

    // if the given float is in range, shift mantissa appropriately
    if(e >= 23) mantissa <<= e - 23;
    else mantissa >>= 23 - e;

    if(uf >> 31) return ~mantissa + 1;
    return mantissa;

}
/*
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
    unsigned int exponent = (uf >> 23) & 0xFF; // exponent
    unsigned int mantissa = (uf & 0x7FFFFF) << 8; // mantissa, shifted 8 bits
    int sign = uf >> 31 << 31;
    // unsigned int mask = ~0 >> 1;

    // If input is +0, return +0
    if(uf == 0) return 0;
    // If input is -0, return -0
    if(uf == 0x80000000) return 0x80000000u;

    // Cases with biggest exponent
    if(exponent == 0xFF) {
        if(mantissa != 0) return uf;
        // return inf
        if(!sign) return 0x7F800000u;
        // return -inf
        else if(sign) return 0xFF800000u;
    }
    // No exponent then add mantissa to itself
    if(exponent == 0) {
        mantissa += mantissa;
    } else { // else increase exponent by 1
        exponent = (exponent + 1) << 23;
        // exponent &= mask;
    }
    mantissa >>= 8; // shift back 8 to return to the correct encoding location
    return sign | exponent | mantissa; // merge sign, exponent, mantissa altogether
}
