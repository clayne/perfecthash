//
// Auto-generated.
//

DECLSPEC_ALIGN(16)
const CHAR CompiledPerfectHashCHeaderRawCStr[] =
    "\n"
    "//\n"
    "// Begin CompiledPerfectHash.h.\n"
    "//\n"
    "\n"
    "/*++\n"
    "\n"
    "Copyright (c) 2018-2024 Trent Nelson <trent@trent.me>\n"
    "\n"
    "Module Name:\n"
    "\n"
    "    CompiledPerfectHash.h\n"
    "\n"
    "Abstract:\n"
    "\n"
    "    This is the main public header file for the compiled perfect hash library.\n"
    "    It defines structures and functions related to loading and using compiled\n"
    "    perfect hash tables.\n"
    "\n"
    "--*/\n"
    "\n"
    "#ifndef _COMPILED_PERFECT_HASH_H_\n"
    "#define _COMPILED_PERFECT_HASH_H_\n"
    "\n"
    "\n"
    "#ifdef __cplusplus\n"
    "extern \"C\" {\n"
    "#endif\n"
    "\n"
    "#include <stdint.h>\n"
    "\n"
    "//\n"
    "// Static inline C versions of rotate and bit extraction.\n"
    "//\n"
    "\n"
    "//\n"
    "// Equivalent to _rotl intrinsic on x64.\n"
    "//\n"
    "\n"
    "static inline\n"
    "uint32_t\n"
    "RotateLeft32_C(\n"
    "    uint32_t a,\n"
    "    uint32_t b\n"
    "    )\n"
    "{\n"
    "    b &= 31;\n"
    "    return (a << b) | (a >> (32 - b));\n"
    "}\n"
    "\n"
    "//\n"
    "// Equivalent to _rotr intrinsic on x64.\n"
    "//\n"
    "\n"
    "static inline\n"
    "uint32_t\n"
    "RotateRight32_C(\n"
    "    uint32_t a,\n"
    "    uint32_t b\n"
    "    )\n"
    "{\n"
    "    b &= 31;\n"
    "    return (a >> b) | (a << (32 - b));\n"
    "}\n"
    "\n"
    "//\n"
    "// Equivalent to _rotl64 intrinsic on x64.\n"
    "//\n"
    "\n"
    "static inline\n"
    "uint64_t\n"
    "RotateLeft64_C(\n"
    "    uint64_t a,\n"
    "    uint64_t b\n"
    "    )\n"
    "{\n"
    "    b &= 63;\n"
    "    return (a << b) | (a >> (64 - b));\n"
    "}\n"
    "\n"
    "//\n"
    "// Equivalent to _rotr64 intrinsic on x64.\n"
    "//\n"
    "\n"
    "static inline\n"
    "uint64_t\n"
    "RotateRight64_C(\n"
    "    uint64_t a,\n"
    "    uint64_t b\n"
    "    )\n"
    "{\n"
    "    b &= 63;\n"
    "    return (a >> b) | (a << (64 - b));\n"
    "}\n"
    "\n"
    "//\n"
    "// Equivalent to _pext_u64 intrinsic on x64.\n"
    "//\n"
    "\n"
    "static inline\n"
    "uint64_t\n"
    "ExtractBits64_C(\n"
    "    uint64_t value,\n"
    "    uint64_t mask\n"
    "    )\n"
    "{\n"
    "    uint64_t result = 0;\n"
    "    uint64_t bit_position = 0;\n"
    "\n"
    "    while (mask != 0) {\n"
    "        if (mask & 1) {\n"
    "            result |= (value & 1) << bit_position;\n"
    "            bit_position++;\n"
    "        }\n"
    "\n"
    "        mask >>= 1;\n"
    "        value >>= 1;\n"
    "    }\n"
    "\n"
    "    return result;\n"
    "}\n"
    "\n"
    "//\n"
    "// Platform-dependent defines.\n"
    "//\n"
    "\n"
    "#ifdef _WIN32\n"
    "\n"
    "#include <sal.h>\n"
    "\n"
    "//\n"
    "// The intrinsics headers trigger a lot of warnings when /Wall is on.\n"
    "//\n"
    "\n"
    "#pragma warning(push)\n"
    "#pragma warning(disable: 4255 4514 4668 4820 28251)\n"
    "#include <intrin.h>\n"
    "#include <mmintrin.h>\n"
    "#pragma warning(pop)\n"
    "\n"
    "#define DEBUGBREAK __debugbreak\n"
    "#define CPHCALLTYPE __stdcall\n"
    "#ifndef FORCEINLINE\n"
    "#define FORCEINLINE __forceinline\n"
    "#endif\n"
    "#elif defined(__linux__) || defined(__APPLE__)\n"
    "#define CPHCALLTYPE\n"
    "#if defined(__clang__) || defined(__GNUC__)\n"
    "#   ifdef __arm64__\n"
    "#       ifdef __ARM_FEATURE_SVE\n"
    "#           include <arm_sve.h>\n"
    "#       else // !__ARM_FEATURE_SVE\n"
    "#           include <arm_neon.h>\n"
    "#       endif // __ARM_FEATURE_SVE\n"
    "\n"
    "#define RotateRight32 __builtin_rotateright32\n"
    "#define RotateLeft32  __builtin_rotateleft32\n"
    "#define RotateRight64 __builtin_rotateright64\n"
    "#define RotateLeft64  __builtin_rotateleft64\n"
    "#define ExtractBits64 ExtractBits64_C\n"
    "\n"
    "#   else // !__arm64__\n"
    "#       include <x86intrin.h>\n"
    "\n"
    "#define RotateRight32 _rotr\n"
    "#define RotateLeft32  _rotl\n"
    "#define RotateRight64 _rotr64\n"
    "#define RotateLeft64  _rotl64\n"
    "#define ExtractBits64 _pext_u64\n"
    "#define Crc32u32      _mm_crc32_u32\n"
    "\n"
    "#   endif // __arm64__\n"
    "#else\n"
    "#error Unrecognized compiler.\n"
    "#endif\n"
    "\n"
    "#ifndef PH_UNITY\n"
    "#include <no_sal2.h>\n"
    "#endif\n"
    "#ifndef FORCEINLINE\n"
    "#define FORCEINLINE static inline __attribute__((always_inline))\n"
    "#define DEBUGBREAK __builtin_trap\n"
    "#endif\n"
    "#else\n"
    "#error Unsupported platform.\n"
    "#endif\n"
    "\n"
    "#if defined(COMPILED_PERFECT_HASH_DLL_BUILD)\n"
    "#define CPHAPI __declspec(dllexport)\n"
    "#elif defined(COMPILED_PERFECT_HASH_EXE_BUILD)\n"
    "#define CPHAPI __declspec(dllimport)\n"
    "#else\n"
    "#define CPHAPI\n"
    "#endif\n"
    "\n"
    "#if defined(COMPILED_PERFECT_HASH_DLL_BUILD)\n"
    "#define CPHAPI __declspec(dllexport)\n"
    "#elif defined(COMPILED_PERFECT_HASH_EXE_BUILD)\n"
    "#define CPHAPI __declspec(dllimport)\n"
    "#else\n"
    "#define CPHAPI\n"
    "#endif\n"
    "\n"
    "#ifdef _M_X64\n"
    "\n"
    "#define RotateRight32 _rotr\n"
    "#define RotateLeft32  _rotl\n"
    "#define RotateRight64 _rotr64\n"
    "#define RotateLeft64  _rotl64\n"
    "#define ExtractBits64 _pext_u64\n"
    "#define Crc32u32      _mm_crc32_u32\n"
    "\n"
    "//\n"
    "// Define start/end markers for IACA.\n"
    "//\n"
    "\n"
    "#define IACA_VC_START() __writegsbyte(111, 111)\n"
    "#define IACA_VC_END()   __writegsbyte(222, 222)\n"
    "\n"
    "#else\n"
    "#define IACA_VC_START()\n"
    "#define IACA_VC_END()\n"
    "#endif\n"
    "\n"
    "#ifdef __APPLE__\n"
    "#ifdef __arm64__\n"
    "#include <time.h>\n"
    "#define __rdtsc() clock_gettime_nsec_np(CLOCK_UPTIME_RAW)\n"
    "#endif\n"
    "#endif\n"
    "//\n"
    "// Define the main functions exposed by a compiled perfect hash table: index,\n"
    "// lookup, insert and delete.\n"
    "//\n"
    "\n"
    "typedef\n"
    "CPHAPI\n"
    "CPHINDEX\n"
    "(CPHCALLTYPE COMPILED_PERFECT_HASH_TABLE_INDEX)(\n"
    "    _In_ CPHKEY Key\n"
    "    );\n"
    "/*++\n"
    "\n"
    "Routine Description:\n"
    "\n"
    "    Looks up given key in a compiled perfect hash table and returns its index.\n"
    "\n"
    "    N.B. If the given key did not appear in the original set from which the\n"
    "         hash table was created, the behavior of this routine is undefined.\n"
    "         (In practice, the key will hash to either an existing key's location\n"
    "         or an empty slot, so there is potential for returning a non-unique\n"
    "         index.)\n"
    "\n"
    "Arguments:\n"
    "\n"
    "    Key - Supplies the key to look up.\n"
    "\n"
    "Return Value:\n"
    "\n"
    "    The index associated with the given key.\n"
    "\n"
    "--*/\n"
    "typedef COMPILED_PERFECT_HASH_TABLE_INDEX *PCOMPILED_PERFECT_HASH_TABLE_INDEX;\n"
    "\n"
    "#ifndef CPH_INDEX_ONLY\n"
    "\n"
    "typedef\n"
    "CPHAPI\n"
    "CPHVALUE\n"
    "(CPHCALLTYPE COMPILED_PERFECT_HASH_TABLE_LOOKUP)(\n"
    "    _In_ CPHKEY Key\n"
    "    );\n"
    "/*++\n"
    "\n"
    "Routine Description:\n"
    "\n"
    "    Looks up given key in a compiled perfect hash table and returns the value\n"
    "    present.  If no insertion has taken place for this key, this routine\n"
    "    guarantees to return 0 as the value.\n"
    "\n"
    "    N.B. If the given key did not appear in the original set from which the\n"
    "         hash table was created, the behavior of this routine is undefined.\n"
    "         (In practice, the value returned will be the value for some other\n"
    "         key in the table that hashes to the same location, or, potentially,\n"
    "         an empty slot in the table.)\n"
    "\n"
    "Arguments:\n"
    "\n"
    "    Key - Supplies the key to look up.\n"
    "\n"
    "Return Value:\n"
    "\n"
    "    The value at the given location.\n"
    "\n"
    "--*/\n"
    "typedef COMPILED_PERFECT_HASH_TABLE_LOOKUP *PCOMPILED_PERFECT_HASH_TABLE_LOOKUP;\n"
    "\n"
    "\n"
    "typedef\n"
    "CPHAPI\n"
    "CPHVALUE\n"
    "(CPHCALLTYPE COMPILED_PERFECT_HASH_TABLE_INSERT)(\n"
    "    _In_ CPHKEY Key,\n"
    "    _In_ CPHVALUE Value\n"
    "    );\n"
    "/*++\n"
    "\n"
    "Routine Description:\n"
    "\n"
    "    Inserts value at key into a compiled hash table, and returns the previously\n"
    "    set value (which will be 0 if no prior insert occurred).\n"
    "\n"
    "    N.B. If the given key did not appear in the original set from which the\n"
    "         hash table was created, the behavior of this routine is undefined.\n"
    "         (In practice, the key will hash to either an existing key's location\n"
    "         or an empty slot, so there is potential to corrupt the table in the\n"
    "         sense that previously inserted values will be trampled over.)\n"
    "\n"
    "Arguments:\n"
    "\n"
    "    Key - Supplies the key for which the value will be inserted.\n"
    "\n"
    "    Value - Supplies the value to insert.\n"
    "\n"
    "Return Value:\n"
    "\n"
    "    Previous value at the relevant table location prior to this insertion.\n"
    "\n"
    "--*/\n"
    "typedef COMPILED_PERFECT_HASH_TABLE_INSERT\n"
    "      *PCOMPILED_PERFECT_HASH_TABLE_INSERT;\n"
    "\n"
    "\n"
    "typedef\n"
    "CPHAPI\n"
    "CPHVALUE\n"
    "(CPHCALLTYPE COMPILED_PERFECT_HASH_TABLE_DELETE)(\n"
    "    _In_ CPHKEY Key\n"
    "    );\n"
    "/*++\n"
    "\n"
    "Routine Description:\n"
    "\n"
    "    Deletes a key from a perfect hash table, optionally returning the value\n"
    "    prior to deletion back to the caller.  Deletion simply clears the value\n"
    "    associated with the key, and thus, is a simple O(1) operation.  Deleting\n"
    "    a key that has not yet been inserted has no effect other than potentially\n"
    "    returning 0 as the previous value.  That is, a caller can safely issue\n"
    "    deletes of keys regardless of whether or not said keys were inserted first.\n"
    "\n"
    "    N.B. If the given key did not appear in the original set from which the\n"
    "         hash table was created, the behavior of this routine is undefined.\n"
    "         (In practice, the key will hash to either an existing key's location\n"
    "         or an empty slot, so there is potential to corrupt the table in the\n"
    "         sense that a previously inserted value for an unrelated, valid key\n"
    "         will be cleared.)\n"
    "\n"
    "Arguments:\n"
    "\n"
    "    Key - Supplies the key to delete.\n"
    "\n"
    "Return Value:\n"
    "\n"
    "    Previous value at the given key's location prior to deletion.  If no prior\n"
    "    insertion, the previous value is guaranteed to be 0.\n"
    "\n"
    "--*/\n"
    "typedef COMPILED_PERFECT_HASH_TABLE_DELETE *PCOMPILED_PERFECT_HASH_TABLE_DELETE;\n"
    "\n"
    "typedef\n"
    "CPHAPI\n"
    "CPHVALUE\n"
    "(CPHCALLTYPE COMPILED_PERFECT_HASH_TABLE_INTERLOCKED_INCREMENT)(\n"
    "    _In_ CPHKEY Key\n"
    "    );\n"
    "/*++\n"
    "\n"
    "Routine Description:\n"
    "\n"
    "    Increments the value associated with a key.\n"
    "\n"
    "Arguments:\n"
    "\n"
    "    Key - Supplies the key to increment.\n"
    "\n"
    "Return Value:\n"
    "\n"
    "    Previous value.\n"
    "\n"
    "--*/\n"
    "typedef COMPILED_PERFECT_HASH_TABLE_INTERLOCKED_INCREMENT\n"
    "      *PCOMPILED_PERFECT_HASH_TABLE_INTERLOCKED_INCREMENT;\n"
    "\n"
    "//\n"
    "// Typedefs of methods for testing and benchmarking.\n"
    "//\n"
    "\n"
    "typedef\n"
    "_Success_(return == 0)\n"
    "ULONG\n"
    "(CPHCALLTYPE TEST_COMPILED_PERFECT_HASH_TABLE)(\n"
    "    _In_opt_ BOOLEAN DebugBreakOnFailure\n"
    "    );\n"
    "/*++\n"
    "\n"
    "Routine Description:\n"
    "\n"
    "    Tests a compiled perfect hash table for correctness.\n"
    "\n"
    "Arguments:\n"
    "\n"
    "    DebugBreakOnFailure - Supplies a boolean flag that indicates whether or\n"
    "        not a __debugbreak() should be issued as soon as a test fails.\n"
    "\n"
    "Return Value:\n"
    "\n"
    "    The number of failed tests.  If 0, all tests passed.\n"
    "\n"
    "--*/\n"
    "typedef TEST_COMPILED_PERFECT_HASH_TABLE *PTEST_COMPILED_PERFECT_HASH_TABLE;\n"
    "\n"
    "typedef\n"
    "ULONG\n"
    "(CPHCALLTYPE BENCHMARK_FULL_COMPILED_PERFECT_HASH_TABLE)(\n"
    "    _In_ ULONG Seconds\n"
    "    );\n"
    "/*++\n"
    "\n"
    "Routine Description:\n"
    "\n"
    "    Benchmarks the time it takes to insert all keys into a table, lookup the\n"
    "    inserted value, then delete all keys.\n"
    "\n"
    "Arguments:\n"
    "\n"
    "    Seconds - TBD.\n"
    "\n"
    "Return Value:\n"
    "\n"
    "    Number of cycles.\n"
    "\n"
    "--*/\n"
    "typedef BENCHMARK_FULL_COMPILED_PERFECT_HASH_TABLE\n"
    "      *PBENCHMARK_FULL_COMPILED_PERFECT_HASH_TABLE;\n"
    "\n"
    "#endif // #ifndef CPH_INDEX_ONLY\n"
    "\n"
    "typedef\n"
    "ULONG\n"
    "(CPHCALLTYPE BENCHMARK_INDEX_COMPILED_PERFECT_HASH_TABLE)(\n"
    "    _In_ ULONG Seconds\n"
    "    );\n"
    "/*++\n"
    "\n"
    "Routine Description:\n"
    "\n"
    "    Benchmarks the time it takes to perform just the Index() operation.\n"
    "\n"
    "Arguments:\n"
    "\n"
    "    Seconds - TBD.\n"
    "\n"
    "Return Value:\n"
    "\n"
    "    Number of cycles.\n"
    "\n"
    "--*/\n"
    "typedef BENCHMARK_INDEX_COMPILED_PERFECT_HASH_TABLE\n"
    "      *PBENCHMARK_INDEX_COMPILED_PERFECT_HASH_TABLE;\n"
    "\n"
    "#ifdef __cplusplus\n"
    "} // extern \"C\"\n"
    "#endif\n"
    "\n"
    "#endif // _COMPILED_PERFECT_HASH_H_\n"
    "\n"
    "// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :\n"
    "\n"
    "//\n"
    "// End CompiledPerfectHash.h.\n"
    "//\n"
    "\n"
;

const STRING CompiledPerfectHashCHeaderRawCString = {
    sizeof(CompiledPerfectHashCHeaderRawCStr) - sizeof(CHAR),
    sizeof(CompiledPerfectHashCHeaderRawCStr),
#ifdef _WIN64
    0,
#endif
    (PCHAR)&CompiledPerfectHashCHeaderRawCStr,
};

#ifndef RawCString
#define RawCString (&CompiledPerfectHashCHeaderRawCString)
#endif
