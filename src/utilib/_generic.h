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
 * \file _generic.h
 *
 * Defines and typedefs used everywhere.
 *
 * \author William E. Hart
 */

#ifndef utilib_generic_h
#define utilib_generic_h

#include <utilib/std_headers.h>
#include <sys/types.h>

#if defined(__cplusplus)
namespace utilib {
#endif

/**
 * \enum EnumDataOwned
 *
 * Ownership categories for objects with reference counts.
 */
#ifndef UTILIB_ENUMDATAOWNED
#define UTILIB_ENUMDATAOWNED
enum EnumDataOwned
{
  DataNotOwned=0,	/**< Data owned by some other object */
  DataOwned=1,          /**< Memory allocated by object itself */
  AcquireOwnership=1,	/**< Synonym for DataOwned */
  AssumeOwnership=2	/**< We own it now, but it comes from elsewhere */
                        /**< Once the object has been made this is      */
                        /**< identical to DataOwned                     */
};
#endif

/**
 * \def ERR
 *
 * The default value of error values.
 */
enum { PEBBL_ERR = -999 };

#if defined(__cplusplus)
} // namespace end
#endif

/**
 * \def NULL
 *
 * Defines the value of empty pointers.
 */
#ifdef NULL
#undef NULL		/* Always override the definition of NULL */
#endif
#define NULL		0

#if !defined(UTILIB_HAVE_STD) && !defined(__cplusplus)
/**
 * \typedef size_t
 *
 * The typedef for \a size_t arguments.
 */
#ifndef UTILIB_SIZE_T
#define UTILIB_SIZE_T
typedef unsigned size_t;
#endif
#endif

/**
 * \typedef VOID
 *
 * The void type is a \a char in standard C.
 */
#ifndef VOID
typedef char VOID;
#endif

/**
 * \typedef size_type
 *
 * Used to provide a consistent definition of the size_t type.
 */
typedef size_t size_type;

#endif
