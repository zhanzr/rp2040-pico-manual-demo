#ifndef __CUSTOM_DEF_H__
#define __CUSTOM_DEF_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

#define	configTICK_RATE_HZ	1000
// Run in performance mode (seeds = 0) for meaningful timing results
#define PERFORMANCE_RUN 1
// Helper macro to convert numeric value to string
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#ifdef __ARMCC_VERSION
    #define COMPILER_NAME "ARMClang " TOSTRING(__ARMCC_VERSION)
#elif defined(__GNUC__) && defined(__ARM_ARCH)
    #define COMPILER_NAME "GCC " __VERSION__
#else
    #define COMPILER_NAME "Unknown Compiler"
#endif

#endif
