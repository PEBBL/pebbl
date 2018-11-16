/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

#include <pebbl_config.h>
#include <stdlib.h>
#include <pebbl/utilib/math_basic.h>
#include <math.h>

/// Calculates how many bits are needed to hold an unsigned value.
/// If the argument is 0, it returns 0.

unsigned int bitWidth(unsigned int x)
{
  unsigned int result;

  result = 0;

  while (x != 0)
    {
      result++;
      x >>= 1;
    }

  return result;
}


/* Computes the greatest common divisor of two integers using the */
/* classical Euclidean algorithm (remainder version).  Got this   */
/* particularly tight loop from Wikipedia!                        */

unsigned int gcd(unsigned int a, unsigned int b)
{
  while (b != 0)
    {
      register unsigned int temp = b;
      b = a % b;
      a = temp;
    }
  return a;
}
