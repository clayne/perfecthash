//
// Auto-generated.
//

DECLSPEC_ALIGN(16)
const CHAR PerfectHashBulkCreateExeUsageTextRawCStr[] =
    "Usage: PerfectHashBulkCreate.exe <KeysDirectory> <OutputDirectory> <Algorithm>\n"
    "    <HashFunction> <MaskFunction> <MaximumConcurrency>\n"
    "    [BulkCreateFlags] [KeysLoadFlags] [TableCreateFlags]\n"
    "    [TableCompileFlags] [TableCreateParameters]\n"
    "\n"
    "Bulk Create Flags:\n"
    "\n"
    "    N/A\n"
    "\n"
    "Keys Load Flags:\n"
    "\n"
    "    --TryLargePagesForKeysData\n"
    "\n"
    "        Tries to allocate the keys buffer using large pages.\n"
    "\n"
    "Table Create Flags:\n"
    "\n"
    "    --FirstGraphWins | --FindBestGraph\n"
    "\n"
    "        --FirstGraphWins [default]\n"
    "\n"
    "            This is the default behavior.  When searching for solutions in\n"
    "            parallel, the first graph to be found, \"wins\".  i.e. it's the\n"
    "            solution that is subsequently written to disk.\n"
    "\n"
    "        --FindBestGraph\n"
    "\n"
    "            Requires the following two table create parameters to be present:\n"
    "\n"
    "                --BestCoverageNumAttempts=N\n"
    "                --BestCoverageType=<CoverageType>\n"
    "\n"
    "            The table create routine will then run until it finds the number of\n"
    "            best coverage attempts specified.  At that point, the graph that was\n"
    "            found to be the \"best\" based on the coverage type predicate \"wins\",\n"
    "            and is subsequently saved to disk.\n"
    "\n"
    "            N.B. This option is significantly more CPU intensive than the\n"
    "                 --FirstGraphWins mode, however, it is highly probable that it\n"
    "                 will find a graph that is better (based on the predicate) than\n"
    "                 when in first graph wins mode.\n"
    "\n"
    "    --SkipGraphVerification\n"
    "\n"
    "        When set, skips the internal graph verification check that ensures a\n"
    "        valid perfect hash solution has been found (i.e. with no collisions\n"
    "        across the entire key set).\n"
    "\n"
    "Table Compile Flags:\n"
    "\n"
    "    N/A\n"
    "\n"
    "Table Create Parameters:\n"
    "\n"
    "    --AttemptsBeforeTableResize=N [default = 18]\n"
    "\n"
    "        Specifies the number of attempts at solving the graph that will be made\n"
    "        before a table resize event will occur (assuming that resize events are\n"
    "        permitted, as per the following flag).\n"
    "\n"
    "    --MaxNumberOfTableResizes=N [default = 5]\n"
    "\n"
    "        Maximum number of table resizes that will be permitted before giving up.\n"
    "\n"
    "    --BestCoverageNumAttempts=N\n"
    "\n"
    "        Where N is a positive integer, and represents the number of\n"
    "        attempts that will be made at finding a \"best\" graph (based\n"
    "        on the best coverage type requested below) before the create\n"
    "        table routine returns.\n"
    "\n"
    "    --BestCoverageType=<CoverageType>\n"
    "\n"
    "        Indicates the predicate to determine what constitutes the best\n"
    "        graph.\n"
    "\n"
    "        Valid coverage types:\n"
    "\n"
    "            HighestNumberOfEmptyCacheLines\n"
;

const STRING PerfectHashBulkCreateExeUsageTextRawCString = {
    sizeof(PerfectHashBulkCreateExeUsageTextRawCStr) - sizeof(CHAR),
    sizeof(PerfectHashBulkCreateExeUsageTextRawCStr),
#ifdef _WIN64
    0,
#endif
    (PCHAR)&PerfectHashBulkCreateExeUsageTextRawCStr,
};

#ifndef RawCString
#define RawCString (&PerfectHashBulkCreateExeUsageTextRawCString)
#endif