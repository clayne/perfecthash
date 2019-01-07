//
// Auto-generated.
//

DECLSPEC_ALIGN(16)
const CHAR CompiledPerfectHashTableChm01IndexJenkinsAndCSourceRawCStr[] =
    "\n"
    "//\n"
    "// Begin CompiledPerfectHashTableChm01IndexJenkinsAnd.c.\n"
    "//\n"
    "\n"
    "\n"
    "DECLARE_INDEX_ROUTINE()\n"
    "{\n"
    "    ULONG A;\n"
    "    ULONG B;\n"
    "    ULONG C;\n"
    "    ULONG D;\n"
    "    ULONG E;\n"
    "    ULONG F;\n"
    "    PBYTE Byte;\n"
    "    ULONG Index;\n"
    "    ULONG Vertex1;\n"
    "    ULONG Vertex2;\n"
    "    ULONG MaskedLow;\n"
    "    ULONG MaskedHigh;\n"
    "    ULONGLONG Combined;\n"
    "\n"
    "    Byte = (PBYTE)&Key;\n"
    "\n"
    "    A = B = 0x9e3779b9;\n"
    "    C = SEED1;\n"
    "\n"
    "    A += (((ULONG)Byte[3]) << 24);\n"
    "    A += (((ULONG)Byte[2]) << 16);\n"
    "    A += (((ULONG)Byte[1]) <<  8);\n"
    "    A += ((ULONG)Byte[0]);\n"
    "\n"
    "    A -= B; A -= C; A ^= (C >> 13);\n"
    "    B -= C; B -= A; B ^= (A <<  8);\n"
    "    C -= A; C -= B; C ^= (B >> 13);\n"
    "    A -= B; A -= C; A ^= (C >> 12);\n"
    "    B -= C; B -= A; B ^= (A << 16);\n"
    "    C -= A; C -= B; C ^= (B >>  5);\n"
    "    A -= B; A -= C; A ^= (C >>  3);\n"
    "    B -= C; B -= A; B ^= (A << 10);\n"
    "    C -= A; C -= B; C ^= (B >> 15);\n"
    "\n"
    "    Vertex1 = C;\n"
    "\n"
    "    D = E = 0x9e3779b9;\n"
    "    F = SEED2;\n"
    "\n"
    "    D += (((ULONG)Byte[3]) << 24);\n"
    "    D += (((ULONG)Byte[2]) << 16);\n"
    "    D += (((ULONG)Byte[1]) <<  8);\n"
    "    D += ((ULONG)Byte[0]);\n"
    "\n"
    "    D -= E; D -= F; D ^= (F >> 13);\n"
    "    E -= F; E -= D; E ^= (D <<  8);\n"
    "    F -= D; F -= E; F ^= (E >> 13);\n"
    "    D -= E; D -= F; D ^= (F >> 12);\n"
    "    E -= F; E -= D; E ^= (D << 16);\n"
    "    F -= D; F -= E; F ^= (E >>  5);\n"
    "    D -= E; D -= F; D ^= (F >>  3);\n"
    "    E -= F; E -= D; E ^= (D << 10);\n"
    "    F -= D; F -= E; F ^= (E >> 15);\n"
    "\n"
    "    Vertex2 = F;\n"
    "\n"
    "    MaskedLow = Vertex1 & HASH_MASK;\n"
    "    MaskedHigh = Vertex2 & HASH_MASK;\n"
    "\n"
    "    Vertex1 = TABLE_DATA[MaskedLow];\n"
    "    Vertex2 = TABLE_DATA[MaskedHigh];\n"
    "\n"
    "    Combined = (ULONGLONG)Vertex1 + (ULONGLONG)Vertex2;\n"
    "\n"
    "    Index = Combined & INDEX_MASK;\n"
    "\n"
    "    return Index;\n"
    "}\n"
    "\n"
    "\n"
    "//\n"
    "// End CompiledPerfectHashTableChm01IndexJenkinsAnd.c.\n"
    "//\n"
    "\n"
;

const STRING CompiledPerfectHashTableChm01IndexJenkinsAndCSourceRawCString = {
    sizeof(CompiledPerfectHashTableChm01IndexJenkinsAndCSourceRawCStr) - sizeof(CHAR),
    sizeof(CompiledPerfectHashTableChm01IndexJenkinsAndCSourceRawCStr),
#ifdef _WIN64
    0,
#endif
    (PCHAR)&CompiledPerfectHashTableChm01IndexJenkinsAndCSourceRawCStr,
};

#ifndef RawCString
#define RawCString (&CompiledPerfectHashTableChm01IndexJenkinsAndCSourceRawCString)
#endif
