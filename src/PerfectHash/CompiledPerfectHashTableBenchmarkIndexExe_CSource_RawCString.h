//
// Auto-generated.
//

DECLSPEC_ALIGN(16)
const CHAR CompiledPerfectHashTableBenchmarkIndexExeCSourceRawCStr[] =
    "\n"
    "//\n"
    "// Begin CompiledPerfectHashTableBenchmarkIndexExe.c.\n"
    "//\n"
    "\n"
    "#pragma optimize(\"\", off)\n"
    "\n"
    "void\n"
    "__stdcall\n"
    "mainCRTStartup(\n"
    "    void\n"
    "    )\n"
    "{\n"
    "    ULONG Cycles;\n"
    "    ULONG Seconds = 0;\n"
    "\n"
    "    Cycles = BENCHMARK_INDEX_CPH_ROUTINE(Seconds);\n"
    "\n"
    "    ExitProcess(Cycles);\n"
    "}\n"
    "\n"
    "// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :\n"
;

const STRING CompiledPerfectHashTableBenchmarkIndexExeCSourceRawCString = {
    sizeof(CompiledPerfectHashTableBenchmarkIndexExeCSourceRawCStr) - sizeof(CHAR),
    sizeof(CompiledPerfectHashTableBenchmarkIndexExeCSourceRawCStr),
#ifdef _WIN64
    0,
#endif
    (PCHAR)&CompiledPerfectHashTableBenchmarkIndexExeCSourceRawCStr,
};

#ifndef RawCString
#define RawCString (&CompiledPerfectHashTableBenchmarkIndexExeCSourceRawCString)
#endif