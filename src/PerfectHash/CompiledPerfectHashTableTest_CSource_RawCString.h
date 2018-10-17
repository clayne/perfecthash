//
// Auto-generated.
//

DECLSPEC_ALIGN(16)
const CHAR CompiledPerfectHashTableTestCSourceRawCStr[] =
    "\n"
    "//\n"
    "// Begin CompiledPerfectHashTableTest.c.\n"
    "//\n"
    "\n"
    "//\n"
    "// Disable global optimizations, even in release builds.  Without this, the\n"
    "// compiler does clever things with regards to optimizing our __debugbreak()\n"
    "// logic below, such that it's impossible to tell which ASSERT() triggered it.\n"
    "//\n"
    "\n"
    "#pragma optimize(\"\", off)\n"
    "\n"
    "//\n"
    "// Use a custom ASSERT() macro that optionally issues a __debugbreak() based\n"
    "// on the caller's DebugBreakOnFailure boolean flag.\n"
    "//\n"
    "\n"
    "#undef ASSERT\n"
    "#define ASSERT(Condition)          \\\n"
    "    if (!(Condition)) {            \\\n"
    "        if (DebugBreakOnFailure) { \\\n"
    "            __debugbreak();        \\\n"
    "        };                         \\\n"
    "        NumberOfErrors++;          \\\n"
    "    }\n"
    "\n"
    "DECLARE_TEST_CPH_ROUTINE()\n"
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
    "{\n"
    "    ULONG Key;\n"
    "    ULONG Index;\n"
    "    ULONG Value;\n"
    "    ULONG Rotated;\n"
    "    ULONG Previous;\n"
    "    ULONG NumberOfErrors = 0;\n"
    "    const ULONG *Source;\n"
    "\n"
    "    Key = *KEYS;\n"
    "\n"
    "    //\n"
    "    // Rotate the key such that it differs from the original value, but in a\n"
    "    // way that can be easily reversed.\n"
    "    //\n"
    "\n"
    "    Rotated = _rotl(Key, 15);\n"
    "    ASSERT(Key == _rotr(Rotated, 15));\n"
    "\n"
    "    //\n"
    "    // Verify looking up a key that hasn't been inserted returns 0 as the value.\n"
    "    //\n"
    "\n"
    "    Value = LOOKUP_ROUTINE(Key);\n"
    "    ASSERT(Value == 0);\n"
    "\n"
    "    //\n"
    "    // Verify insertion.\n"
    "    //\n"
    "\n"
    "    Previous = INSERT_ROUTINE(Key, Rotated);\n"
    "    ASSERT(Previous == 0);\n"
    "\n"
    "    //\n"
    "    // Lookup the inserted key.\n"
    "    //\n"
    "\n"
    "    Value = LOOKUP_ROUTINE(Key);\n"
    "    ASSERT(Value == Rotated);\n"
    "\n"
    "    //\n"
    "    // Delete the inserted key.  Returned value should be Rotated.\n"
    "    //\n"
    "\n"
    "    Value = DELETE_ROUTINE(Key);\n"
    "    ASSERT(Value == Rotated);\n"
    "\n"
    "    //\n"
    "    // Verify a subsequent lookup returns 0.\n"
    "    //\n"
    "\n"
    "    Value = LOOKUP_ROUTINE(Key);\n"
    "    ASSERT(Value == 0);\n"
    "\n"
    "    //\n"
    "    // Loop through the entire key set and insert a rotated version of the key.\n"
    "    //\n"
    "\n"
    "    FOR_EACH_KEY {\n"
    "\n"
    "        Key = *Source++;\n"
    "        Rotated = _rotl(Key, 15);\n"
    "\n"
    "        Previous = INSERT_ROUTINE(Key, Rotated);\n"
    "        ASSERT(Previous == 0);\n"
    "\n"
    "    }\n"
    "\n"
    "    //\n"
    "    // Loop through the entire set again and ensure that lookup returns the\n"
    "    // rotated version.\n"
    "    //\n"
    "\n"
    "    FOR_EACH_KEY {\n"
    "\n"
    "        Key = *Source++;\n"
    "        Rotated = _rotl(Key, 15);\n"
    "\n"
    "        Value = LOOKUP_ROUTINE(Key);\n"
    "        ASSERT(Value == Rotated);\n"
    "\n"
    "    }\n"
    "\n"
    "    //\n"
    "    // Loop through again and delete everything.\n"
    "    //\n"
    "\n"
    "    FOR_EACH_KEY {\n"
    "\n"
    "        Key = *Source++;\n"
    "        Rotated = _rotl(Key, 15);\n"
    "\n"
    "        Previous = DELETE_ROUTINE(Key);\n"
    "        ASSERT(Previous == Rotated);\n"
    "\n"
    "    }\n"
    "\n"
    "    //\n"
    "    // And a final loop through to confirm all lookups now return 0.\n"
    "    //\n"
    "\n"
    "    FOR_EACH_KEY {\n"
    "\n"
    "        Key = *Source++;\n"
    "\n"
    "        Value = LOOKUP_ROUTINE(Key);\n"
    "        ASSERT(Value == 0);\n"
    "\n"
    "    }\n"
    "\n"
    "    //\n"
    "    // We're finished!  Return the number of errors.\n"
    "    //\n"
    "\n"
    "    return NumberOfErrors;\n"
    "}\n"
    "\n"
    "//\n"
    "// End CompiledPerfectHashTableTest.c.\n"
    "//\n"
;

const STRING CompiledPerfectHashTableTestCSourceRawCString = {
    sizeof(CompiledPerfectHashTableTestCSourceRawCStr) - sizeof(CHAR),
    sizeof(CompiledPerfectHashTableTestCSourceRawCStr),
#ifdef _WIN64
    0,
#endif
    (PCHAR)&CompiledPerfectHashTableTestCSourceRawCStr,
};

#ifndef RawCString
#define RawCString (&CompiledPerfectHashTableTestCSourceRawCString)
#endif
