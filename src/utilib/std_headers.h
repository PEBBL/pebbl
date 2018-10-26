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
 * \file std_headers.h
 *
 * This header file is used to facilitate portability of C and C++ code by
 * encapsulating the includes for C and C++ headers that vary across 
 * different platforms.  For example, this header file will include
 * ANSI-compliant C++ header on platforms that support them.
 *
 * \author William E. Hart
 */

#ifndef utilib_std_headers_h
#define utilib_std_headers_h

#include <pebbl_config.h>

#if defined(UTILIB_HAVE_STD)
///* C++ compiler using new C headers */
#include <cstring>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cassert>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <limits>

#else
///* C compiler (old style headers are used to avoid std namespace)
//   or a C++ compiler which uses old C headers */
#ifdef UTILIB_HAVE_VALUES_H
#include <values.h> /* Obsolete: It defines MAXINT, MAXFLOAT, etc. */
#endif
#ifdef UTILIB_HAVE_FLOAT_H
#include <float.h>
#endif
#ifdef UTILIB_HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef UTILIB_HAVE_STRINGS_H
#include <strings.h>
#endif
#include <string.h>
#include <math.h>
#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#endif

#include <typeinfo>
#include <iterator>

#if defined(UTILIB_HAVE_STD)
#include <iostream>
#include <vector>
#include <list>
#include <iomanip>
#include <fstream>
#include <set>
#include <map>
#include <queue>
#include <stack>
#else
#include <iostream.h>
#include <vector.h>
#include <list.h>
#include <iomanip.h>
#include <fstream.h>
#include <set.h>
#include <map.h>
#include <queue.h>
#include <stack.h>
#endif

#ifdef UTILIB_HAVE_EXCEPTIONS
#include <stdexcept>
#else
#error "fix file to work in the absence of exceptions"
#endif

#ifndef UTILIB_HAVE_SSTREAM
#ifdef _MSC_VER
#include <strstrea.h>
#elif !defined(UTILIB_HAVE_STD)
#include <strstream.h>
#else
#include <strstream>
#endif
#else   // UTILIB_HAVE_SSTREAM
#include <sstream>
#endif  // UTILIB_HAVE_SSTREAM

#endif
