/***********************************************************************
Copyright (c) 2006-2011, Skype Limited. All rights reserved.
Copyright (c) 2013       Parrot
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
- Neither the name of Internet Society, IETF or IETF Trust, nor the
names of specific contributors, may be used to endorse or promote
products derived from this software without specific prior written
permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
***********************************************************************/

#ifndef SILK_MACROS_ARMv5E_H
#define SILK_MACROS_ARMv5E_H

#ifdef USE_MSVS_ARM_INTRINCICS
/* (a32 * (opus_int32)((opus_int16)(b32))) >> 16 output have to be 32bit int */
#define silk_SMULWB_armv5e(a, b)      _arm_smulwb((a), (b))
/* a32 + (b32 * (opus_int32)((opus_int16)(c32))) >> 16 output have to be 32bit int */
#define silk_SMLAWB_armv5e(a, b, c)   _arm_smlawb((b), (c), (a))
/* (a32 * (b32 >> 16)) >> 16 */
#define silk_SMULWT_armv5e(a, b)      _arm_smulwt((a), (b))
/* a32 + (b32 * (c32 >> 16)) >> 16 */
#define silk_SMLAWT_armv5e(a, b, c)   _arm_smlawt((b), (c), (a)) 
/* (opus_int32)((opus_int16)(a3))) * (opus_int32)((opus_int16)(b32)) output have to be 32bit int */
#define silk_SMULBB_armv5e(a, b)      _arm_smulbb((a), (b))
/* a32 + (opus_int32)((opus_int16)(b32)) * (opus_int32)((opus_int16)(c32)) output have to be 32bit int */
#define silk_SMLABB_armv5e(a, b, c)   _arm_smlabb((b), (c), (a))
/* (opus_int32)((opus_int16)(a32)) * (b32 >> 16) */
#define silk_SMULBT_armv5e(a, b)      _arm_smulbt((a), (b))
/* a32 + (opus_int32)((opus_int16)(b32)) * (c32 >> 16) */
#define silk_SMLABT_armv5e(a, b, c)   _arm_smlabt((b), (c), (a))
/* add/subtract with output saturated */
#define silk_ADD_SAT32_armv5e(a, b)   _arm_qadd((a), (b))
#define silk_SUB_SAT32_armv5e(a, b)   _arm_qsub((a), (b))
#define silk_CLZ16_armv5(in16)        ((opus_int32)_arm_clz(((opus_int16)(in16))<<16|0x8000))
#define silk_CLZ32_armv5(in32)        ((opus_int32)_arm_clz((unsigned int)(in32)))
#else

/* This macro only avoids the undefined behaviour from a left shift of
   a negative value. It should only be used in macros that can't include
   SigProc_FIX.h. In other cases, use silk_LSHIFT32(). */
#define SAFE_SHL(a,b) ((opus_int32)((opus_uint32)(a) << (b)))

/* (a32 * (opus_int32)((opus_int16)(b32))) >> 16 output have to be 32bit int */
static OPUS_INLINE opus_int32 silk_SMULWB_armv5e(opus_int32 a, opus_int16 b)
{
  int res;
  __asm__(
      "#silk_SMULWB\n\t"
      "smulwb %0, %1, %2\n\t"
      : "=r"(res)
      : "r"(a), "r"(b)
  );
  return res;
}

/* a32 + (b32 * (opus_int32)((opus_int16)(c32))) >> 16 output have to be 32bit int */
static OPUS_INLINE opus_int32 silk_SMLAWB_armv5e(opus_int32 a, opus_int32 b,
 opus_int16 c)
{
  int res;
  __asm__(
      "#silk_SMLAWB\n\t"
      "smlawb %0, %1, %2, %3\n\t"
      : "=r"(res)
      : "r"(b), "r"(c), "r"(a)
  );
  return res;
}

/* (a32 * (b32 >> 16)) >> 16 */
static OPUS_INLINE opus_int32 silk_SMULWT_armv5e(opus_int32 a, opus_int32 b)
{
  int res;
  __asm__(
      "#silk_SMULWT\n\t"
      "smulwt %0, %1, %2\n\t"
      : "=r"(res)
      : "r"(a), "r"(b)
  );
  return res;
}

/* a32 + (b32 * (c32 >> 16)) >> 16 */
static OPUS_INLINE opus_int32 silk_SMLAWT_armv5e(opus_int32 a, opus_int32 b,
 opus_int32 c)
{
  int res;
  __asm__(
      "#silk_SMLAWT\n\t"
      "smlawt %0, %1, %2, %3\n\t"
      : "=r"(res)
      : "r"(b), "r"(c), "r"(a)
  );
  return res;
}

/* (opus_int32)((opus_int16)(a3))) * (opus_int32)((opus_int16)(b32)) output have to be 32bit int */
static OPUS_INLINE opus_int32 silk_SMULBB_armv5e(opus_int32 a, opus_int32 b)
{
  int res;
  __asm__(
      "#silk_SMULBB\n\t"
      "smulbb %0, %1, %2\n\t"
      : "=r"(res)
      : "%r"(a), "r"(b)
  );
  return res;
}

/* a32 + (opus_int32)((opus_int16)(b32)) * (opus_int32)((opus_int16)(c32)) output have to be 32bit int */
static OPUS_INLINE opus_int32 silk_SMLABB_armv5e(opus_int32 a, opus_int32 b,
 opus_int32 c)
{
  int res;
  __asm__(
      "#silk_SMLABB\n\t"
      "smlabb %0, %1, %2, %3\n\t"
      : "=r"(res)
      : "%r"(b), "r"(c), "r"(a)
  );
  return res;
}

/* (opus_int32)((opus_int16)(a32)) * (b32 >> 16) */
static OPUS_INLINE opus_int32 silk_SMULBT_armv5e(opus_int32 a, opus_int32 b)
{
  int res;
  __asm__(
      "#silk_SMULBT\n\t"
      "smulbt %0, %1, %2\n\t"
      : "=r"(res)
      : "r"(a), "r"(b)
  );
  return res;
}

/* a32 + (opus_int32)((opus_int16)(b32)) * (c32 >> 16) */
static OPUS_INLINE opus_int32 silk_SMLABT_armv5e(opus_int32 a, opus_int32 b,
 opus_int32 c)
{
  int res;
  __asm__(
      "#silk_SMLABT\n\t"
      "smlabt %0, %1, %2, %3\n\t"
      : "=r"(res)
      : "r"(b), "r"(c), "r"(a)
  );
  return res;
}

/* add/subtract with output saturated */
static OPUS_INLINE opus_int32 silk_ADD_SAT32_armv5e(opus_int32 a, opus_int32 b)
{
  int res;
  __asm__(
      "#silk_ADD_SAT32\n\t"
      "qadd %0, %1, %2\n\t"
      : "=r"(res)
      : "%r"(a), "r"(b)
  );
  return res;
}

static OPUS_INLINE opus_int32 silk_SUB_SAT32_armv5e(opus_int32 a, opus_int32 b)
{
  int res;
  __asm__(
      "#silk_SUB_SAT32\n\t"
      "qsub %0, %1, %2\n\t"
      : "=r"(res)
      : "r"(a), "r"(b)
  );
  return res;
}

static OPUS_INLINE opus_int32 silk_CLZ16_armv5(opus_int16 in16)
{
  int res;
  __asm__(
      "#silk_CLZ16\n\t"
      "clz %0, %1;\n"
      : "=r"(res)
      : "r"(SAFE_SHL(in16,16)|0x8000)
  );
  return res;
}

static OPUS_INLINE opus_int32 silk_CLZ32_armv5(opus_int32 in32)
{
  int res;
  __asm__(
      "#silk_CLZ32\n\t"
      "clz %0, %1\n\t"
      : "=r"(res)
      : "r"(in32)
  );
  return res;
}

#endif //USE_MSVS_ARM_INTRINCICS

#undef silk_SMULWB
#define silk_SMULWB(a, b) (silk_SMULWB_armv5e(a, b))
#undef silk_SMLAWB
#define silk_SMLAWB(a, b, c) (silk_SMLAWB_armv5e(a, b, c))
#undef silk_SMULWT
#define silk_SMULWT(a, b) (silk_SMULWT_armv5e(a, b))
#undef silk_SMLAWT
#define silk_SMLAWT(a, b, c) (silk_SMLAWT_armv5e(a, b, c))
#undef silk_SMULBB
#define silk_SMULBB(a, b) (silk_SMULBB_armv5e(a, b))
#undef silk_SMLABB
#define silk_SMLABB(a, b, c) (silk_SMLABB_armv5e(a, b, c))
#undef silk_SMULBT
#define silk_SMULBT(a, b) (silk_SMULBT_armv5e(a, b))
#undef silk_SMLABT
#define silk_SMLABT(a, b, c) (silk_SMLABT_armv5e(a, b, c))
#undef silk_ADD_SAT32
#define silk_ADD_SAT32(a, b) (silk_ADD_SAT32_armv5e(a, b))
#undef silk_SUB_SAT32
#define silk_SUB_SAT32(a, b) (silk_SUB_SAT32_armv5e(a, b))
#undef silk_CLZ16
#define silk_CLZ16(in16) (silk_CLZ16_armv5(in16))
#undef silk_CLZ32
#define silk_CLZ32(in32) (silk_CLZ32_armv5(in32))

#undef SAFE_SHL

#endif /* SILK_MACROS_ARMv5E_H */
