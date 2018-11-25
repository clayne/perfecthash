/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    Graph.c

Abstract:

    This module implements generic graph functionality.

--*/

#include "stdafx.h"

#define IsEmpty(Value) ((ULONG)Value == EMPTY)
#define IsNeighborEmpty(Neighbor) ((ULONG)Neighbor == EMPTY)

//
// COM scaffolding routines for initialization and rundown.
//

GRAPH_INITIALIZE GraphInitialize;

_Use_decl_annotations_
HRESULT
GraphInitialize(
    PGRAPH Graph
    )
/*++

Routine Description:

    Initializes a graph structure.  This is a relatively simple method that
    just primes the COM scaffolding.

Arguments:

    Graph - Supplies a pointer to a GRAPH structure for which initialization
        is to be performed.

Return Value:

    S_OK - Success.

    E_POINTER - Graph is NULL.

    E_UNEXPECTED - All other errors.

--*/
{
    HRESULT Result = S_OK;

    if (!ARGUMENT_PRESENT(Graph)) {
        return E_POINTER;
    }

    Graph->SizeOfStruct = sizeof(*Graph);

    //
    // Create Rtl and Allocator components.
    //

    Result = Graph->Vtbl->CreateInstance(Graph,
                                         NULL,
                                         &IID_PERFECT_HASH_RTL,
                                         &Graph->Rtl);

    if (FAILED(Result)) {
        goto Error;
    }

    Result = Graph->Vtbl->CreateInstance(Graph,
                                         NULL,
                                         &IID_PERFECT_HASH_ALLOCATOR,
                                         &Graph->Allocator);

    if (FAILED(Result)) {
        goto Error;
    }

    //
    // We're done!  Indicate success and finish up.
    //

    Result = S_OK;
    goto End;

Error:

    if (Result == S_OK) {
        Result = E_UNEXPECTED;
    }

    //
    // Intentional follow-on to End.
    //

End:

    return Result;
}


GRAPH_RUNDOWN GraphRundown;

_Use_decl_annotations_
VOID
GraphRundown(
    PGRAPH Graph
    )
/*++

Routine Description:

    Release all resources associated with a graph.

Arguments:

    Graph - Supplies a pointer to a GRAPH structure for which rundown is to
        be performed.

Return Value:

    None.

--*/
{
    //
    // Sanity check structure size.
    //

    ASSERT(Graph->SizeOfStruct == sizeof(*Graph));

    //
    // Release applicable COM references.
    //

    RELEASE(Graph->Rtl);
    RELEASE(Graph->Allocator);

    return;
}

//
// Implement main vtbl routines.
//

GRAPH_SOLVE GraphSolve;

_Use_decl_annotations_
HRESULT
GraphSolve(
    PGRAPH Graph,
    PGRAPH *NewGraphPointer
    )
/*++

Routine Description:

    Add all keys to the hypergraph using the unique seeds to hash each key into
    two vertex values, connected by a "hyper-edge".  Determine if the graph is
    acyclic, if it is, we've "solved" the graph.  If not, we haven't.

Arguments:

    Graph - Supplies a pointer to the graph to be solved.

    NewGraphPointer - Supplies the address of a variable which will receive the
        address of a new graph instance to be used for solving if the routine
        returns PH_S_USE_NEW_GRAPH_FOR_SOLVING.

Return Value:

    PH_S_STOP_GRAPH_SOLVING - Stop graph solving.

    PH_S_CONTINUE_GRAPH_SOLVING - Continue graph solving.

    PH_S_USE_NEW_GRAPH_FOR_SOLVING - Continue graph solving but use the graph
        returned via the NewGraphPointer parameter.

--*/
{
    KEY Key;
    PKEY Keys;
    EDGE Edge;
    PEDGE Edges;
    VERTEX Vertex1;
    VERTEX Vertex2;
    PGRAPH_INFO Info;
    ULONG Iterations;
    ULONG NumberOfKeys;
    ULARGE_INTEGER Hash;
    HRESULT Result = S_OK;
    LONGLONG FinishedCount;
    PPERFECT_HASH_TABLE Table;
    PPERFECT_HASH_CONTEXT Context;
    const ULONG CheckForTerminationAfterIterations = 1024;

    Info = Graph->Info;
    Context = Info->Context;
    Table = Context->Table;
    NumberOfKeys = Table->Keys->NumberOfElements.LowPart;
    Edges = Keys = (PKEY)Table->Keys->File->BaseAddress;

    //
    // Enumerate all keys in the input set, hash them into two unique vertices,
    // then add them to the hypergraph.
    //

    Iterations = CheckForTerminationAfterIterations;

    for (Edge = 0; Edge < NumberOfKeys; Edge++) {
        Key = *Edges++;

        SEEDED_HASH(Key, &Hash.QuadPart);

        ASSERT(Hash.HighPart != Hash.LowPart);

        //
        // Mask the individual vertices.
        //

        MASK_HASH(Hash.LowPart, &Vertex1);
        MASK_HASH(Hash.HighPart, &Vertex2);

        //
        // We can't have two vertices point to the same location.
        // Abort this graph attempt.
        //

        if (Vertex1 == Vertex2) {
            goto Failed;
        }

        //
        // Add the edge to the graph connecting these two vertices.
        //

        GraphAddEdge(Graph, Edge, Vertex1, Vertex2);

        //
        // Every 1024 iterations, check whether the context is indicating to
        // stop solving, either because a solution has been found and we're in
        // "first graph wins" mode, or the explicit state flag to stop solving
        // has been set.
        //

        if (!--Iterations) {

            if (FirstSolvedGraphWins(Context)) {
                if (Context->FinishedCount > 0) {
                    return PH_S_STOP_GRAPH_SOLVING;
                }
            } else if (StopSolving(Context)) {
                return PH_S_STOP_GRAPH_SOLVING;
            }

            //
            // Reset the iteration counter.
            //

            Iterations = CheckForTerminationAfterIterations;

        }
    }

    //
    // We've added all of the vertices to the graph.  Determine if the graph
    // is acyclic.
    //

    if (!IsGraphAcyclic(Graph)) {

        //
        // Failed to create an acyclic graph.
        //

        goto Failed;
    }

    //
    // We created an acyclic graph.
    //

    //
    // Increment the finished count.  If the context indicates "first solved
    // graph wins", and the value is 1, we're the winning thread, so continue
    // with graph assignment.  Otherwise, just return with the stop graph
    // solving code and let the other thread finish up (i.e. perform the
    // assignment step and then persist the result).
    //
    // If the context does not indicate "first solved graph wins", perform
    // the assignment step regardless.
    //

    FinishedCount = InterlockedIncrement64(&Context->FinishedCount);

    if (FirstSolvedGraphWins(Context)) {

        if (FinishedCount != 1) {

            //
            // Some other thread beat us.  Nothing left to do.
            //

            return PH_S_STOP_GRAPH_SOLVING;
        }
    }

    //
    // Perform the assignment step.
    //

    GraphAssign(Graph);

    //
    // Stop the solve timers here if we're in "first graph wins" mode.
    //

    if (FirstSolvedGraphWins(Context)) {
        CONTEXT_END_TIMERS(Solve);
    }

    //
    // If we're in "first graph wins" mode and we reach this point, we're the
    // winning thread, so, push the graph onto the finished list head, then
    // submit the relevant finished threadpool work item and return stop graph
    // solving.
    //

    if (FirstSolvedGraphWins(Context)) {
        if (WantsAssignedMemoryCoverage(Graph)) {
            Graph->Vtbl->CalculateAssignedMemoryCoverage(Graph);
            CopyCoverage(Context->Table->Coverage,
                         &Graph->AssignedMemoryCoverage);
        }
        InsertHeadFinishedWork(Context, &Graph->ListEntry);
        SubmitThreadpoolWork(Context->FinishedWork);
        return PH_S_STOP_GRAPH_SOLVING;
    }

    //
    // If we reach this mode, we're in "find best memory coverage" mode, so,
    // register the solved graph then continue solving.
    //

    ASSERT(FindBestMemoryCoverage(Context));

    //
    // Calculate memory coverage information if applicable.
    //

    if (WantsAssignedMemoryCoverage(Graph)) {
        Graph->Vtbl->CalculateAssignedMemoryCoverage(Graph);
    } else if (WantsAssignedMemoryCoverageForKeysSubset(Graph)) {
        Graph->Vtbl->CalculateAssignedMemoryCoverageForKeysSubset(Graph);
    }

    //
    // Register the solved graph.  We can return this result directly.
    //

    Result = Graph->Vtbl->RegisterSolved(Graph, NewGraphPointer);
    return Result;

Failed:

    //
    // Increment the failed attempts counter.
    //

    InterlockedIncrement64(&Context->FailedAttempts);

    //
    // Intentional follow-on to Error.
    //

Error:

    //
    // If any of the HASH/MASK macros fail, they'll jump to this Error: label.
    //

    return PH_S_CONTINUE_GRAPH_SOLVING;
}


GRAPH_VERIFY GraphVerify;

_Use_decl_annotations_
HRESULT
GraphVerify(
    PGRAPH Graph
    )
/*++

Routine Description:

    Verify a solved graph is working correctly.  This walks through the entire
    original key set, captures the index that is returned when the key is hashed
    (i.e. simulates the Index() method), sets a bit in a bitmap for each index,
    verifying that we never see the same index twice, as this would indicate a
    collision, and then finally, verify that the number of set bits in the
    bitmap exactly equals the number of keys we saw.

    N.B. The original chm.c-style modulus-oriented solution fails to pass this
         step, which actually matches the experience I had with it during my
         initial evaluation.

Arguments:

    Graph - Supplies a pointer to the graph to be verified.

Return Value:

    S_OK - Graph was solved successfully.

    PH_S_GRAPH_VERIFICATION_SKIPPED - The verification step was skipped.

    E_POINTER - Graph was NULL.

    E_OUTOFMEMORY - Out of memory.

    E_UNEXPECTED - Internal error.

    PH_E_COLLISIONS_ENCOUNTERED_DURING_GRAPH_VERIFICATION - Collisions were
        detected during graph validation.

    PH_E_NUM_ASSIGNMENTS_NOT_EQUAL_TO_NUM_KEYS_DURING_GRAPH_VERIFICATION -
        The number of value assignments did not equal the number of keys
        during graph validation.

--*/
{
    PRTL Rtl;
    KEY Key;
    KEY PreviousKey;
    PKEY Keys;
    EDGE Edge;
    PEDGE Edges;
    ULONG Bit;
    ULONG Index;
    ULONG PrevIndex;
    PULONG Values = NULL;
    VERTEX Vertex1;
    VERTEX Vertex2;
    VERTEX MaskedLow;
    VERTEX MaskedHigh;
    VERTEX PrevVertex1;
    VERTEX PrevVertex2;
    VERTEX PrevMaskedLow;
    VERTEX PrevMaskedHigh;
    PVERTEX Assigned;
    PGRAPH_INFO Info;
    ULONG NumberOfKeys;
    ULONG NumberOfAssignments;
    ULONG Collisions = 0;
    LONGLONG Combined;
    LONGLONG PrevCombined;
    ULARGE_INTEGER Hash;
    ULARGE_INTEGER PrevHash;
    HRESULT Result = S_OK;
    PALLOCATOR Allocator;
    PPERFECT_HASH_TABLE Table;
    PPERFECT_HASH_CONTEXT Context;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Graph)) {
        return E_POINTER;
    }

    if (SkipGraphVerification(Graph)) {
        return PH_S_GRAPH_VERIFICATION_SKIPPED;
    }

    //
    // Initialize aliases.
    //

    Info = Graph->Info;
    Context = Info->Context;
    Rtl = Context->Rtl;
    Table = Context->Table;
    Allocator = Graph->Allocator;
    NumberOfKeys = Graph->NumberOfKeys;
    Edges = Keys = (PKEY)Table->Keys->File->BaseAddress;
    Assigned = Graph->Assigned;

    //
    // Sanity check our assigned bitmap is clear.
    //

    NumberOfAssignments = Rtl->RtlNumberOfSetBits(&Graph->AssignedBitmap);
    ASSERT(NumberOfAssignments == 0);

    //
    // Allocate a values array if one is not present.
    //

    Values = Graph->Values;

    if (!Values) {
        Values = Graph->Values = (PULONG)(
            Allocator->Vtbl->Calloc(
                Allocator,
                Info->ValuesSizeInBytes,
                sizeof(*Graph->Values)
            )
        );
    }

    if (!Values) {
        return E_OUTOFMEMORY;
    }

    //
    // Enumerate all keys in the input set and verify they can be resolved
    // correctly from the assigned vertex array.
    //

    for (Edge = 0; Edge < NumberOfKeys; Edge++) {
        Key = *Edges++;

        //
        // Hash the key.
        //

        SEEDED_HASH(Key, &Hash.QuadPart);

        ASSERT(Hash.QuadPart);
        ASSERT(Hash.HighPart != Hash.LowPart);

        //
        // Mask the high and low parts of the hash.
        //

        MASK_HASH(Hash.LowPart, &MaskedLow);
        MASK_HASH(Hash.HighPart, &MaskedHigh);

        //
        // Extract the individual vertices.
        //

        Vertex1 = Assigned[MaskedLow];
        Vertex2 = Assigned[MaskedHigh];

        //
        // Mask the result.
        //

        Combined = (LONGLONG)Vertex1 + (LONGLONG)Vertex2;

        MASK_INDEX(Combined, &Index);

        Bit = Index;

        //
        // Make sure we haven't seen this bit before.
        //

        if (TestGraphBit(AssignedBitmap, Bit)) {

            //
            // We've seen this index before!  Get the key that previously
            // mapped to it.
            //

            PreviousKey = Values[Index];

            SEEDED_HASH(PreviousKey, &PrevHash.QuadPart);

            MASK_HASH(PrevHash.LowPart, &PrevMaskedLow);
            MASK_HASH(PrevHash.HighPart, &PrevMaskedHigh);

            PrevVertex1 = Assigned[MaskedLow];
            PrevVertex2 = Assigned[MaskedHigh];

            PrevCombined = (LONGLONG)PrevVertex1 + (LONGLONG)PrevVertex2;

            MASK_INDEX(PrevCombined, &PrevIndex);

            Collisions++;

        }

        //
        // Set the bit and store this key in the underlying values array.
        //

        SetGraphBit(AssignedBitmap, Bit);
        Values[Index] = Key;

    }

    if (Collisions) {
        Result = PH_E_COLLISIONS_ENCOUNTERED_DURING_GRAPH_VERIFICATION;
        goto Error;
    }

    NumberOfAssignments = Rtl->RtlNumberOfSetBits(&Graph->AssignedBitmap);

    if (NumberOfAssignments != NumberOfKeys) {
        Result =
           PH_E_NUM_ASSIGNMENTS_NOT_EQUAL_TO_NUM_KEYS_DURING_GRAPH_VERIFICATION;
        goto Error;
    }

    //
    // We're done, finish up.
    //

    goto End;

Error:

    if (Result == S_OK) {
        Result = E_UNEXPECTED;
    }

    //
    // Intentional follow-on to End.
    //

End:

    if (Graph->Values) {
        Allocator->Vtbl->FreePointer(Allocator, &Graph->Values);
    }

    return Result;
}


VOID
VerifyMemoryCoverageInvariants(
    _In_ PGRAPH Graph,
    _In_ PASSIGNED_MEMORY_COVERAGE Coverage
    );

GRAPH_CALCULATE_ASSIGNED_MEMORY_COVERAGE GraphCalculateAssignedMemoryCoverage;

_Use_decl_annotations_
VOID
GraphCalculateAssignedMemoryCoverage(
    PGRAPH Graph
    )
/*++

Routine Description:

    Calculate the memory coverage of a solved, assigned graph.  This routine
    walks the entire assigned array (see comments at the start of Graph.h for
    more info about the role of the assigned array) and calculates how many
    cache lines, pages and large pages are empty vs used.  ("Used" means one
    or more assigned values were found.)

Arguments:

    Graph - Supplies a pointer to the graph for which memory coverage of the
        assigned array is to be calculated.

Return Value:

    None.

--*/
{
    BYTE Count;
    BYTE FirstCount;
    BYTE SecondCount;
    USHORT PageCount;
    ULONG LargePageCount;
    ULONG PageIndex;
    ULONG CacheLineIndex;
    ULONG LargePageIndex;
    ULONG NumberOfCacheLines;
    ULONG TotalBytesProcessed;
    ULONG PageSizeBytesProcessed;
    ULONG LargePageSizeBytesProcessed;
    BOOLEAN FoundFirst = FALSE;
    BOOLEAN IsLastCacheLine = FALSE;
    PASSIGNED_CACHE_LINE AssignedCacheLine;
    PASSIGNED_MEMORY_COVERAGE Coverage;

#ifndef __AVX2__
    ULONG Index;
    PASSIGNED Assigned;
#else
    ULONG Mask;
    PBYTE Assigned;
    YMMWORD NotZerosYmm;
    YMMWORD AssignedYmm;
    YMMWORD ShiftedYmm;
    const YMMWORD AllZeros = _mm256_set1_epi8(0);
#endif

    Coverage = &Graph->AssignedMemoryCoverage;
    Coverage->Attempt = Graph->Attempt;
    NumberOfCacheLines = Coverage->TotalNumberOfCacheLines;
    AssignedCacheLine = (PASSIGNED_CACHE_LINE)Graph->Assigned;

    PageIndex = 0;
    LargePageIndex = 0;
    TotalBytesProcessed = 0;
    PageSizeBytesProcessed = 0;
    LargePageSizeBytesProcessed = 0;

    //
    // Enumerate the assigned array in cache-line-sized strides.
    //

    for (CacheLineIndex = 0;
         CacheLineIndex < NumberOfCacheLines;
         CacheLineIndex++) {

        Count = 0;
        IsLastCacheLine = (CacheLineIndex == NumberOfCacheLines - 1);

#ifndef __AVX2__

        //
        // For each cache line, enumerate over each individual element, and,
        // if it is not NULL, increment the local count and total count.
        //

        for (Index = 0; Index < NUM_ASSIGNED_PER_CACHE_LINE; Index++) {
            Assigned = AssignedCacheLine[Index];
            if (*Assigned) {
                Count++;
                Coverage->TotalNumberOfAssigned++;
            }
        }
#else

        //
        // An AVX2 version of the logic above.  Load 32 bytes into a YMM
        // register and compare it against a YMM register that is all zeros.
        // Shift the resulting comparison result right 24 bits, then generate
        // a ULONG mask (we need the shift because we have to use the intrinsic
        // _mm256_movemask_epi8() as there's no _mm256_movemask_epi32()).  The
        // population count of the resulting mask provides us with the number
        // of non-zero ULONG elements within that 32 byte chunk.  Update the
        // counts and then repeat for the second 32 byte chunk.
        //

        //
        // First 32 bytes of the cache line.
        //

        Assigned = (PBYTE)AssignedCacheLine;
        AssignedYmm = _mm256_stream_load_si256((PYMMWORD)Assigned);
        NotZerosYmm = _mm256_cmpgt_epi32(AssignedYmm, AllZeros);
        ShiftedYmm = _mm256_srli_epi32(NotZerosYmm, 24);
        Mask = _mm256_movemask_epi8(ShiftedYmm);
        FirstCount = (BYTE)PopulationCount32(Mask);
        ASSERT(FirstCount >= 0 && FirstCount <= 8);

        //
        // Second 32 bytes of the cache line.
        //

        Assigned += 32;
        AssignedYmm = _mm256_stream_load_si256((PYMMWORD)Assigned);
        NotZerosYmm = _mm256_cmpgt_epi32(AssignedYmm, AllZeros);
        ShiftedYmm = _mm256_srli_epi32(NotZerosYmm, 24);
        Mask = _mm256_movemask_epi8(ShiftedYmm);
        SecondCount = (BYTE)PopulationCount32(Mask);
        ASSERT(SecondCount >= 0 && SecondCount <= 8);

        Count = FirstCount + SecondCount;
        Coverage->TotalNumberOfAssigned += Count;

#endif

        ASSERT(Count >= 0 && Count <= 16);
        Coverage->NumberOfAssignedPerCacheLineCounts[Count]++;

        //
        // Advance the cache line pointer.
        //

        AssignedCacheLine++;

        //
        // Increment the empty or used counters depending on whether or not
        // any assigned elements were detected.
        //

        if (!Count) {

            Coverage->NumberOfEmptyCacheLines++;

        } else {

            Coverage->NumberOfUsedCacheLines++;

            if (!FoundFirst) {
                FoundFirst = TRUE;
                Coverage->FirstCacheLineUsed = CacheLineIndex;
                Coverage->FirstPageUsed = PageIndex;
                Coverage->FirstLargePageUsed = LargePageIndex;
                Coverage->LastCacheLineUsed = CacheLineIndex;
                Coverage->LastPageUsed = PageIndex;
                Coverage->LastLargePageUsed = LargePageIndex;
            } else {
                Coverage->LastCacheLineUsed = CacheLineIndex;
                Coverage->LastPageUsed = PageIndex;
                Coverage->LastLargePageUsed = LargePageIndex;
            }

        }

        //
        // Update histograms based on the count we just observed.
        //

        Coverage->NumberOfAssignedPerCacheLine[CacheLineIndex] = Count;
        Coverage->NumberOfAssignedPerLargePage[LargePageIndex] += Count;
        Coverage->NumberOfAssignedPerPage[PageIndex] += Count;

        TotalBytesProcessed += CACHE_LINE_SIZE;
        PageSizeBytesProcessed += CACHE_LINE_SIZE;
        LargePageSizeBytesProcessed += CACHE_LINE_SIZE;

        //
        // If we've hit a page boundary, or this is the last cache line we'll
        // be processing, finalize counts for this page.  Likewise for large
        // pages.
        //

        if (PageSizeBytesProcessed == PAGE_SIZE || IsLastCacheLine) {

            PageSizeBytesProcessed = 0;
            PageCount = Coverage->NumberOfAssignedPerPage[PageIndex];

            if (PageCount) {
                Coverage->NumberOfUsedPages++;
            } else {
                Coverage->NumberOfEmptyPages++;
            }

            PageIndex++;

            if (LargePageSizeBytesProcessed == LARGE_PAGE_SIZE ||
                IsLastCacheLine) {

                LargePageSizeBytesProcessed = 0;
                LargePageCount =
                    Coverage->NumberOfAssignedPerLargePage[LargePageIndex];

                if (LargePageCount) {
                    Coverage->NumberOfUsedLargePages++;
                } else {
                    Coverage->NumberOfEmptyLargePages++;
                }

                LargePageIndex++;
            }
        }
    }

    //
    // Enumeration of the assigned array complete; verify invariants.
    //

    VerifyMemoryCoverageInvariants(Graph, Coverage);

    return;
}


GRAPH_CALCULATE_ASSIGNED_MEMORY_COVERAGE_FOR_KEYS_SUBSET
    GraphCalculateAssignedMemoryCoverageForKeysSubset;

_Use_decl_annotations_
VOID
GraphCalculateAssignedMemoryCoverageForKeysSubset(
    PGRAPH Graph
    )
/*++

Routine Description:

    Calculate the memory coverage of a solved, assigned graph for a subset of
    keys.  This routine enumerates each key (edge) in the subset, recalculates
    the two hash values (vertices), then increments the relevant counts for
    where each vertex resides based on cache line, page and large page.  Then,
    these counts are enumerated and for those where a count greater than zero
    is observed, the coverage's NumberOfUsedPages, NumberOfUsedLargePages and
    NumberOfUsedCacheLines counts are incremented.  A histogram is also kept
    of cache line counts (NumberOfAssignedPerCacheLineCounts).

Arguments:

    Graph - Supplies a pointer to the graph for which memory coverage of the
        assigned array for a subset of keys is to be calculated.

Return Value:

    None.

--*/
{
    KEY Key;
    ULONG Index;
    ULONG Count;
    PULONG Value;
    VERTEX Vertex1;
    VERTEX Vertex2;
    ULONG_PTR Offset1;
    ULONG_PTR Offset2;
    ULONG PageIndex1;
    ULONG PageIndex2;
    ULONG LargePageIndex1;
    ULONG LargePageIndex2;
    ULONG CacheLineIndex1;
    ULONG CacheLineIndex2;
    PKEYS_SUBSET Subset;
    ULARGE_INTEGER Hash;
    HRESULT Result = S_OK;
    PPERFECT_HASH_TABLE Table;
    PPERFECT_HASH_CONTEXT Context;
    PASSIGNED_MEMORY_COVERAGE Coverage;
    PASSIGNED_PAGE_COUNT PageCount;
    PASSIGNED_LARGE_PAGE_COUNT LargePageCount;
    PASSIGNED_CACHE_LINE_COUNT CacheLineCount;

    //
    // Initialize aliases.
    //

    Context = Graph->Context;
    Table = Context->Table;
    Subset = Context->KeysSubset;
    Value = Subset->Values;
    Coverage = &Graph->AssignedMemoryCoverage;
    Coverage->Attempt = Graph->Attempt;

    //
    // Walk the key subset, hash each key, identify the assigned array location,
    // increment the relevant counters.
    //

    for (Index = 0; Index < Subset->NumberOfValues; Index++) {
        Key = *Value++;

        //
        // Generate both hashes for the key.
        //

        SEEDED_HASH(Key, &Hash.QuadPart);

        //
        // Mask the individual vertices.
        //

        MASK_HASH(Hash.LowPart, &Vertex1);
        MASK_HASH(Hash.HighPart, &Vertex2);

        //
        // Invariant check: the vertices must differ by this point (see the
        // check in GraphSolve()).
        //

        ASSERT(Vertex1 != Vertex2);

        //
        // Process the first vertex.
        //

        Offset1 = ((ULONG_PTR)Vertex1) << ASSIGNED_SHIFT;

        PageIndex1 = (ULONG)(Offset1 >> PAGE_SHIFT);
        LargePageIndex1 = (ULONG)(Offset1 >> LARGE_PAGE_SHIFT);
        CacheLineIndex1 = (ULONG)(Offset1 >> CACHE_LINE_SHIFT);

        Coverage->NumberOfAssignedPerPage[PageIndex1]++;
        Coverage->NumberOfAssignedPerLargePage[LargePageIndex1]++;
        Coverage->NumberOfAssignedPerCacheLine[CacheLineIndex1]++;

        //
        // Process the second vertex.
        //

        Offset2 = ((ULONG_PTR)Vertex2) << ASSIGNED_SHIFT;

        PageIndex2 = (ULONG)(Offset2 >> PAGE_SHIFT);
        LargePageIndex2 = (ULONG)(Offset2 >> LARGE_PAGE_SHIFT);
        CacheLineIndex2 = (ULONG)(Offset2 >> CACHE_LINE_SHIFT);

        Coverage->NumberOfAssignedPerPage[PageIndex2]++;
        Coverage->NumberOfAssignedPerLargePage[LargePageIndex2]++;
        Coverage->NumberOfAssignedPerCacheLine[CacheLineIndex2]++;

        //
        // Increment relevant counters if the same large page, page or cache
        // line is shared between the vertices.
        //

        if (LargePageIndex1 == LargePageIndex2) {
            Coverage->NumberOfKeysWithVerticesMappingToSameLargePage++;
            if (PageIndex1 == PageIndex2) {
                Coverage->NumberOfKeysWithVerticesMappingToSamePage++;
                if (CacheLineIndex1 == CacheLineIndex2) {
                    Coverage->NumberOfKeysWithVerticesMappingToSameCacheLine++;
                }
            }
        }
    }

    //
    // Sum the counts captured above.
    //

    PageCount = Coverage->NumberOfAssignedPerPage;
    for (Index = 0; Index < Coverage->TotalNumberOfPages; Index++) {
        Count = *PageCount++;
        if (Count > 0) {
            Coverage->NumberOfUsedPages++;
        }
    }

    LargePageCount = Coverage->NumberOfAssignedPerLargePage;
    for (Index = 0; Index < Coverage->TotalNumberOfLargePages; Index++) {
        Count = *LargePageCount++;
        if (Count > 0) {
            Coverage->NumberOfUsedLargePages++;
        }
    }

    CacheLineCount = Coverage->NumberOfAssignedPerCacheLine;
    for (Index = 0; Index < Coverage->TotalNumberOfCacheLines; Index++) {
        Count = *CacheLineCount++;
        if (Count > 0) {
            Coverage->NumberOfUsedCacheLines++;
        }
        Coverage->NumberOfAssignedPerCacheLineCounts[Count]++;
    }

    return;

    //
    // We need the following Error: label in order to use the SEEDED_HASH()
    // and MASK_HASH() macros above.  As we've already solved the graph at
    // this point (and thus, all generated vertices were usable), we shouldn't
    // hit this point, so, PH_RAISE() if we do.
    //

Error:

    Result = PH_E_UNREACHABLE_CODE;
    PH_ERROR(GraphCalculateAssignedMemoryCoverageForKeysSubset_Error, Result);
    PH_RAISE(Result);

}


//
// Disable optimization for this routine to prevent grouping of the PH_RAISE()
// statements (i.e. such that you can't tell exactly what line triggered the
// exception).
//
// N.B. This would be better addressed by individual error codes for each
//      invariant failure.
//

#pragma optimize("", off)
VOID
VerifyMemoryCoverageInvariants(
    _In_ PGRAPH Graph,
    _In_ PASSIGNED_MEMORY_COVERAGE Coverage
    )
{

    //
    // Invariant check: the total number of assigned elements we observed
    // should be less than or equal to the number of vertices.
    //

    if (Coverage->TotalNumberOfAssigned > Graph->NumberOfVertices) {
        PH_RAISE(PH_E_INVARIANT_CHECK_FAILED);
    }

    //
    // Invariant check: the number of used plus number of empty should equal
    // the total for each element type.
    //

    if (Coverage->NumberOfUsedPages + Coverage->NumberOfEmptyPages !=
        Coverage->TotalNumberOfPages) {
        PH_RAISE(PH_E_INVARIANT_CHECK_FAILED);
    }

    if (Coverage->NumberOfUsedLargePages + Coverage->NumberOfEmptyLargePages !=
        Coverage->TotalNumberOfLargePages) {
        PH_RAISE(PH_E_INVARIANT_CHECK_FAILED);
    }

    if (Coverage->NumberOfUsedCacheLines + Coverage->NumberOfEmptyCacheLines !=
        Coverage->TotalNumberOfCacheLines) {
        PH_RAISE(PH_E_INVARIANT_CHECK_FAILED);
    }

    //
    // Invariant check: the last used element should be greater than or equal
    // to the first used element.
    //

    if (Coverage->LastPageUsed < Coverage->FirstPageUsed) {
        PH_RAISE(PH_E_INVARIANT_CHECK_FAILED);
    }

    if (Coverage->LastLargePageUsed < Coverage->FirstLargePageUsed) {
        PH_RAISE(PH_E_INVARIANT_CHECK_FAILED);
    }

    if (Coverage->LastCacheLineUsed < Coverage->FirstCacheLineUsed) {
        PH_RAISE(PH_E_INVARIANT_CHECK_FAILED);
    }
}
#pragma optimize("", on)

GRAPH_REGISTER_SOLVED GraphRegisterSolved;

_Use_decl_annotations_
HRESULT
GraphRegisterSolved(
    PGRAPH Graph,
    PGRAPH *NewGraphPointer
    )
/*++

Routine Description:

    Attempts to register a solved graph with a context if the graph's memory
    coverage is the best that's been encountered so far.

Arguments:

    Graph - Supplies a pointer to the solved graph to register.

    NewGraphPointer - Supplies the address of a variable which will receive the
        address of a new graph instance to be used for solving if the routine
        returns PH_S_USE_NEW_GRAPH_FOR_SOLVING.

Return Value:

    PH_S_CONTINUE_GRAPH_SOLVING - Continue graph solving with the current graph.

    PH_S_USE_NEW_GRAPH_FOR_SOLVING - Continue graph solving but use the graph
        returned via the NewGraphPointer parameter.

    PH_S_GRAPH_SOLVING_STOPPED - The context indicated that graph solving was
        to stop (due to a best solution already being found, or a limit being
        hit, for example).  No graph registration is performed in this instance.

--*/
{
    HRESULT Result = PH_S_CONTINUE_GRAPH_SOLVING;
    PGRAPH SpareGraph;
    PGRAPH PreviousBestGraph;
    PPERFECT_HASH_CONTEXT Context;
    PASSIGNED_MEMORY_COVERAGE Coverage;
    PASSIGNED_MEMORY_COVERAGE PreviousBestCoverage;
    PERFECT_HASH_TABLE_BEST_COVERAGE_TYPE CoverageType;

    //
    // Initialize aliases.
    //

    Context = Graph->Context;
    Coverage = &Graph->AssignedMemoryCoverage;
    CoverageType = Context->BestCoverageType;

    //
    // Fast-path exit: if the context is indicating to stop solving, return.
    //

    if (StopSolving(Context)) {
        return PH_S_GRAPH_SOLVING_STOPPED;
    }

    //
    // Enter the best graph critical section.  Check the stop solving indicator
    // again, as it may have been set between our last check above, and when we
    // enter the critical section.
    //

    EnterCriticalSection(&Context->BestGraphCriticalSection);

    if (StopSolving(Context)) {
        Result = PH_S_GRAPH_SOLVING_STOPPED;
        goto End;
    }

    //
    // If there is no best graph currently set, proceed with setting it to
    // our current graph, then use the spare graph to continue solving.
    //

    if (!Context->BestGraph) {
        SpareGraph = Context->SpareGraph;
        ASSERT(SpareGraph != NULL);
        ASSERT(IsSpareGraph(SpareGraph));
        SpareGraph->Flags.IsSpare = FALSE;
        Context->SpareGraph = NULL;
        Context->BestGraph = Graph;
        *NewGraphPointer = SpareGraph;
        Result = PH_S_USE_NEW_GRAPH_FOR_SOLVING;
        goto End;
    }

    //
    // There's an existing best graph set.  Verify spare graph is NULL, then
    // initialize aliases to the previous best.
    //

    ASSERT(Context->SpareGraph == NULL);
    PreviousBestGraph = Context->BestGraph;
    PreviousBestCoverage = &PreviousBestGraph->AssignedMemoryCoverage;

    //
    // Determine if this graph has the "best" memory coverage and update the
    // best graph accordingly if so.
    //

#define EXPAND_AS_DETERMINE_IF_BEST_GRAPH(Name, Comparison, Comparator) \
    case BestCoverageType##Comparison##Name##Id:                        \
        if (Coverage->##Name Comparator PreviousBestCoverage->##Name) { \
            Context->BestGraph = Graph;                                 \
            *NewGraphPointer = PreviousBestGraph;                       \
            Result = PH_S_USE_NEW_GRAPH_FOR_SOLVING;                    \
        }                                                               \
        break;

    switch (CoverageType) {

        BEST_COVERAGE_TYPE_TABLE_ENTRY(EXPAND_AS_DETERMINE_IF_BEST_GRAPH)

        default:
            Result = PH_E_INVALID_BEST_COVERAGE_TYPE;
            break;
    }

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Leave the critical section and return.
    //

    LeaveCriticalSection(&Context->BestGraphCriticalSection);

    //
    // Any failure code at this point is a critical internal invariant failure.
    //

    if (FAILED(Result)) {
        PH_RAISE(Result);
    }

    return Result;
}


GRAPH_SET_INFO GraphSetInfo;

_Use_decl_annotations_
HRESULT
GraphSetInfo(
    PGRAPH Graph,
    PGRAPH_INFO Info
    )
/*++

Routine Description:

    Registers information about a graph with an individual graph instance.
    This routine is called once per unique graph info (that is, if a table
    resize event occurs it will be called again with the new graph info).
    The LoadInfo() routine will use the provided info for allocating or
    reallocating the necessary buffers required for graph solving.

    N.B. This routine is intended to be called from the "main" thread, whereas
         LoadInfo() is intended to be called as the first operation by graph
         solving worker threads.  Thus, this routine is pretty simple.

Arguments:

    Graph - Supplies a pointer to the graph instance.

    Info - Supplies a pointer to the graph info instance.

Return Value:

    S_OK - Success.

    E_POINTER - Graph or Info were NULL.

--*/
{
    if (!ARGUMENT_PRESENT(Graph)) {
        return E_POINTER;
    }

    if (!ARGUMENT_PRESENT(Info)) {
        return E_POINTER;
    }

    Graph->Info = Info;
    Graph->Flags.IsInfoSet = TRUE;

    return S_OK;
}


GRAPH_ENTER_SOLVING_LOOP GraphEnterSolvingLoop;

_Use_decl_annotations_
HRESULT
GraphEnterSolvingLoop(
    PGRAPH Graph
    )
/*++

Routine Description:

    Enters the graph solving loop.

Arguments:

    Graph - Supplies a pointer to a graph instance.

Return Value:

    S_OK - Success.

    E_POINTER - Graph was NULL.

    E_OUTOFMEMORY - Out of memory.

    Non-exhaustive list of additional errors that may be returned:

    PH_E_GRAPH_NO_INFO_SET - No graph information was set.

    PH_E_NO_MORE_SEEDS - No more seed data is available.

--*/
{
    PGRAPH NewGraph;
    HRESULT Result = S_OK;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Graph)) {
        return E_POINTER;
    }

    //
    // Acquire the exclusive graph lock for the duration of the routine.
    //

    AcquireGraphLockExclusive(Graph);

    //
    // Load the graph info.
    //

    Result = Graph->Vtbl->LoadInfo(Graph);

    if (FAILED(Result)) {

        if (Result != E_OUTOFMEMORY) {

            //
            // Anything other than an out-of-memory indication from LoadInfo()
            // indicates an internal error somewhere; log the error, then raise.
            //

            PH_ERROR(GraphLoadInfo, Result);
            PH_RAISE(PH_E_INVARIANT_CHECK_FAILED);

        }

        //
        // We failed to allocate sufficient memory for the graph.  Check for the
        // edge case where *all* threads failed to allocate memory, and set the
        // context state flag and FailedEvent accordingly.
        //

        if (InterlockedDecrement(&Graph->Context->GraphMemoryFailures) == 0) {
            Graph->Context->State.AllGraphsFailedMemoryAllocation = TRUE;
            if (!SetEvent(Graph->Context->FailedEvent)) {
                SYS_ERROR(SetEvent);
                Result = PH_E_SYSTEM_CALL_FAILED;
            }
        }

        goto End;
    }

    //
    // Begin the solving loop.
    //

    while (ShouldWeContinueTryingToSolveGraphChm01(Graph->Context)) {

        Result = Graph->Vtbl->LoadNewSeeds(Graph);
        if (FAILED(Result)) {

            //
            // N.B. This will need to be adjusted when we support the notion
            //      of no more seed data (PH_E_NO_MORE_SEEDS).
            //

            PH_ERROR(GraphLoadNewSeeds, Result);
            break;
        }

        Result = Graph->Vtbl->Reset(Graph);
        if (FAILED(Result)) {
            PH_ERROR(GraphReset, Result);
            break;
        } else if (Result != PH_S_CONTINUE_GRAPH_SOLVING) {
            break;
        }

        NewGraph = NULL;
        Result = Graph->Vtbl->Solve(Graph, &NewGraph);
        if (FAILED(Result)) {
            PH_ERROR(GraphSolve, Result);
            break;
        }

        if (Result == PH_S_STOP_GRAPH_SOLVING ||
            Result == PH_S_GRAPH_SOLVING_STOPPED) {
            ASSERT(NewGraph == NULL);
            break;
        }

        if (Result == PH_S_USE_NEW_GRAPH_FOR_SOLVING) {
            if (NewGraph == NULL) {
                PH_RAISE(PH_E_INVARIANT_CHECK_FAILED);
            }

            AcquireGraphLockExclusive(NewGraph);

            if (!IsGraphInfoLoaded(NewGraph) ||
                NewGraph->LastLoadedNumberOfVertices <
                Graph->NumberOfVertices) {

                Result = NewGraph->Vtbl->LoadInfo(NewGraph);
                if (FAILED(Result)) {
                    PH_ERROR(GraphLoadInfo_NewGraph, Result);
                    goto End;
                }
            }

            ReleaseGraphLockExclusive(Graph);

            Graph = NewGraph;
            continue;
        }

        //
        // Invariant check: result should always be PH_S_CONTINUE_GRAPH_SOLVING
        // at this point.
        //

        ASSERT(Result == PH_S_CONTINUE_GRAPH_SOLVING);

        //
        // Continue the loop and attempt another solve.
        //

    }

    //
    // Intentional follow-on to End.
    //

End:

    if (SUCCEEDED(Result)) {

        //
        // Normalize the success error codes (e.g. PH_S_STOP_GRAPH_SOLVING)
        // into a single S_OK return value.
        //

        Result = S_OK;
    }

    ReleaseGraphLockExclusive(Graph);

    return Result;
}


GRAPH_LOAD_INFO GraphLoadInfo;

_Use_decl_annotations_
HRESULT
GraphLoadInfo(
    PGRAPH Graph
    )
/*++

Routine Description:

    This routine is called by graph solving worker threads prior to attempting
    any solving; it is responsible for initializing the graph structure and
    allocating (or reallocating) the necessary buffers required for graph
    solving, using the sizes indicated by the info structure previously set
    by the main thread via SetInfo().

Arguments:

    Graph - Supplies a pointer to the graph instance.

Return Value:

    S_OK - Success.

    E_POINTER - Graph was NULL.

    E_OUTOFMEMORY - Out of memory.

    PH_E_GRAPH_NO_INFO_SET - No graph information has been set for this graph.

    PH_E_GRAPH_INFO_ALREADY_LOADED - Graph information has already been loaded
        for this graph.

--*/
{
    PRTL Rtl;
    HRESULT Result = S_OK;
    PGRAPH_INFO Info;
    PGRAPH_INFO PrevInfo;
    PALLOCATOR Allocator;
    PPERFECT_HASH_TABLE Table;
    PPERFECT_HASH_CONTEXT Context;
    PASSIGNED_MEMORY_COVERAGE Coverage;
    PTABLE_INFO_ON_DISK TableInfoOnDisk;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Graph)) {
        return E_POINTER;
    }

    if (!IsGraphInfoSet(Graph)) {
        return PH_E_GRAPH_NO_INFO_SET;
    } else if (IsGraphInfoLoaded(Graph)) {
        return PH_E_GRAPH_INFO_ALREADY_LOADED;
    } else {
        Info = Graph->Info;
    }

    //
    // Sanity check the graph size is correct.
    //

    ASSERT(sizeof(*Graph) == Info->SizeOfGraphStruct);

    //
    // Initialize aliases.
    //

    Context = Info->Context;
    Rtl = Context->Rtl;
    PrevInfo = Info->PrevInfo;
    Allocator = Graph->Allocator;
    Table = Context->Table;
    TableInfoOnDisk = Table->TableInfoOnDisk;

    //
    // Set the relevant graph fields based on the provided info.
    //

    Graph->Context = Context;
    Graph->NumberOfSeeds = Table->TableInfoOnDisk->NumberOfSeeds;
    Graph->NumberOfKeys = Table->Keys->NumberOfElements.LowPart;

    Graph->ThreadId = GetCurrentThreadId();
    Graph->ThreadAttempt = 0;

    Graph->EdgeMask = Table->IndexMask;
    Graph->VertexMask = Table->HashMask;
    Graph->MaskFunctionId = Info->Context->MaskFunctionId;

    Graph->Flags.Paranoid = IsParanoid(Table);

    CopyInline(&Graph->Dimensions,
               &Info->Dimensions,
               sizeof(Graph->Dimensions));

    //
    // Allocate (or reallocate) arrays.
    //

#define ALLOC_ARRAY(Name, Type)                       \
    if (!Graph->##Name) {                             \
        Graph->##Name = (Type)(                       \
            Allocator->Vtbl->AlignedMalloc(           \
                Allocator,                            \
                (ULONG_PTR)Info->##Name##SizeInBytes, \
                YMMWORD_ALIGNMENT                     \
            )                                         \
        );                                            \
    } else {                                          \
        Graph->##Name## = (Type)(                     \
            Allocator->Vtbl->AlignedReAlloc(          \
                Allocator,                            \
                Graph->##Name,                        \
                (ULONG_PTR)Info->##Name##SizeInBytes, \
                YMMWORD_ALIGNMENT                     \
            )                                         \
        );                                            \
    }                                                 \
    if (!Graph->##Name) {                             \
        Result = E_OUTOFMEMORY;                       \
        goto Error;                                   \
    }

    ALLOC_ARRAY(Edges, PEDGE);
    ALLOC_ARRAY(Next, PEDGE);
    ALLOC_ARRAY(First, PVERTEX);
    ALLOC_ARRAY(Prev, PVERTEX);
    ALLOC_ARRAY(Assigned, PVERTEX);

    //
    // Set the bitmap sizes and then allocate (or reallocate) the bitmap
    // buffers.
    //

    Graph->DeletedEdgesBitmap.SizeOfBitMap = Graph->TotalNumberOfEdges;
    Graph->VisitedVerticesBitmap.SizeOfBitMap = Graph->NumberOfVertices;
    Graph->AssignedBitmap.SizeOfBitMap = Graph->NumberOfVertices;
    Graph->IndexBitmap.SizeOfBitMap = Graph->NumberOfVertices;

#define ALLOC_BITMAP_BUFFER(Name)                          \
    if (!Graph->##Name##.Buffer) {                         \
        Graph->##Name##.Buffer = (PULONG)(                 \
            Allocator->Vtbl->Malloc(                       \
                Allocator,                                 \
                (ULONG_PTR)Info->##Name##BufferSizeInBytes \
            )                                              \
        );                                                 \
    } else {                                               \
        Graph->##Name##.Buffer = (PULONG)(                 \
            Allocator->Vtbl->ReAlloc(                      \
                Allocator,                                 \
                Graph->##Name##.Buffer,                    \
                (ULONG_PTR)Info->##Name##BufferSizeInBytes \
            )                                              \
        );                                                 \
    }                                                      \
    if (!Graph->##Name##.Buffer) {                         \
        Result = E_OUTOFMEMORY;                            \
        goto Error;                                        \
    }

    ALLOC_BITMAP_BUFFER(DeletedEdgesBitmap);
    ALLOC_BITMAP_BUFFER(VisitedVerticesBitmap);
    ALLOC_BITMAP_BUFFER(AssignedBitmap);
    ALLOC_BITMAP_BUFFER(IndexBitmap);

    //
    // If we're in "first graph wins" mode, we don't need to prep any of the
    // memory coverage information.
    //

    if (FirstSolvedGraphWinsAndSkipMemoryCoverage(Context)) {
        Graph->Flags.WantsAssignedMemoryCoverage = FALSE;
        goto End;
    }

    if (FirstSolvedGraphWins(Context)) {

        Graph->Flags.WantsAssignedMemoryCoverage = TRUE;

    } else {

        switch (Context->BestCoverageType) {

            case BestCoverageTypeHighestNumberOfEmptyCacheLinesId:
                Graph->Flags.WantsAssignedMemoryCoverage = TRUE;
                break;

            case BestCoverageTypeLowestNumberOfCacheLinesUsedByKeysSubsetId:
                Graph->Flags.WantsAssignedMemoryCoverageForKeysSubset = TRUE;
                break;

            default:
                Result = PH_E_INVARIANT_CHECK_FAILED;
                PH_ERROR(GraphLoadInfo_InvalidBestCoverageType, Result);
                PH_RAISE(Result);

        }
    }

    //
    // Fill out the assigned memory coverage structure and allocate buffers.
    //

    Coverage = &Graph->AssignedMemoryCoverage;

    Coverage->TotalNumberOfPages = Info->AssignedArrayNumberOfPages;
    Coverage->TotalNumberOfLargePages = Info->AssignedArrayNumberOfLargePages;
    Coverage->TotalNumberOfCacheLines = Info->AssignedArrayNumberOfCacheLines;

#define ALLOC_ASSIGNED_ARRAY(Name, Type)               \
    if (!Coverage->##Name) {                           \
        Coverage->##Name = (PASSIGNED_##Type##_COUNT)( \
            Allocator->Vtbl->AlignedMalloc(            \
                Allocator,                             \
                (ULONG_PTR)Info->##Name##SizeInBytes,  \
                YMMWORD_ALIGNMENT                      \
            )                                          \
        );                                             \
    } else {                                           \
        Coverage->##Name = (PASSIGNED_##Type##_COUNT)( \
            Allocator->Vtbl->AlignedReAlloc(           \
                Allocator,                             \
                Coverage->##Name,                      \
                (ULONG_PTR)Info->##Name##SizeInBytes,  \
                YMMWORD_ALIGNMENT                      \
            )                                          \
        );                                             \
    }                                                  \
    if (!Coverage->##Name) {                           \
        Result = E_OUTOFMEMORY;                        \
        goto Error;                                    \
    }

    ALLOC_ASSIGNED_ARRAY(NumberOfAssignedPerPage, PAGE);
    ALLOC_ASSIGNED_ARRAY(NumberOfAssignedPerCacheLine, CACHE_LINE);

    //
    // The number of large pages consumed may not change between resize events;
    // avoid a realloc if unnecessary by checking the previous info's number of
    // large pages if applicable.
    //

#define ALLOC_ASSIGNED_LARGE_PAGE_ARRAY(Name)                                 \
    if (!Coverage->##Name) {                                                  \
        Coverage->##Name = (PASSIGNED_LARGE_PAGE_COUNT)(                      \
            Allocator->Vtbl->AlignedMalloc(                                   \
                Allocator,                                                    \
                (ULONG_PTR)Info->##Name##SizeInBytes,                         \
                YMMWORD_ALIGNMENT                                             \
            )                                                                 \
        );                                                                    \
    } else {                                                                  \
        BOOLEAN DoReAlloc = TRUE;                                             \
        if (PrevInfo) {                                                       \
            if (PrevInfo->##Name##SizeInBytes == Info->##Name##SizeInBytes) { \
                DoReAlloc = FALSE;                                            \
            }                                                                 \
        }                                                                     \
        if (DoReAlloc) {                                                      \
            Coverage->##Name = (PASSIGNED_LARGE_PAGE_COUNT)(                  \
                Allocator->Vtbl->AlignedReAlloc(                              \
                    Allocator,                                                \
                    Coverage->##Name,                                         \
                    (ULONG_PTR)Info->##Name##SizeInBytes,                     \
                    YMMWORD_ALIGNMENT                                         \
                )                                                             \
            );                                                                \
        }                                                                     \
    }                                                                         \
    if (!Coverage->##Name) {                                                  \
        Result = E_OUTOFMEMORY;                                               \
        goto Error;                                                           \
    }

    ALLOC_ASSIGNED_LARGE_PAGE_ARRAY(NumberOfAssignedPerLargePage);

    //
    // We're done, finish up.
    //

    goto End;

Error:

    if (Result == S_OK) {
        Result = E_UNEXPECTED;
    }

    //
    // Intentional follow-on to End.
    //

End:

    if (SUCCEEDED(Result)) {
        Graph->Flags.IsInfoLoaded = TRUE;
        Graph->LastLoadedNumberOfVertices = Graph->NumberOfVertices;
    }

    return Result;
}


GRAPH_RESET GraphReset;

_Use_decl_annotations_
HRESULT
GraphReset(
    PGRAPH Graph
    )
/*++

Routine Description:

    Resets the state of a graph instance after a solving attempt, such that it
    can be used for a subsequent attempt.

Arguments:

    Graph - Supplies a pointer to the graph instance to reset.

Return Value:

    PH_S_CONTINUE_GRAPH_SOLVING - Graph was successfully reset and graph solving
        should continue.

    PH_S_GRAPH_SOLVING_STOPPED - Graph solving has been stopped.  The graph is
        not reset and solving should not continue.

    PH_S_TABLE_RESIZE_IMMINENT - The reset was not performed as a table resize
        is imminent (and thus, attempts at solving this current graph can be
        stopped).

    PH_E_SYSTEM_CALL_FAILED - A system call failed.

--*/
{
    PRTL Rtl;
    PGRAPH_INFO Info;
    HRESULT Result = PH_S_CONTINUE_GRAPH_SOLVING;
    PPERFECT_HASH_CONTEXT Context;
    ULONG TotalNumberOfPages;
    ULONG TotalNumberOfLargePages;
    ULONG TotalNumberOfCacheLines;
    PASSIGNED_MEMORY_COVERAGE Coverage;
    PASSIGNED_PAGE_COUNT NumberOfAssignedPerPage;
    PASSIGNED_LARGE_PAGE_COUNT NumberOfAssignedPerLargePage;
    PASSIGNED_CACHE_LINE_COUNT NumberOfAssignedPerCacheLine;

    //
    // Initialize aliases.
    //

    Context = Graph->Context;
    Info = Graph->Info;
    Rtl = Context->Rtl;

    //
    // Fast-path exit: if the context is indicating to stop solving, return.
    //

    if (StopSolving(Context)) {
        return PH_S_GRAPH_SOLVING_STOPPED;
    }

    //
    // Increment the thread attempt counter, and interlocked-increment the
    // global context counter.  If the global attempt is equal to the resize
    // table threshold, signal the event to try a larger table size and return
    // with the error code indicating a table resize is imminent.
    //

    ++Graph->ThreadAttempt;

    Graph->Attempt = InterlockedIncrement64(&Context->Attempts);

    if (!Context->FinishedCount &&
        Graph->Attempt == Context->ResizeTableThreshold) {

        if (!SetEvent(Context->TryLargerTableSizeEvent)) {
            SYS_ERROR(SetEvent);
            Result = PH_E_SYSTEM_CALL_FAILED;
            goto Error;
        }
        return PH_S_TABLE_RESIZE_IMMINENT;
    }

    //
    // If find best memory coverage mode is active, determine if we've hit the
    // target number of attempts, and if so, whether or not a best graph has
    // been found.
    //

    if (FindBestMemoryCoverage(Context)) {
        if (Graph->Attempt == Context->BestCoverageAttempts) {

            //
            // Toggle the stop solving flag.
            //

            SetStopSolving(Context);

            //
            // Acquire the best graph critical section then determine if best
            // graph is non-NULL.  If so, a solution has been found; verify
            // the finished count is greater than 0, then clear the context's
            // best graph field and push it onto the context's finished list.
            //

            EnterCriticalSection(&Context->BestGraphCriticalSection);

            if (Context->BestGraph) {

                PGRAPH BestGraph;
                PASSIGNED_MEMORY_COVERAGE BestCoverage;

                //
                // If finished count is 0 at this point, a critical invariant
                // has been violated, so raise a runtime exception.
                //

                if (Context->FinishedCount == 0) {
                    PH_RAISE(PH_E_INVARIANT_CHECK_FAILED);
                }

                //
                // Copy the best graph's coverage information to the table,
                // then insert the graph to the head of the finished work list.
                //

                BestGraph = Context->BestGraph;
                Context->BestGraph = NULL;

                BestCoverage = &BestGraph->AssignedMemoryCoverage;
                CopyCoverage(Context->Table->Coverage, BestCoverage);

                InsertHeadFinishedWork(Context, &BestGraph->ListEntry);

            } else {

                //
                // Verify our finished count is 0.
                //
                // N.B. This invariant is less critical than the one above,
                //      and may need reviewing down the track, if we ever
                //      support the notion of finding solutions but none of
                //      them meet our criteria for "best" (i.e. they didn't
                //      hit a target number of empty free cache lines, for
                //      example).
                //

                if (Context->FinishedCount != 0) {
                    PH_RAISE(PH_E_INVARIANT_CHECK_FAILED);
                }
            }

            ASSERT(Context->BestGraph == NULL);

            LeaveCriticalSection(&Context->BestGraphCriticalSection);

            //
            // Stop the solve timers here.  (These are less useful when not in
            // "first graph wins" mode.)
            //

            CONTEXT_END_TIMERS(Solve);

            //
            // Submit the finished threadpool work regardless of whether or
            // not a graph was found.  The finished callback will set the
            // appropriate success or failure events after waiting for all
            // the graph contexts to finish and then assessing the context.
            //

            SubmitThreadpoolWork(Context->FinishedWork);

            //
            // Return graph solving stopped.
            //

            Result = PH_S_GRAPH_SOLVING_STOPPED;
            return Result;
        }
    }

    //
    // Clear the bitmap buffers.
    //
    // N.B. We use __stosq() to prevent the PGO builds trying to call memset.
    //

#define ZERO_BITMAP_BUFFER(Name)                           \
    ASSERT(0 == Info->##Name##BufferSizeInBytes -          \
           ((Info->##Name##BufferSizeInBytes >> 3) << 3)); \
    __stosq((PDWORD64)Graph->##Name##.Buffer,              \
            0,                                             \
            Info->##Name##BufferSizeInBytes >> 3)

    ZERO_BITMAP_BUFFER(DeletedEdgesBitmap);
    ZERO_BITMAP_BUFFER(VisitedVerticesBitmap);
    ZERO_BITMAP_BUFFER(AssignedBitmap);
    ZERO_BITMAP_BUFFER(IndexBitmap);

    //
    // "Empty" all of the nodes.
    //
    // N.B. We use __stosq() to prevent the PGO builds trying to call memset.
    //

#define EMPTY_ARRAY(Name)                            \
    ASSERT(0 == Info->##Name##SizeInBytes -          \
           ((Info->##Name##SizeInBytes >> 3) << 3)); \
    __stosq((PDWORD64)Graph->##Name,                 \
            ~0ULL,                                   \
            Info->##Name##SizeInBytes >> 3)

    EMPTY_ARRAY(First);
    EMPTY_ARRAY(Prev);
    EMPTY_ARRAY(Next);
    EMPTY_ARRAY(Edges);

    //
    // Clear any remaining values.
    //

    Graph->Collisions = 0;
    Graph->DeletedEdgeCount = 0;
    Graph->VisitedVerticesCount = 0;

    Graph->Flags.Shrinking = FALSE;
    Graph->Flags.IsAcyclic = FALSE;

    //
    // Skip the memory coverage reset if we're in "first graph wins" mode and
    // have been requested to skip memory coverage.
    //

    if (FirstSolvedGraphWinsAndSkipMemoryCoverage(Context)) {
        goto End;
    }

    //
    // Clear the assigned memory coverage counts and arrays.
    //

    Coverage = &Graph->AssignedMemoryCoverage;

    //
    // Capture the totals and pointers prior to zeroing the struct.
    //

    TotalNumberOfPages = Coverage->TotalNumberOfPages;
    TotalNumberOfLargePages = Coverage->TotalNumberOfLargePages;
    TotalNumberOfCacheLines = Coverage->TotalNumberOfCacheLines;

    NumberOfAssignedPerPage = Coverage->NumberOfAssignedPerPage;
    NumberOfAssignedPerLargePage = Coverage->NumberOfAssignedPerLargePage;
    NumberOfAssignedPerCacheLine = Coverage->NumberOfAssignedPerCacheLine;

    ZeroStructPointer(Coverage);

    //
    // Restore the totals and pointers.
    //

    Coverage->TotalNumberOfPages = TotalNumberOfPages;
    Coverage->TotalNumberOfLargePages = TotalNumberOfLargePages;
    Coverage->TotalNumberOfCacheLines = TotalNumberOfCacheLines;

    Coverage->NumberOfAssignedPerPage = NumberOfAssignedPerPage;
    Coverage->NumberOfAssignedPerLargePage = NumberOfAssignedPerLargePage;
    Coverage->NumberOfAssignedPerCacheLine = NumberOfAssignedPerCacheLine;

#define ZERO_ASSIGNED_ARRAY(Name) \
    ZeroMemory(Coverage->##Name, Info->##Name##SizeInBytes)

    ZERO_ASSIGNED_ARRAY(NumberOfAssignedPerPage);
    ZERO_ASSIGNED_ARRAY(NumberOfAssignedPerLargePage);
    ZERO_ASSIGNED_ARRAY(NumberOfAssignedPerCacheLine);

    //
    // We're done, finish up.
    //

    goto End;

Error:

    if (Result == S_OK) {
        Result = E_UNEXPECTED;
    }

    //
    // Intentional follow-on to End.
    //

End:

    return Result;
}


GRAPH_LOAD_NEW_SEEDS GraphLoadNewSeeds;

_Use_decl_annotations_
HRESULT
GraphLoadNewSeeds(
    PGRAPH Graph
    )
/*++

Routine Description:

    Loads new seed data for a graph instance.  This is called prior to each
    solving attempt.

Arguments:

    Graph - Supplies a pointer to the graph instance for which the new seed
        data will be loaded.

Return Value:

    S_OK - Success.

    E_POINTER - Graph was NULL.

    PH_E_NO_MORE_SEEDS - No more seed data is available.  (Not currently
        returned for this implementation.)

    PH_E_SPARE_GRAPH - Graph is indicated as the spare graph.

--*/
{
    PRTL Rtl;
    HRESULT Result;
    ULONG SizeInBytes;

    if (!ARGUMENT_PRESENT(Graph)) {
        return E_POINTER;
    }

    if (IsSpareGraph(Graph)) {
        return PH_E_SPARE_GRAPH;
    }

    SizeInBytes = Graph->NumberOfSeeds * sizeof(Graph->FirstSeed);

    Rtl = Graph->Rtl;

    Result = Rtl->Vtbl->GenerateRandomBytes(Rtl,
                                            SizeInBytes,
                                            (PBYTE)&Graph->FirstSeed);

    return Result;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
