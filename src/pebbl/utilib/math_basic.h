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
#include <pebbl/utilib/_generic.h>


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

#ifdef _MSC_VER
const long double M_PI = 3.1415926535897932384626433832795029L;
const long double M_E  = 2.7182818284590452354L;
#endif


#if defined(__cplusplus)
namespace utilib {

/*
 * 
 * OPERATIONS ON SIMPLE VALUES
 *
 */


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

// convert any printable thing to a std::string
// used by pscientific
template <class T>
std::string tostring(const T &arg) {
  std::ostringstream out;
  out << arg;
  return(out.str());
  }


///
/// pscientific returns a portable scientific notation output, consistent
/// across platforms and bit sizes (within the machine precision).
/// stream options ios::uppercase and ios::showpos are supported
/// this does not support all stream options, and will have undetermined
/// results if flags such as left, right, hex, etc are set.
/// ios::setpoint and ios::width are not supported, but probably should be
///
std::string pscientific(const double t, int mantissa=6, int expwidth=3,
	std::ostream* os = NULL);

}

#endif

/*
 *
 * MISCELLANEOUS 
 *
 */

/** Compute the number of lines in file filename. */
int calc_filesize(char* filename);

/**
 * Return the integer value of the rounded value of x.  
 * If the fractional part of x is less than 0.5, then x is
 * rounded down.  Otherwise, x is rounded up.
 */
#ifndef UTILIB_HAVE_LROUND
#ifndef DOXYGEN
long int lround _((double x));
#else
long int lround(double x);
#endif
#endif
#if defined(UTILIB_SGI_CC) || defined(UTILIB_OSF_CC) || defined(__PGI) || defined(_MSC_VER)
long int lround(double x);
#endif

/**
 * A method for rounding a double to num_digits
 * number of decimal digits after the decimal point.
 */
double d_round(double to_round, unsigned int num_digits);


/* Compute number of bits needed to hold an integer value */
unsigned int bitWidth(unsigned int x);


/* Computes the greatest common divisor of two integers using the */
/* classical Euclidean algorithm (remainder version).             */
unsigned int gcd(unsigned int a, unsigned int b);

#endif


