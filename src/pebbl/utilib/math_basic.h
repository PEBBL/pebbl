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

/**
 * \file math_basic.h
 *
 * Defines and constants for basic numerical operations.
 */

#ifndef utilib_math_basic_h
#define utilib_math_basic_h

#include <pebbl/utilib/std_headers.h>

#ifdef UTILIB_SOLARIS_CC
#include <sunmath.h>
#include <ieeefp.h>
#endif

/*
 *
 * DEFINE MATHEMATICAL CONSTANTS
 *
 */
/** Value of the largest integer */
#ifndef MAXINT
#define MAXINT INT_MAX
#endif

/** Value of the largest float */
#ifndef MAXFLOAT
#define MAXFLOAT FLT_MAX
#endif

/** Value of the largest double */
#ifndef MAXDOUBLE
#define MAXDOUBLE DBL_MAX
#endif

/** Value of the largest long double */
#ifdef LDBL_MAX
#undef  MAXLONGDOUBLE
#define MAXLONGDOUBLE LDBL_MAX
#endif

namespace utilib {

/** Returns +1 if argument is positive, -1 if it is negative, and 0 otherwise. */
template<class T>
inline int sgn(const T& thing)
{
  if (thing > 0)
    return 1;
  if (thing < 0)
    return -1;
  return 0;
}

}

/* Compute number of bits needed to hold an integer value */
unsigned int bitWidth(unsigned int x);


/* Computes the greatest common divisor of two integers using the */
/* classical Euclidean algorithm (remainder version).             */
unsigned int gcd(unsigned int a, unsigned int b);

#endif

