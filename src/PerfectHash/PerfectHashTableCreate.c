/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    PerfectHashTableCreate.c

Abstract:

    This module implements the Create() routine for the perfect hash table
    component.

--*/

#include "stdafx.h"

PERFECT_HASH_TABLE_CREATE PerfectHashTableCreate;

_Use_decl_annotations_
HRESULT
PerfectHashTableCreate(
    PPERFECT_HASH_TABLE Table,
    PPERFECT_HASH_CONTEXT Context,
    PERFECT_HASH_ALGORITHM_ID AlgorithmId,
    PERFECT_HASH_MASK_FUNCTION_ID MaskFunctionId,
    PERFECT_HASH_HASH_FUNCTION_ID HashFunctionId,
    PPERFECT_HASH_KEYS Keys,
    PCUNICODE_STRING BaseOutputDirectory,
    PCUNICODE_STRING TableBaseName,
    PPERFECT_HASH_TABLE_CREATE_FLAGS TableCreateFlagsPointer
    )
/*++

Routine Description:

    Creates a perfect hash table.

Arguments:

    Table - Supplies a perfect hash table instance for which the creation
        operation will be performed.

    Context - Supplies a pointer to an initialized PERFECT_HASH_CONTEXT
        structure that can be used by the underlying algorithm in order to
        search for perfect hash solutions in parallel.  The context must not
        be locked; this routine will acquire the context's lock exclusively
        for its duration.

    AlgorithmId - Supplies the algorithm to use.

    MaskFunctionId - Supplies the type of masking to use.  The algorithm and
        hash function must both support the requested masking type.

    HashFunctionId - Supplies the hash function to use.

    Keys - Supplies a pointer to a PERFECT_HASH_KEYS interface.

    BaseOutputDirectory - Supplies the base output directory to use for saving
        files related to the perfect hash table solution.

    TableBaseName - Optionally supplies an explicit base name to use for the
        perfect hash table.  This will override the base name that is derived
        from the Keys instance.

    TableCreateFlags - Optionally supplies a pointer to a context create
        table flags structure that can be used to customize table creation.

Return Value:

    S_OK - Table created successfully.

    The following error codes may also be returned.  Note that this list is not
    guaranteed to be exhaustive; that is, error codes other than the ones listed
    below may also be returned.

    E_POINTER - Table, Context, Keys or BaseOutputDirectory params were NULL.

    E_INVALIDARG - BaseOutputDirectory or TableBaseName were not valid.

    E_UNEXPECTED - Internal error.

    E_OUTOFMEMORY - Out of memory.

    PH_E_INVALID_ALGORITHM_ID - Invalid algorithm ID.

    PH_E_INVALID_HASH_FUNCTION_ID - Invalid hash function ID.

    PH_E_INVALID_MASK_FUNCTION_ID - Invalid mask function ID.

    PH_E_INVALID_TABLE_CREATE_FLAGS - Invalid table creation flags.

    PH_E_TABLE_LOCKED - The table is locked.

    PH_E_CONTEXT_LOCKED - The context is locked.

    PH_E_KEYS_FILE_NAME_NOT_VALID_C_IDENTIFIER - The file name component of
        the keys path contained characters that prevented it from being
        considered a valid C identifier.  As this name is used to generate
        various C structures, it must be valid.

    PH_E_BASE_NAME_INVALID_C_IDENTIFIER - The table base name component was
        not a valid C identifier.

--*/
{
    PRTL Rtl;
    PALLOCATOR Allocator;
    HRESULT Result = S_OK;
    PERFECT_HASH_TABLE_CREATE_FLAGS TableCreateFlags;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Table)) {
        return E_POINTER;
    } else {
        Rtl = Table->Rtl;
        Allocator = Table->Allocator;
    }

    if (!ARGUMENT_PRESENT(Context)) {
        return E_POINTER;
    }

    if (!ARGUMENT_PRESENT(Keys)) {
        return E_POINTER;
    }

    if (Keys->NumberOfElements.QuadPart > MAXIMUM_NUMBER_OF_KEYS) {
        return PH_E_TOO_MANY_KEYS;
    }

    if (!ARGUMENT_PRESENT(BaseOutputDirectory)) {
        return E_POINTER;
    } else if (Table->BaseOutputDirectory) {
        Result = PH_E_INVARIANT_CHECK_FAILED;
        PH_ERROR(PerfectHashTableCreate, Result);
        return Result;
    }

    if (!IsValidMinimumDirectoryNullTerminatedUnicodeString(
            BaseOutputDirectory)) {
        return E_INVALIDARG;
    } else {
        Table->BaseOutputDirectory = BaseOutputDirectory;
    }

    if (ARGUMENT_PRESENT(TableBaseName)) {
        if (!IsValidUnicodeString(TableBaseName)) {
            return E_INVALIDARG;
        }
    }

    VALIDATE_FLAGS(TableCreate, TABLE_CREATE);

    if (!IsValidPerfectHashAlgorithmId(AlgorithmId)) {
        return PH_E_INVALID_ALGORITHM_ID;
    }

    if (!IsValidPerfectHashHashFunctionId(HashFunctionId)) {
        return PH_E_INVALID_HASH_FUNCTION_ID;
    }

    if (!IsValidPerfectHashMaskFunctionId(MaskFunctionId)) {
        return PH_E_INVALID_MASK_FUNCTION_ID;
    }

    if (!TryAcquirePerfectHashContextLockExclusive(Context)) {
        return PH_E_CONTEXT_LOCKED;
    }

    if (!TryAcquirePerfectHashTableLockExclusive(Table)) {
        ReleasePerfectHashContextLockExclusive(Context);
        return PH_E_TABLE_LOCKED;
    }

    if (Context->State.NeedsReset) {
        Result = PerfectHashContextReset(Context);
        if (FAILED(Result)) {
            PH_ERROR(PerfectHashContextReset, Result);
            goto Error;
        }
    }

    //
    // No table should be associated with the context at this point and vice
    // versa, and no keys should be set.
    //

    if (Context->Table) {
        Result = PH_E_INVARIANT_CHECK_FAILED;
        PH_ERROR(PerfectHashTableCreate, Result);
        goto Error;
    }

    if (Table->Context) {
        Result = PH_E_INVARIANT_CHECK_FAILED;
        PH_ERROR(PerfectHashTableCreate, Result);
        goto Error;
    }

    if (Table->Keys) {
        Result = PH_E_INVARIANT_CHECK_FAILED;
        PH_ERROR(PerfectHashTableCreate, Result);
        goto Error;
    }

    Context->Vtbl->AddRef(Context);
    Table->Context = Context;
    Context->Table = Table;

    Keys->Vtbl->AddRef(Keys);
    Table->Keys = Keys;

    //
    // Copy create flags.
    //

    Table->TableCreateFlags.AsULong = TableCreateFlags.AsULong;

    //
    // Our main enumeration IDs get replicated in both structures.
    //

    Table->AlgorithmId = Context->AlgorithmId = AlgorithmId;
    Table->MaskFunctionId = Context->MaskFunctionId = MaskFunctionId;
    Table->HashFunctionId = Context->HashFunctionId = HashFunctionId;

    //
    // Complete initialization of the table's vtbl now that the hash/mask IDs
    // have been set.
    //

    CompletePerfectHashTableVtblInitialization(Table);

    //
    // Dispatch remaining creation work to the algorithm-specific routine.
    //

    Result = CreationRoutines[AlgorithmId](Table);

    if (FAILED(Result)) {
        goto Error;
    }

    //
    // Successfully created the table.  Create the values array, then update
    // flags and state.
    //

    Result = PerfectHashTableCreateValuesArray(Table, 0);
    if (FAILED(Result)) {
        PH_ERROR(PerfectHashTableCreateValuesArray, Result);
        goto Error;
    }

    Table->Flags.Created = TRUE;
    Table->Flags.Loaded = FALSE;
    Table->State.Valid = TRUE;

    goto End;

Error:

    if (Result == S_OK) {
        Result = E_UNEXPECTED;
    }

    //
    // Intentional follow-on to End.
    //

End:

    Table->Context = NULL;
    Context->Table = NULL;
    Context->State.NeedsReset = TRUE;

    ReleasePerfectHashContextLockExclusive(Context);
    ReleasePerfectHashContextLockExclusive(Table);

    return Result;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
// # vim:set ts=8 sw=4 sts=4 tw=78 et:
