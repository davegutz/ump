// Memory monitor functions

#ifndef CHECK_MEM_H
#define CHECK_MEM_H

// Includes
using namespace std;
#include <common.h>
#include <unistd.h>    // sbrk
#ifdef BACKWARD
#  include <fstream.h>   // cerr etc
#else
#  include <iostream>     // cerr etc
#endif

#ifdef HAVE_SBRK
// Initialize these in top level program someplace
extern int verbose, mem; 
void printMem(const char *comment);
int  getMem();
void checkMem(const char *comment);
#else
#error "ERROR(checkMem.h):  sbrk() not available.  Undefine DEBUG_MEM in common.h"
#endif  // HAVE_SBRK
#endif  // CHECK_MEM_H


