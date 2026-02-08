/** @file
  Processor or Compiler specific defines and types for RISC-V 64.

  Copyright (c) 2023, Manas Kamal Choudhury. All rights reserved.<BR>
**/

#ifndef __PROCESSOR_BIND_H__
#define __PROCESSOR_BIND_H__

///
/// Define the processor type so other code can make processor based choices
///
#define MDE_CPU_RISCV64

//
// Make sure we are using the correct packing rules per EFI specification
//
#if !defined(__GNUC__) && !defined(__ASSEMBLER__)
#pragma pack()
#endif

#if defined(_MSC_EXTENSIONS)

//
// Disable some level 4 compilation warnings
//
#pragma warning ( disable : 4214 )
#pragma warning ( disable : 4100 )
#pragma warning ( disable : 4057 )
#pragma warning ( disable : 4127 )
#pragma warning ( disable : 4505 )
#pragma warning ( disable : 4206 )
#pragma warning ( disable : 4701 )
#pragma warning ( disable : 4703 )

  //
  // use Microsoft* C compiler dependent integer width types
  //
  typedef unsigned __int64    UINT64;
  typedef __int64             INT64;
  typedef unsigned __int32    UINT32;
  typedef __int32             INT32;
  typedef unsigned short      UINT16;
  typedef unsigned short      CHAR16;
  typedef short               INT16;
  typedef unsigned char       BOOLEAN;
  typedef unsigned char       UINT8;
  typedef char                CHAR8;
  typedef signed char         INT8;

#else

  //
  // Assume standard RISC-V 64 alignment.
  //
  typedef unsigned long long  UINT64;
  typedef long long           INT64;
  typedef unsigned int        UINT32;
  typedef int                 INT32;
  typedef unsigned short      UINT16;
  typedef unsigned short      CHAR16;
  typedef short               INT16;
  typedef unsigned char       BOOLEAN;
  typedef unsigned char       UINT8;
  typedef char                CHAR8;
  typedef signed char         INT8;

#endif

///
/// Unsigned value of native width.
///
typedef UINT64  UINTN;

///
/// Signed value of native width.
///
typedef INT64   INTN;

//
// Processor specific defines
//

#define MAX_BIT     0x8000000000000000ULL
#define MAX_2_BITS  0xC000000000000000ULL
#define MAX_ADDRESS   0xFFFFFFFFFFFFFFFFULL
#define MAX_INTN   ((INTN)0x7FFFFFFFFFFFFFFFULL)
#define MAX_UINTN  ((UINTN)0xFFFFFFFFFFFFFFFFULL)
#define MIN_INTN   (((INTN)-9223372036854775807LL) - 1)

///
/// The stack alignment required for RISC-V 64
///
#define CPU_STACK_ALIGNMENT  16

#define DEFAULT_PAGE_ALLOCATION_GRANULARITY   (0x1000)
#define RUNTIME_PAGE_ALLOCATION_GRANULARITY   (0x10000)

#define EFIAPI

// When compiling with Clang, we still use GNU as for the assembler, so we still
// need to define the GCC_ASM* macros.
#if defined(__GNUC__) || defined(__clang__)
  #define ASM_GLOBAL .globl
  #define GCC_ASM_EXPORT(func__)  \
         .global  _CONCATENATE (__USER_LABEL_PREFIX__, func__)    ;\
         .type ASM_PFX(func__), %function

  #define GCC_ASM_IMPORT(func__)  \
         .extern  _CONCATENATE (__USER_LABEL_PREFIX__, func__)

#endif

#define FUNCTION_ENTRY_POINT(FunctionPointer) (VOID *)(UINTN)(FunctionPointer)

#ifndef __USER_LABEL_PREFIX__
#define __USER_LABEL_PREFIX__
#endif

#endif
