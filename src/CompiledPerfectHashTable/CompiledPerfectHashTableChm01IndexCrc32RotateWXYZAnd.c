
DECLARE_INDEX_ROUTINE()
{
    CPHINDEX Index;
    CPHDKEY Vertex1;
    CPHDKEY Vertex2;
    CPHDKEY MaskedLow;
    CPHDKEY MaskedHigh;
    CPHDKEY DownsizedKey;

    //IACA_VC_START();

    DownsizedKey = DOWNSIZE_KEY(Key);

    Vertex1 = Crc32u32(SEED1, RotateRight32(DownsizedKey, SEED3_BYTE1));
    Vertex1 = RotateLeft32(Vertex1, SEED3_BYTE2);

    Vertex2 = Crc32u32(SEED2, RotateLeft32(DownsizedKey, SEED3_BYTE3));
    Vertex2 = RotateRight32(Vertex2, SEED3_BYTE4);

    MaskedLow = Vertex1 & HASH_MASK;
    MaskedHigh = Vertex2 & HASH_MASK;

    Vertex1 = TABLE_DATA[MaskedLow];
    Vertex2 = TABLE_DATA[MaskedHigh];

    Index = (CPHINDEX)((Vertex1 + Vertex2) & INDEX_MASK);

    //IACA_VC_END();

    return Index;
}

#ifndef CPH_INLINE_ROUTINES

DECLARE_INDEX_IACA_ROUTINE()
{
    CPHINDEX Index;
    CPHDKEY Vertex1;
    CPHDKEY Vertex2;
    CPHDKEY MaskedLow;
    CPHDKEY MaskedHigh;
    CPHDKEY DownsizedKey;

    IACA_VC_START();

    DownsizedKey = DOWNSIZE_KEY(Key);

    Vertex1 = Crc32u32(SEED1, RotateRight32(DownsizedKey, SEED3_BYTE1));
    Vertex1 = RotateLeft32(Vertex1, SEED3_BYTE2);

    Vertex2 = Crc32u32(SEED2, RotateLeft32(DownsizedKey, SEED3_BYTE3));
    Vertex2 = RotateRight32(Vertex2, SEED3_BYTE4);

    MaskedLow = Vertex1 & HASH_MASK;
    MaskedHigh = Vertex2 & HASH_MASK;

    Vertex1 = TABLE_DATA[MaskedLow];
    Vertex2 = TABLE_DATA[MaskedHigh];

    Index = (CPHINDEX)((Vertex1 + Vertex2) & INDEX_MASK);

    IACA_VC_END();

    return Index;
}

#endif