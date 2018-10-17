
//
// Begin CompiledPerfectHashTableMacroGlue.c.
//

#define CPH_SEED1(U) U##_SEED1
#define CPH_SEED2(U) U##_SEED2
#define CPH_SEED3(U) U##_SEED3
#define CPH_SEED4(U) U##_SEED4

#define CPH_HASH_MASK(U) U##_HASH_MASK
#define CPH_INDEX_MASK(U) U##_INDEX_MASK

#define CPH_TABLE_DATA(T) T##_TableData
#define CPH_TABLE_VALUES(T) T##_TableValues
#define CPH_KEYS(T) T##_Keys
#define CPH_NUMBER_OF_KEYS(U) U##_NUMBER_OF_KEYS

#define CPH_INDEX_ROUTINE_NAME(T) CompiledPerfectHash_##T##_Index
#define CPH_LOOKUP_ROUTINE_NAME(T) CompiledPerfectHash_##T##_Lookup
#define CPH_INSERT_ROUTINE_NAME(T) CompiledPerfectHash_##T##_Insert
#define CPH_DELETE_ROUTINE_NAME(T) CompiledPerfectHash_##T##_Delete

#define CPH_INDEX_INLINE_ROUTINE_NAME(T) CompiledPerfectHash_##T##_IndexInline
#define CPH_LOOKUP_INLINE_ROUTINE_NAME(T) CompiledPerfectHash_##T##_LookupInline
#define CPH_INSERT_INLINE_ROUTINE_NAME(T) CompiledPerfectHash_##T##_InsertInline
#define CPH_DELETE_INLINE_ROUTINE_NAME(T) CompiledPerfectHash_##T##_DeleteInline

////////////////////////////////////////////////////////////////////////////////
// Index
////////////////////////////////////////////////////////////////////////////////

//
// Normal
//

#define CPH_INDEX_ROUTINE_HEADER(T)      \
CPHAPI COMPILED_PERFECT_HASH_TABLE_INDEX \
    CompiledPerfectHash_##T##_Index;     \
                                         \
_Use_decl_annotations_                   \
ULONG                                    \
CompiledPerfectHash_##T##_Index(         \
    ULONG Key                            \
    )

//
// Inline
//

#define CPH_INDEX_INLINE_ROUTINE_HEADER(T) \
FORCEINLINE                                \
ULONG                                      \
CompiledPerfectHash_##T##_IndexInline(     \
    ULONG Key                              \
    )

////////////////////////////////////////////////////////////////////////////////
// Lookup
////////////////////////////////////////////////////////////////////////////////

//
// Normal
//

#define CPH_LOOKUP_ROUTINE_HEADER(T)      \
CPHAPI COMPILED_PERFECT_HASH_TABLE_LOOKUP \
    CompiledPerfectHash_##T##_Lookup;     \
                                          \
_Use_decl_annotations_                    \
ULONG                                     \
CompiledPerfectHash_##T##_Lookup(         \
    ULONG Key                             \
    )

//
// Inline
//

#define CPH_LOOKUP_INLINE_ROUTINE_HEADER(T) \
FORCEINLINE                                 \
ULONG                                       \
CompiledPerfectHash_##T##_LookupInline(     \
    ULONG Key                               \
    )

////////////////////////////////////////////////////////////////////////////////
// Insert
////////////////////////////////////////////////////////////////////////////////

//
// Normal
//

#define CPH_INSERT_ROUTINE_HEADER(T)      \
CPHAPI COMPILED_PERFECT_HASH_TABLE_INSERT \
    CompiledPerfectHash_##T##_Insert;     \
                                          \
_Use_decl_annotations_                    \
ULONG                                     \
CompiledPerfectHash_##T##_Insert(         \
    ULONG Key,                            \
    ULONG Value                           \
    )

//
// Inline
//

#define CPH_INSERT_INLINE_ROUTINE_HEADER(T) \
FORCEINLINE                                 \
ULONG                                       \
CompiledPerfectHash_##T##_InsertInline(     \
    ULONG Key,                              \
    ULONG Value                             \
    )


////////////////////////////////////////////////////////////////////////////////
// Delete
////////////////////////////////////////////////////////////////////////////////

//
// Normal
//

#define CPH_DELETE_ROUTINE_HEADER(T)      \
CPHAPI COMPILED_PERFECT_HASH_TABLE_DELETE \
    CompiledPerfectHash_##T##_Delete;     \
                                          \
_Use_decl_annotations_                    \
ULONG                                     \
CompiledPerfectHash_##T##_Delete(         \
    ULONG Key                             \
    )

//
// Inline
//

#define CPH_DELETE_INLINE_ROUTINE_HEADER(T) \
FORCEINLINE                                 \
ULONG                                       \
CompiledPerfectHash_##T##_DeleteInline(     \
    ULONG Key                               \
    )


////////////////////////////////////////////////////////////////////////////////
// Test and Benchmarking
////////////////////////////////////////////////////////////////////////////////

#define CPH_TEST_CPH_ROUTINE_NAME(T) TestCompiledPerfectHashTable_##T
#define CPH_BENCHMARK_FULL_CPH_ROUTINE_NAME(T) BenchmarkFullCompiledPerfectHashTable_##T
#define CPH_BENCHMARK_INDEX_CPH_ROUTINE_NAME(T) BenchmarkIndexCompiledPerfectHashTable_##T

#define TEST_CPH_ROUTINE_HEADER(T)    \
TEST_COMPILED_PERFECT_HASH_TABLE      \
    TestCompiledPerfectHashTable_##T; \
                                      \
_Use_decl_annotations_                \
ULONG                                 \
TestCompiledPerfectHashTable_##T##(   \
    BOOLEAN DebugBreakOnFailure       \
    )

#define BENCHMARK_FULL_CPH_ROUTINE_HEADER(T)   \
BENCHMARK_FULL_COMPILED_PERFECT_HASH_TABLE     \
    BenchmarkFullCompiledPerfectHashTable_##T; \
                                               \
_Use_decl_annotations_                         \
ULONG                                          \
BenchmarkFullCompiledPerfectHashTable_##T##(   \
    ULONG Seconds                              \
    )

#define BENCHMARK_INDEX_CPH_ROUTINE_HEADER(T)   \
BENCHMARK_INDEX_COMPILED_PERFECT_HASH_TABLE     \
    BenchmarkIndexCompiledPerfectHashTable_##T; \
                                                \
_Use_decl_annotations_                          \
ULONG                                           \
BenchmarkIndexCompiledPerfectHashTable_##T##(   \
    ULONG Seconds                               \
    )


////////////////////////////////////////////////////////////////////////////////
// End of routine headers.
////////////////////////////////////////////////////////////////////////////////

#define CPH_DEFINE_TABLE_ROUTINES(T)                                        \
CPHAPI COMPILED_PERFECT_HASH_TABLE_INDEX CompiledPerfectHash_##T##_Index;   \
CPHAPI COMPILED_PERFECT_HASH_TABLE_LOOKUP CompiledPerfectHash_##T##_Lookup; \
CPHAPI COMPILED_PERFECT_HASH_TABLE_INSERT CompiledPerfectHash_##T##_Insert; \
CPHAPI COMPILED_PERFECT_HASH_TABLE_DELETE CompiledPerfectHash_##T##_Delete

#define CPH_DEFINE_TEST_AND_BENCHMARKING_ROUTINES(T) \
extern TEST_COMPILED_PERFECT_HASH_TABLE              \
    TestCompiledPerfectHashTable_##T;                \
                                                     \
extern BENCHMARK_FULL_COMPILED_PERFECT_HASH_TABLE    \
    BenchmarkFullCompiledPerfectHashTable_##T;       \
                                                     \
extern BENCHMARK_INDEX_COMPILED_PERFECT_HASH_TABLE   \
    BenchmarkIndexCompiledPerfectHashTable_##T

#define CPH_INDEX_ROUTINE(T) CPH_INDEX_ROUTINE_NAME(T)
#define CPH_LOOKUP_ROUTINE(T) CPH_LOOKUP_ROUTINE_NAME(T)
#define CPH_INSERT_ROUTINE(T) CPH_INSERT_ROUTINE_NAME(T)
#define CPH_DELETE_ROUTINE(T) CPH_DELETE_ROUTINE_NAME(T)

#define CPH_INDEX_INLINE_ROUTINE(T) CPH_INDEX_INLINE_ROUTINE_NAME(T)
#define CPH_LOOKUP_INLINE_ROUTINE(T) CPH_LOOKUP_INLINE_ROUTINE_NAME(T)
#define CPH_INSERT_INLINE_ROUTINE(T) CPH_INSERT_INLINE_ROUTINE_NAME(T)
#define CPH_DELETE_INLINE_ROUTINE(T) CPH_DELETE_INLINE_ROUTINE_NAME(T)

#define EXPAND_SEED1(U) CPH_SEED1(U)
#define EXPAND_SEED2(U) CPH_SEED2(U)
#define EXPAND_SEED3(U) CPH_SEED3(U)
#define EXPAND_SEED4(U) CPH_SEED4(U)

#define EXPAND_HASH_MASK(U) CPH_HASH_MASK(U)
#define EXPAND_INDEX_MASK(U) CPH_INDEX_MASK(U)
#define EXPAND_TABLE_DATA(T) CPH_TABLE_DATA(T)
#define EXPAND_TABLE_VALUES(T) CPH_TABLE_VALUES(T)
#define EXPAND_KEYS(T) CPH_KEYS(T)
#define EXPAND_NUMBER_OF_KEYS(U) CPH_NUMBER_OF_KEYS(U)

#define EXPAND_INDEX_ROUTINE(T) CPH_INDEX_ROUTINE(T)
#define EXPAND_LOOKUP_ROUTINE(T) CPH_LOOKUP_ROUTINE(T)
#define EXPAND_INSERT_ROUTINE(T) CPH_INSERT_ROUTINE(T)
#define EXPAND_DELETE_ROUTINE(T) CPH_DELETE_ROUTINE(T)

#define EXPAND_INDEX_INLINE_ROUTINE(T) CPH_INDEX_INLINE_ROUTINE(T)
#define EXPAND_LOOKUP_INLINE_ROUTINE(T) CPH_LOOKUP_INLINE_ROUTINE(T)
#define EXPAND_INSERT_INLINE_ROUTINE(T) CPH_INSERT_INLINE_ROUTINE(T)
#define EXPAND_DELETE_INLINE_ROUTINE(T) CPH_DELETE_INLINE_ROUTINE(T)

#define EXPAND_INDEX_ROUTINE_HEADER(T) CPH_INDEX_ROUTINE_HEADER(T)
#define EXPAND_LOOKUP_ROUTINE_HEADER(T) CPH_LOOKUP_ROUTINE_HEADER(T)
#define EXPAND_INSERT_ROUTINE_HEADER(T) CPH_INSERT_ROUTINE_HEADER(T)
#define EXPAND_DELETE_ROUTINE_HEADER(T) CPH_DELETE_ROUTINE_HEADER(T)

#define EXPAND_INDEX_INLINE_ROUTINE_HEADER(T) CPH_INDEX_INLINE_ROUTINE_HEADER(T)
#define EXPAND_LOOKUP_INLINE_ROUTINE_HEADER(T) CPH_LOOKUP_INLINE_ROUTINE_HEADER(T)
#define EXPAND_INSERT_INLINE_ROUTINE_HEADER(T) CPH_INSERT_INLINE_ROUTINE_HEADER(T)
#define EXPAND_DELETE_INLINE_ROUTINE_HEADER(T) CPH_DELETE_INLINE_ROUTINE_HEADER(T)

#define EXPAND_TEST_CPH_ROUTINE_HEADER(T) TEST_CPH_ROUTINE_HEADER(T)
#define EXPAND_BENCHMARK_FULL_CPH_ROUTINE_HEADER(T) BENCHMARK_FULL_CPH_ROUTINE_HEADER(T)
#define EXPAND_BENCHMARK_INDEX_CPH_ROUTINE_HEADER(T) BENCHMARK_INDEX_CPH_ROUTINE_HEADER(T)

#define EXPAND_TEST_CPH_ROUTINE_NAME(T) CPH_TEST_CPH_ROUTINE_NAME(T)
#define EXPAND_BENCHMARK_FULL_CPH_ROUTINE_NAME(T) CPH_BENCHMARK_FULL_CPH_ROUTINE_NAME(T)
#define EXPAND_BENCHMARK_INDEX_CPH_ROUTINE_NAME(T) CPH_BENCHMARK_INDEX_CPH_ROUTINE_NAME(T)

#define EXPAND_DEFINE_TABLE_ROUTINES(T) CPH_DEFINE_TABLE_ROUTINES(T)
#define EXPAND_DEFINE_TEST_AND_BENCHMARKING_ROUTINES(T) CPH_DEFINE_TEST_AND_BENCHMARKING_ROUTINES(T)

#define SEED1 EXPAND_SEED1(CPH_TABLENAME_UPPER)
#define SEED2 EXPAND_SEED2(CPH_TABLENAME_UPPER)
#define SEED3 EXPAND_SEED3(CPH_TABLENAME_UPPER)
#define SEED4 EXPAND_SEED4(CPH_TABLENAME_UPPER)

#define HASH_MASK EXPAND_HASH_MASK(CPH_TABLENAME_UPPER)
#define INDEX_MASK EXPAND_INDEX_MASK(CPH_TABLENAME_UPPER)
#define TABLE_DATA EXPAND_TABLE_DATA(CPH_TABLENAME)
#define TABLE_VALUES EXPAND_TABLE_VALUES(CPH_TABLENAME)
#define KEYS EXPAND_KEYS(CPH_TABLENAME)
#define NUMBER_OF_KEYS EXPAND_NUMBER_OF_KEYS(CPH_TABLENAME_UPPER)

#define DECLARE_INDEX_ROUTINE_HEADER() EXPAND_INDEX_ROUTINE_HEADER(CPH_TABLENAME)
#define DECLARE_LOOKUP_ROUTINE_HEADER() EXPAND_LOOKUP_ROUTINE_HEADER(CPH_TABLENAME)
#define DECLARE_INSERT_ROUTINE_HEADER() EXPAND_INSERT_ROUTINE_HEADER(CPH_TABLENAME)
#define DECLARE_DELETE_ROUTINE_HEADER() EXPAND_DELETE_ROUTINE_HEADER(CPH_TABLENAME)

#define DECLARE_INDEX_INLINE_ROUTINE_HEADER() EXPAND_INDEX_INLINE_ROUTINE_HEADER(CPH_TABLENAME)
#define DECLARE_LOOKUP_INLINE_ROUTINE_HEADER() EXPAND_LOOKUP_INLINE_ROUTINE_HEADER(CPH_TABLENAME)
#define DECLARE_INSERT_INLINE_ROUTINE_HEADER() EXPAND_INSERT_INLINE_ROUTINE_HEADER(CPH_TABLENAME)
#define DECLARE_DELETE_INLINE_ROUTINE_HEADER() EXPAND_DELETE_INLINE_ROUTINE_HEADER(CPH_TABLENAME)

#define INDEX_INLINE_ROUTINE EXPAND_INDEX_INLINE_ROUTINE(CPH_TABLENAME)
#define LOOKUP_INLINE_ROUTINE EXPAND_LOOKUP_INLINE_ROUTINE(CPH_TABLENAME)
#define INSERT_INLINE_ROUTINE EXPAND_INSERT_INLINE_ROUTINE(CPH_TABLENAME)
#define DELETE_INLINE_ROUTINE EXPAND_DELETE_INLINE_ROUTINE(CPH_TABLENAME)

#define TEST_CPH_ROUTINE EXPAND_TEST_CPH_ROUTINE_NAME(CPH_TABLENAME)
#define BENCHMARK_FULL_CPH_ROUTINE EXPAND_BENCHMARK_FULL_CPH_ROUTINE_NAME(CPH_TABLENAME)
#define BENCHMARK_INDEX_CPH_ROUTINE EXPAND_BENCHMARK_INDEX_CPH_ROUTINE_NAME(CPH_TABLENAME)

#define DECLARE_TEST_CPH_ROUTINE() EXPAND_TEST_CPH_ROUTINE_HEADER(CPH_TABLENAME)
#define DECLARE_BENCHMARK_FULL_CPH_ROUTINE() EXPAND_BENCHMARK_FULL_CPH_ROUTINE_HEADER(CPH_TABLENAME)
#define DECLARE_BENCHMARK_INDEX_CPH_ROUTINE() EXPAND_BENCHMARK_INDEX_CPH_ROUTINE_HEADER(CPH_TABLENAME)

#define DEFINE_TABLE_ROUTINES() EXPAND_DEFINE_TABLE_ROUTINES(CPH_TABLENAME)
#define DEFINE_TEST_AND_BENCHMARKING_ROUTINES() EXPAND_DEFINE_TEST_AND_BENCHMARKING_ROUTINES(CPH_TABLENAME)

//
// End CompiledPerfectHashTableMacroGlue.c.
//