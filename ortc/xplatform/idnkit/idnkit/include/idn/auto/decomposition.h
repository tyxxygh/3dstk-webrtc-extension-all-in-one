/*
 * Do not edit this file!
 * This file is generated from:
 *    UnicodeData.txt (version 6.3.0)
 */

#ifndef IDN_DECOMPOSITION_H
#define IDN_DECOMPOSITION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#define IS_COMPAT_DECOMPOSITION(v) ((v) & 0x8000UL)
#define IS_DECOMPOSITIONSEQ_DATA_END(v) ((v) & 0x80000000UL)
#define DECOMPOSITIONSEQ_DATA(v) ((v) & ~0x80000000UL)

/*
 * Table accessor.
 */
extern unsigned long
idn__sparsemap_getdecomposition(unsigned long v);
extern const unsigned long *
idn__sparsemap_getdecompositionseq(unsigned long idx);

#ifdef __cplusplus
}
#endif

#endif
