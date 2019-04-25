/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

/**
 * \file packPointer.h
 * \author Jonathan Eckstein
 * 
 * Generic code for putting pointer in and out of (Un)PackBuffers.
 */

#ifndef pebbl_packPointer_h
#define pebbl_packPointer_h

#include <pebbl_config.h>
#include <pebbl/utilib/PackBuf.h>

namespace pebbl {


#define PEBBL_packAs(buf,what,kind,castType) \
   { \
       castType packTemp_ = 0; \
       memcpy(&packTemp_,&what,sizeof(kind)); \
       buf << packTemp_; \
   }


#define PEBBL_unpackAs(buf,what,kind,castType) \
   { \
       castType unpackTemp_; \
       buf >> unpackTemp_; \
       kind* tp = (kind*) &unpackTemp_; \
       what = *tp; \
   }


// JE rewrote this 4/19/2019 to avoid -Wall warnings

inline void packPointer(PackBuffer& outBuffer,void* ptr)
{
  if (sizeof(void*) == sizeof(int))
    PEBBL_packAs(outBuffer,ptr,void*,int)
  else if (sizeof(void*) == sizeof(long))
    PEBBL_packAs(outBuffer,ptr,void*,long)
  else if (sizeof(void*) == sizeof(double))
    PEBBL_packAs(outBuffer,ptr,void*,double)
  else
    EXCEPTION_MNGR(std::runtime_error, "Can't figure out how to pack " << sizeof(void*) << "-byte pointers");
}


inline void* unpackPointer(UnPackBuffer& inBuffer)
{
  void* temp = 0;
  if (sizeof(void*) == sizeof(int))
    PEBBL_unpackAs(inBuffer,temp,void*,int)
  else if (sizeof(void*) == sizeof(long))
    PEBBL_unpackAs(inBuffer,temp,void*,long)
  else if (sizeof(void*) == sizeof(double))
    PEBBL_unpackAs(inBuffer,temp,void*,double)
  else
    EXCEPTION_MNGR(std::runtime_error, "Can't figure out how to unpack " << sizeof(void*) << "-byte pointers");
  return (void*) temp;
}
  
} // namespace pebbl

#endif
