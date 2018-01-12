#ifndef _LKL_EXPORTS_H
#define _LKL_EXPORTS_H

#ifdef __cplusplus
extern "C" {
#endif

/* #include <lkl/asm-generic/posix_types.h> */
#include <lkl.h>

struct SimExported {
#include "lkl_exports.generated.h"
};


#ifdef __cplusplus
}
#endif

#endif

