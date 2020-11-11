/*
 * Copyright (c) 2011 Vesperix Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef FFTW_SINGLE
#error "NEON only works in single precision"
#endif

#define RIGHT_CPU X(have_neon)
extern int RIGHT_CPU(void);

#define VL                             1
#define ALIGNMENT                      8
#define ALIGNMENTA                     8
#define SIMD_VSTRIDE_OKA(x)            1
#define SIMD_STRIDE_OKPAIR             SIMD_STRIDE_OKA

#ifdef __ARM_NEON__

#ifdef HAVE_ARM_NEON_H
#include <arm_neon.h>
#endif

typedef float32x2_t V;

#define LDK(x)                         x
#define DVK(var, val)                  const V var = { val, val }

#define VADD(a, b)                     vadd_f32(a, b)
#define VSUB(a, b)                     vsub_f32(a, b)
#define VMUL(a, b)                     vmul_f32(a, b)
#define VFMA(a, b, c)                  vmla_f32(c, b, a)
#define VFNMS(a, b, c)                 vmls_f32(c, b, a)
#define VFMS(a, b, c)                  vmla_f32(vneg_f32(c), b, a)

#define LD(x, ivs, aligned_like)       vld1_f32((float32_t *)x)
#define LDA(x, ivs, aligned_like)      vld1_f32((float32_t *)x)

#define ST(x, v, ovs, aligned_like)    vst1_f32((float32_t *)x, v)
#define STA(x, v, ovs, aligned_like)   vst1_f32((float32_t *)x, v)

#define STM2(x, v, ovs, aligned_like)  vst1_f32((float32_t *)x, v)

#define STN2(x, v0, v1, ovs)           /* not using STN2, using STM2 */

static inline void STM4(R *x, V v, INT ovs, R *aligned_like)
{
     *x         = vget_lane_f32(v,0);
     *(x + ovs) = vget_lane_f32(v,1);
}

#define STN4(x, v0, v1, v2, v3, ovs)   /* not using STN4, using STM4 */

#define FLIP_RI(x)                     vrev64_f32(x)

static inline V VCONJ(V x)
{
     const V pm = {1.0, -1.0};
     return VMUL(x, pm);
}

static inline V VBYI(V x)
{
     return FLIP_RI(VCONJ(x));
}

static inline V VZMUL(V a, V b)
{
     V tr = vdup_n_f32(vget_lane_f32(a,0));
     V ti = vdup_n_f32(vget_lane_f32(a,1));
     tr = VMUL(b, tr);
     b = VBYI(b);
#ifdef HAVE_FMA
     return VFMA(ti,b,tr);
#else
     return VADD(tr,VMUL(ti,b));
#endif /* HAVE_FMA */
}

#define VFMAI(a, b)                    VADD(b, VBYI(a))
#define VFNMSI(a, b)                   VSUB(b, VBYI(a))
#define VZMULJ(a, b)                   VZMUL(VCONJ(a), b)
#define VZMULI(a, b)                   VBYI(VZMUL(a, b))
#define VZMULIJ(a, b)                  VBYI(VZMULJ(a, b))
#define VFMACONJ(a, b)                 VADD(VCONJ(a), b)
#define VFMSCONJ(a, b)                 VSUB(VCONJ(a), b)
#define VFNMSCONJ(a, b)                VSUB(b, VCONJ(a))

#define VTW1(v,x)                      {TW_CEXP,v,x}
#define TWVL1                          1
#define VTW2(v,x)	               {TW_COS,v,x}, {TW_COS,v,x}, {TW_SIN,v,-x}, {TW_SIN,v,x}
#define TWVL2                          2
#define VTW3(v,x)                      VTW1(v,x)
#define TWVL3                          TWVL1
#define VTWS(v,x)		       {TW_COS,v,x}, {TW_COS,v+1,x}, {TW_SIN,v,x}, {TW_SIN,v+1,x}
#define TWVLS                          (2 * VL)

static inline V BYTW1(const R *t, V sr)
{
     return VZMUL(vld1_f32((float32_t *)t),sr);
}

static inline V BYTWJ1(const R *t, V sr)
{
     return VZMULJ(vld1_f32((float32_t *)t),sr);
}

static inline V BYTW2(const R *t, V sr)
{
     const V *tv = (const V *)t;
     return VADD(VMUL(*tv,sr),VMUL(*(tv + 1),FLIP_RI(sr)));
}

static inline V BYTWJ2(const R *t, V sr)
{
     const V *tv = (const V *)t;
     return VSUB(VMUL(*tv,sr),VMUL(*(tv + 1),FLIP_RI(sr)));
}

#endif /* __ARM_NEON__ */
