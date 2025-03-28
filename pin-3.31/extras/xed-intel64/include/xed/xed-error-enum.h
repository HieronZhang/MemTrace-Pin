/* BEGIN_LEGAL 

Copyright (c) 2024 Intel Corporation

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
  
END_LEGAL */
/// @file xed-error-enum.h

// This file was automatically generated.
// Do not edit this file.

#if !defined(XED_ERROR_ENUM_H)
# define XED_ERROR_ENUM_H
#include "xed-common-hdrs.h"
#define XED_ERROR_NONE_DEFINED 1
#define XED_ERROR_BUFFER_TOO_SHORT_DEFINED 1
#define XED_ERROR_GENERAL_ERROR_DEFINED 1
#define XED_ERROR_INVALID_FOR_CHIP_DEFINED 1
#define XED_ERROR_BAD_REGISTER_DEFINED 1
#define XED_ERROR_BAD_LOCK_PREFIX_DEFINED 1
#define XED_ERROR_BAD_REP_PREFIX_DEFINED 1
#define XED_ERROR_BAD_LEGACY_PREFIX_DEFINED 1
#define XED_ERROR_BAD_REX_PREFIX_DEFINED 1
#define XED_ERROR_BAD_MAP_DEFINED 1
#define XED_ERROR_BAD_EVEX_V_PRIME_DEFINED 1
#define XED_ERROR_BAD_EVEX_Z_NO_MASKING_DEFINED 1
#define XED_ERROR_NO_OUTPUT_POINTER_DEFINED 1
#define XED_ERROR_NO_AGEN_CALL_BACK_REGISTERED_DEFINED 1
#define XED_ERROR_BAD_MEMOP_INDEX_DEFINED 1
#define XED_ERROR_CALLBACK_PROBLEM_DEFINED 1
#define XED_ERROR_GATHER_REGS_DEFINED 1
#define XED_ERROR_INSTR_TOO_LONG_DEFINED 1
#define XED_ERROR_INVALID_MODE_DEFINED 1
#define XED_ERROR_BAD_EVEX_LL_DEFINED 1
#define XED_ERROR_BAD_REG_MATCH_DEFINED 1
#define XED_ERROR_LAST_DEFINED 1
typedef enum {
  XED_ERROR_NONE, ///< There was no error
  XED_ERROR_BUFFER_TOO_SHORT, ///< There were not enough bytes in the given buffer
  XED_ERROR_GENERAL_ERROR, ///< XED could not decode the given instruction
  XED_ERROR_INVALID_FOR_CHIP, ///< The instruciton is not valid for the specified chip
  XED_ERROR_BAD_REGISTER, ///< XED could not decode the given instruction because an invalid register encoding was used.
  XED_ERROR_BAD_LOCK_PREFIX, ///< A lock prefix was found where none is allowed.
  XED_ERROR_BAD_REP_PREFIX, ///< An F2 or F3 prefix was found where none is allowed.
  XED_ERROR_BAD_LEGACY_PREFIX, ///< A 66, F2 or F3 prefix was found where none is allowed.
  XED_ERROR_BAD_REX_PREFIX, ///< A REX prefix was found where none is allowed.
  XED_ERROR_BAD_MAP, ///< An illegal value for the MAP field was detected in the instruction.
  XED_ERROR_BAD_EVEX_V_PRIME, ///< EVEX.V'=0 was detected in a non-64b mode instruction.
  XED_ERROR_BAD_EVEX_Z_NO_MASKING, ///< EVEX.Z!=0 when EVEX.aaa==0
  XED_ERROR_NO_OUTPUT_POINTER, ///< The output pointer for xed_agen was zero
  XED_ERROR_NO_AGEN_CALL_BACK_REGISTERED, ///< One or both of the callbacks for xed_agen were missing.
  XED_ERROR_BAD_MEMOP_INDEX, ///< Memop indices must be 0 or 1.
  XED_ERROR_CALLBACK_PROBLEM, ///< The register or segment callback for xed_agen experienced a problem
  XED_ERROR_GATHER_REGS, ///< The index, dest and mask regs for AVX2 gathers must be different.
  XED_ERROR_INSTR_TOO_LONG, ///< Full decode of instruction would exeed 15B.
  XED_ERROR_INVALID_MODE, ///< The instruction was not valid for the specified mode
  XED_ERROR_BAD_EVEX_LL, ///< EVEX.LL must not ==3 unless using embedded rounding
  XED_ERROR_BAD_REG_MATCH, ///< Some registers must not match for this instruction (e.g. source with dest or dest with dest).
  XED_ERROR_LAST
} xed_error_enum_t;

/// This converts strings to #xed_error_enum_t types.
/// @param s A C-string.
/// @return #xed_error_enum_t
/// @ingroup ENUM
XED_DLL_EXPORT xed_error_enum_t str2xed_error_enum_t(const char* s);
/// This converts strings to #xed_error_enum_t types.
/// @param p An enumeration element of type xed_error_enum_t.
/// @return string
/// @ingroup ENUM
XED_DLL_EXPORT const char* xed_error_enum_t2str(const xed_error_enum_t p);

/// Returns the last element of the enumeration
/// @return xed_error_enum_t The last element of the enumeration.
/// @ingroup ENUM
XED_DLL_EXPORT xed_error_enum_t xed_error_enum_t_last(void);
#endif
