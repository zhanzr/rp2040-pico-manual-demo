#ifndef __CUSTOM_DEF_H__
#define __CUSTOM_DEF_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

#define	configTICK_RATE_HZ	1000

// Helper macro to convert numeric value to string
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// Compiler detection
#if defined(__ARMCC_VERSION) && defined(__ARM_ARCH)
    #define COMPILER_NAME "ARMClang " TOSTRING(__ARMCC_VERSION)
#elif defined(__GNUC__) && defined(__ARM_ARCH)
    #define COMPILER_NAME "ARM GCC " __VERSION__
#elif defined(__GNUC__) && defined(__riscv)
    #define COMPILER_NAME "RISC-V GCC " __VERSION__
#else
    #define COMPILER_NAME "Unknown Compiler"
#endif

// CPU architecture detection — printed at startup
#if defined(__ARM_ARCH)
    #define CPU_ARCH "ARM Cortex-M33"
#elif defined(__riscv)
    #define CPU_ARCH "RISC-V Hazard3"
#else
    #define CPU_ARCH "Unknown CPU"
#endif

#endif
