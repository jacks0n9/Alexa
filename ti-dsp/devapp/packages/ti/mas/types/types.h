/*!
 * Module `types` for package `ti.mas.types`
 *
 * This file contains descriptions for module `types` of the package
 * `{@link ti.mas.types}`
 *
 * Module `types` defines the basic integral data types that are
 * to be used by all other packages within the `ti.mas` tree. The
 * suggested usage for data types is provided with individual
 * definitions. One should strictly follow the rules for data
 * types usage in order to facilitate portability and 
 * reusability of software.
 *
 * The usage of standard ANSI C types is in general discouraged since
 * they are not portable. They are listed here for completeness. Suggested
 * usage is provided with each individual data type. The following ANSI C
 * types are not defined here, but within the C language implementation:
 *
 * @p
 *   <tt>int</tt> - ANSI C signed integer. Defined by local compiler implementation.
 *           Number of bits used in representation is machine dependent.
 *           <b>Suggested usage:</b> should be avoided to minimize portability
 *           problems.</p>
 *
 *   <tt>long</tt> - ANSI C signed long integer. Defined by local compiler
 *            implementation. Number of bits used in representation is machine
 *            dependent.
 *            <b>Suggested usage:</b> should be avoided to minimize portability
 *            problems.</p>
 *
 *   <tt>enum</tt> - ANSI C enumeration integer. Defined by local compiler
 *            implementation. Number of bits used in representation is machine
 *            dependent.
 *            <b>Suggested usage:</b> only when introducing symbolic constants that
 *            are not bit maps. It should never be used for defining or
 *            allocating variables since it may not be memory efficient.</p>
 *
 *   <tt>float</tt> - ANSI C single precision float. Defined by local compiler
 *             implementation. Number of bits and floating point format used in
 *             representation are machine dependent.
 *             <b>Suggested usage:</b> should be avoided. Instead, one should use
 *             Fract or LFract types. Portability across platforms is achieved
 *             by using portability header files that independently define
 *             fractional constants as float's or int's depending on hardware
 *             architecture in use and desired target number representation.</p>
 *
 *  <tt>double</tt> - ANSI C double precision float. Same as float, but double
 *             precision.</p>
 *
 *   <tt>char</tt> - ANSI C signed character. Defined by local compiler
 *            implementation. Number of bits used in representation is machine
 *            dependent.
 *            <b>Suggested usage:</b> character strings. Should be avoided to
 *            minimize portability problems and memory overhead. Some
 *            platforms may use more than 8 bits for storage of characters.</p>
 *
 *   <tt>void*</tt> - ANSI C void pointer. <b>Suggested usage:</b> pointers to structures
 *             that are passed between software modules in real time, or
 *             pointers to data objects that may use different type definitions
 *             in different implementations. Pointer to objects that need to
 *             hide their actual type definition within a software module.</p>
 *
 * The usage of fixed size RTSC data types if not allowed under any
 * circumstances. Here is the description of the fixed size RTSC types:</p>
 *
 *  <tt>Bits8</tt> - Non portable unsigned byte. It has exactly 8 bits of storage.
 *            This type is equivalent to a commonly known <tt>UINT8</tt> data type.
 *            <b>Suggested usage:</b> portable code never uses this type.</p>
 *
 *  <tt>Bits16</tt> - Non portable unsigned word. It has exactly 16 bits of storage.
 *             This type is equivalent to a commonly known <tt>UINT16</tt> data type.
 *             <b>Suggested usage:</b> portable code never uses this type.</p>
 *
 *  <tt>Bits32</tt> - Non portable unsigned long word. It has exactly 32 bits of
 *             storage. This type is equivalent to a commonly known <tt>UINT32</tt> data
 *             type.
 *             <b>Suggested usage:</b> portable code never uses this type.</p>
 * @p
 *
 * @a(BIT_SIZES)
 *
 *  Each data type that is defined in this module has two constants
 *  associated with it.
 *
 *  `TYP_<name>_SIZE` and `TYP_<name>_PREC`
 *
 *  `TYP_<name>_SIZE` is defined as the number of bits that will be used
 *  when storing type `<name>` in memory. For example, `TYP_TINT_SIZE` would
 *  be the number of bits used when storing an element of type `tint`.
 *
 *  `TYP_<name>_PREC` is defined as the number of bits that will be used
 *  when doing numerical calculations with elements of type `<name>`. For
 *  example, `TYP_TINT_PREC` would be the number of bits used when doing
 *  calculations with an element of type `tint`.
 *
 *  In general, `TYP_<name>_SIZE >= TYP_<name>_PREC`.
 *
 *  The bit sizes for data types on different hardware platforms may vary.
 *  One should use size and precision constants in situations where portable
 *  code is needed. Making assumptions about exact bit sizes is not good since
 *  that results in code that is not portable. However, if such assumptions
 *  have to be made, the size and precision constants can be used to make
 *  assertions that could prevent compilation or execution of software for
 *  cases that would not be handled properly.
 *
 * @a(USAGE)
 *
 *  To use data types defined by this module one needs to include the
 *  `types.h` file:
 *
 * @p(code)
 * #include <ti/mas/types/types.h>
 * @p
 *
 * @a(Copyright)
 * @p
 * &#169; 2006 Texas Instruments, Inc.
 * @p
 */

#ifndef _TYPES_H
#define _TYPES_H

/** @defgroup  TYPES */

/** @ingroup TYPES */
/* @{ */
#include <stdint.h>     /* C99 standard integer types */

/* Define target symbols for ease of use */
#ifdef ti_targets_C54
#define _TYPES_C54 1
#else
#define _TYPES_C54 0
#endif

#ifdef ti_targets_C54_far
#define _TYPES_C54_FAR 1
#else
#define _TYPES_C54_FAR 0
#endif

#ifdef ti_targets_C55
#define _TYPES_C55 1
#else
#define _TYPES_C55 0
#endif

#ifdef ti_targets_C55_large
#define _TYPES_C55_LARGE 1
#else
#define _TYPES_C55_LARGE 0
#endif

#ifdef ti_targets_C64
#define _TYPES_C64 1
#else
#define _TYPES_C64 0
#endif

#ifdef ti_targets_C64_big_endian
#define _TYPES_C64_BIG_ENDIAN 1
#else
#define _TYPES_C64_BIG_ENDIAN 0
#endif

#ifdef ti_targets_C674
#define _TYPES_C674 1
#else
#define _TYPES_C674 0
#endif

#ifdef ti_targets_C674_big_endian
#define _TYPES_C674_BIG_ENDIAN 1
#else
#define _TYPES_C674_BIG_ENDIAN 0
#endif



#ifdef ti_targets_C64P
#define _TYPES_C64P 1
#else
#define _TYPES_C64P 0
#endif

#ifdef ti_targets_C64P_big_endian
#define _TYPES_C64P_BIG_ENDIAN 1
#else
#define _TYPES_C64P_BIG_ENDIAN 0
#endif

#ifdef ti_targets_C66
#define _TYPES_C66 1
#else
#define _TYPES_C66 0
#endif

#ifdef ti_targets_C66_big_endian
#define _TYPES_C66_BIG_ENDIAN 1
#else
#define _TYPES_C66_BIG_ENDIAN 0
#endif

#ifdef gnu_targets_MVArm9
#define _TYPES_MVARM9 1
#else
#define _TYPES_MVARM9 0
#endif

#ifdef ti_targets_arm_Arm11
#define _TYPES_ARM_ARM11 1
#else
#define _TYPES_ARM_ARM11 0
#endif


#ifdef ti_targets_arm_Arm11_big_endian
#define _TYPES_ARM_ARM11_BIG_ENDIAN 1
#else
#define _TYPES_ARM_ARM11_BIG_ENDIAN 0
#endif

#ifdef gnu_targets_arm_GCArmv6
#define _TYPES_ARM_GCARMV6 1
#else
#define _TYPES_ARM_GCARMV6 0
#endif

#ifdef gnu_targets_arm_GCArmv7A
#define _TYPES_ARM_GCARMV7A 1
#else
#define _TYPES_ARM_GCARMV7A 0
#endif


#ifdef microsoft_targets_arm_WinCE
#define _TYPES_WINCE 1
#else
#define _TYPES_WINCE 0
#endif

#ifdef microsoft_targets_VC98
#define _TYPES_VC98 1
#else
#define _TYPES_VC98 0
#endif


/* Include target specific version of some types and type sizes.
 * These are automatically generated using the template file. */

#if _TYPES_C54
#include <ti/mas/types/c54/types.h>
#elif _TYPES_C54_FAR
#include <ti/mas/types/c54f/types.h>
#elif _TYPES_C55
#include <ti/mas/types/c55/types.h>
#elif _TYPES_C55_LARGE
#include <ti/mas/types/c55L/types.h>
#elif _TYPES_C64
#include <ti/mas/types/c64/types.h>
#elif _TYPES_C64_BIG_ENDIAN
#include <ti/mas/types/c64e/types.h>
#elif _TYPES_C674
#include <ti/mas/types/c674/types.h>
#elif _TYPES_C674_BIG_ENDIAN
#include <ti/mas/types/c674e/types.h>
#elif _TYPES_C64P
#include <ti/mas/types/c64P/types.h>
#elif _TYPES_C64P_BIG_ENDIAN
#include <ti/mas/types/c64Pe/types.h>
#elif _TYPES_C66
#include <ti/mas/types/c66/types.h>
#elif _TYPES_C66_BIG_ENDIAN
#include <ti/mas/types/c66e/types.h>
#elif _TYPES_MVARM9
#include <ti/mas/types/mvarm9/types.h>
#elif _TYPES_ARM_GCARMV6
#include <ti/mas/types/gcarmv6/types.h>
#elif _TYPES_ARM_GCARMV7A
#include <ti/mas/types/gcarmv7a/types.h>
#elif _TYPES_WINCE
#include <ti/mas/types/wince/types.h>
#elif _TYPES_VC98
#include <ti/mas/types/vc98/types.h>
#elif _TYPES_ARM_ARM11
#define _LITTLE_ENDIAN  1
#include <ti/mas/types/tiarmv6le/types.h>
#elif _TYPES_ARM_ARM11_BIG_ENDIAN
#define _BIG_ENDIAN  1
#include <ti/mas/types/tiarmv6be/types.h>
#else
#error invalid target
#endif

/***********************************************************************
 * DATA TYPE DEFINITION: Basic Integral Data Types.
 ***********************************************************************
 * DESCRIPTION: The following are basic integral data types, 
 *              with descriptions of their suggested usage.
 **********************************************************************/

/*-----------------------*/
/* Standard ANSI C Types */
/*-----------------------*/

/* int    - signed integer */
/* long   - signed long integer */
/* float  - single precision floating point number */
/* char   - character that is a part of a string */

/* uint, ulong, and uchar types have been removed
 *       since they were not portable */

/*-------------------------*/
/* MAS Integral Data Types */
/*-------------------------*/

/*! Portable signed short integer.
 *
 * Portable signed short integer has at least 8 bits of precision.
 *
 * <b>Suggested usage:</b> signed short integers that fall in [-128, 127] range
 * and occupy the smallest number of bits (8 if possible). Some platforms
 * may use different number of bits for short int and tshort types.
 * Hence, tshort is a preferred type that facilitates portability and
 * improves performance.
 */
typedef int_least8_t  tshort;

/*! Portable unsigned short integer.
 *
 * Portable unsigned short integer has at least 8 bits of precision.
 * It is the same as `{@link #tshort}` but unsigned. It may be used for very short bit
 * maps (up to 8 bits in length).
 */
typedef uint_least8_t tushort;

/*! Portable signed integer.
 *
 * Portable signed integer has at least 16 bits of precision.
 *
 * <b>Suggested usage:</b> signed integers that fall in [-32768, 32767] range and
 * occupy the smallest number of bits (16 if possible). Some platforms may
 * use different number of bits for int and tint types. Hence, tint is a
 * preferred type that facilitates portability and improves performance.
 */
typedef int_least16_t   tint;

/*! Portable unsigned integer.
 *
 * Portable unsigned integer has at least 16 bits of precision.
 * It is the same as {@link #tint} but unsigned. It may be used for short bit maps
 * (up to 16 bits in length).
 */
typedef uint_least16_t  tuint;

/*! Portable signed long integer.
 *
 * Portable signed long integer has at least 32 bits of precision.
 *
 * <b>Suggested usage:</b> signed long integers that fall in [-2^31, 2^31-1] range
 * and occupy the smallest number of bits (32 if possible). Some platforms
 * may use different number of bits for long and tlong types. Hence, tlong
 * is a preferred type that facilitates portability and improves
 * performance.
 */
typedef int_least32_t   tlong;


/*! Portable unsigned long integer.
 *
 * Portable unsigned long integer has at least 32 bits of precision.
 * It is the same as {@link #tlong} but unsigned. It may be used for bit maps
 * (up to 32 bits in length).
 */
typedef uint_least32_t  tulong;

/*! Portable machine word.
 *
 * Portable machine word is the smallest addressable and dynamically
 * allocatable integral memory element for a given target platform.
 * Number of bits used in representation is machine dependent. Most often
 * it corresponds to the number of bits used in representing a char.
 * The sizeof(tword) is always one.
 *
 * <b>Suggested usage:</b> in situations where exact number of bits or bytes is
 * important, e.g. when packing bytes into words, when implementing general
 * purpose circular buffers or queues, when implementing arbitrary length
 * bit fields and associated support routines. When assembling and
 * disassembling packets and arbitrary bit maps.</p>
 *
 * <b>IMPORTANT:</b> By using tword, one can implement portable and reusable code
 * even in situations where memory objects need to use the exact number of
 * bits. The software in such situations should not use types like Bits8 or
 * similar to avoid portability problems.</p>
 *
 * <b>IMPLEMENTATION NOTE:</b> Although we could have used UInt8 to define this
 * type, we decided to use uchar instead. This was done in order to remain
 * true to the actual type description.</p>
 */
typedef uint_least8_t tword;
  
/* Bitmap type(s) have no place in this package. They have to be
 * handled by the legacy software, e.g. ggdsp VOB. */
/* typedef unsigned int    tbmap; */ 

/******************************************************************************
 * DATA TYPE DEFINITION: Derived Integral Data Types
 ******************************************************************************
 * DESCRIPTION: The following are integral data types derived from the basic
 *              integral types, with descriptions of their suggested usage.
 *
 *****************************************************************************/

/*! Portable unsigned buffer size integer.
 *
 * Portable unsigned buffer size integer has at least 16 bits of precision.
 * Some platforms may have to define this type to be 32 bit long in
 * case huge buffers would be supported by the software architecture.
 *
 * <b>Suggested usage:</b> specifying the number of <tt>{@link #tword}</tt>'s in a buffer. This type
 * is not the same as size_t which is used in ANSI C for a similar purpose.
 */
typedef tuint tsize;

/*! Portable logic variable type.
 *
 * Portable logic variable type has only two possible values: FALSE (zero)
 * and TRUE (one).
 *
 * <b>Suggested usage:</b> within the logic expressions. One should avoid storing
 * too many boolean variables in permanent memory using this type. It is
 * more efficient to implement the bit fields instead. For example, types
 * like <tt>{@link #tushort}</tt>, <tt>{@link #tuint}</tt>, <tt>{@link #tulong}</tt>, or
 * <tt>{@link #tword}</tt> could be used for such purposes.
 */
typedef tshort  tbool;

/* These definitions are already present in xdc. So don't include in IDL.
 * Leads to redefinition warnings. Currently, we still check if they were
 * defined to avoid conflicts with dspbios and similar packages. */
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef NULL
#define NULL 0
#endif

/*! Portable fractional number.
 *
 * Portable fractional number has at least 16 bits of precision.
 *
 * <b>Suggested usage on fixed point platforms:</b> by default this is a rational
 * number in [-1, 1) range. On 16-bit platforms, it is in Q15 format.
 * However, any Q-format may be used instead. Fractional numbers in Q0
 * format fall in [-32768, 32767] range and occupy the smallest number of
 * bits (16 if possible). One should not assume that the Fract type has
 * more than 16 bits of precision. Internally this type is based on tint
 * type.
 *
 * <b>Suggested usage on floating point platforms:</b> Q-format is ignored. Fract
 * type is equivalent to float type. When portability between floating
 * point and fixed point implementations is required, the semantics of the
 * Fract type is completely determined by the fixed point platform
 * definition.
 *
 * <b>NOTE:</b> Sometimes it makes sense to use fixed point math even on a
 * floating point hardware platform. In such a case the Fract type would
 * be defined as if it was on a fixed point platform and one would not
 * use any floating point types although they might be supported by
 * by the hardware.
 * 
 */
typedef tint Fract;

/*! Portable long fractional number.
 *
 * Portable long fractional number has at least 32 bits of precision.
 *
 * <b>Suggested usage on fixed point platforms:</b> by default this is a rational
 * number in [-1, 1) range. On 16-bit platforms, it is in Q31 format.
 * However, any Q-format may be used instead. Fractional numbers in Q0
 * format fall in [-2^31, 2^31-1] range and occupy the smallest number of
 * bits (32 if possible). One should not assume that the LFract type has
 * more than 32 bits of precision. Internally this type is based on tlong
 * type. This type is often used in linear power and energy calculations as
 * well as in IIR filter implementations.
 *
 * <b>Suggested usage on floating point platforms:</b> Q-format is ignored. LFract
 * type is equivalent to double type. When portability between floating
 * point and fixed point implementations is required, the semantics of the
 * LFract type is completely determined by the fixed point platform
 * definition.
 * 
 */
typedef tlong LFract;

/*! Portable unsigned fractional number.
 *
 * Portable unsigned fractional number has at least 16 bits of precision.
 * It is the same as Fract but unsigned. On floating point platforms UFract
 * is used only when the portability with fixed point implementation is
 * required.
 */
typedef tuint UFract;

/*! Portable unsigned long fractional number.
 *
 * Portable unsigned long fractional number has at least 32 bits of
 * precision. It is the same as LFract but unsigned. On floating point
 * platforms ULFract is used only when the portability with fixed point
 * implementation is required.
 */
typedef tulong ULFract;

/*! Portable linear sample.
 *
 * Portable linear sample has at least 16 bits of precision.
 *
 * <b>Suggested usage:</b> linear signal samples are always in [-32768, 32767]
 * range. They may be obtained from the PCM compressed samples using
 * appropriate expansion lookup table. Internally, linSample is based on
 * the Fract type. Signal processing algorithms assume that the analog sine
 * wave of +3dBm power is scaled into a digital sine wave that has a full
 * scale amplitude of 32768. On a fixed point platform linear samples are
 * in Q0 format.
 *
 * <b>NOTE:</b> On floating point platforms linear samples would also have the
 * range [-32768.0, 32767.0].
 */
typedef tint linSample;
 
/*! The log2 of the maximum linear sample amplitude.
 *
 * The log2 of the maximum linear sample amplitude is always 15. Hence, the
 * maximum linear sample amplitude is always in the range of
 * [-32768,32767]=[-2^15,2^15-1]. This range is used for both fixed point
 * and floating point platforms.
 */
#define TYP_LINSAMPLE_MAG   15

/*------------------------------------------------*/
/* Special type related definitions and macros    */
/*------------------------------------------------*/

/*! The log2 alignment for structures.
 *
 * TYP_STRUCT_LOG2ALIGN
 *
 * Structures need to be aligned on a certain log2 address boundary.
 * This parameter specifies this log2 alignment. For example, if the
 * structures have to be aligned on even word boundary one would specify
 * this parameter to be euqal to one. If no alignment is needed, this
 * parameters would be equal to zero.
 *
 * This parameter may be used with the `{@link #typChkAlign()}` macro.
 */
/* Platform dependent definition: moved to appropriate platform dependent header files */

/*! Macro for checking log2 alignment of a base address.
 *
 * This macro may have to be used when receiving a pointer to a buffer that
 * requires specific `log2` alignment.
 *
 * @param[in] base  Base address of a buffer for which we want to check alignment.
 *
 * @param[in] l2a   The `log2` alignment that is requested. For example, for alignment
 *               on even number of words one should specify 1 which
 *               corresponds to 2^1 alignment.
 *
 * @retval `FALSE` - The base address is not aligned on l2a boundary.
 * @retval `TRUE`  - The base address is properly aligned on l2a boundary.
 *
 * <b> Typical Usage: </b> 
 *
 * <code>
 *    if (!typChkAlign(bufp->base, bufp->log2align)) { <BR>
 *      <base address not aligned, generate error> <BR>
 *    } <BR>
 * </code> 
 
 */
#define typChkAlign(base, l2a)    \
       (((tulong)(base)&(((l2a)==TYP_TULONG_SIZE)?~0UL:~(~0UL<<(l2a))))==0UL)

/* @} */ /* ingroup */
#endif
/* nothing past this point */
