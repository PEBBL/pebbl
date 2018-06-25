/*
 * \file pebblComms.h
 *
 * Defines the pebblComms class, which provides a home for bounding
 * communicators and pebbl communicators and there associated
 * wrapper functions
 *
 * \author Will Loughlin
 *
 */

#ifndef pebbl_pebblComm_H
#define pebbl_pebblComm_H

#include <mpiComm.h>

namespace pebbl {

class pebblComms
{
protected:
    mpiComm headComm;
    mpiComm boundComm;

public:
    
    /// Get the head Communicator wrapper
    mpiComm myHeadComm() { return headComm; };

    /// Get the bound Communicator wrapper
    mpiComm myBoundComm() { return boundComm; };

    /// Check if this processor is a part of Pebble
    mpiComm isHead() { return headComm != NULL; };

}

#endif
