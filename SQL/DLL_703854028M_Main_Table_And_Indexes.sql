USE [CHANGE_TO_PHALANX_DATABASE]
GO

IF OBJECT_ID(N'dbo.tblCensusAddressRangeGeocode', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.tblCensusAddressRangeGeocode
    (
        RowID bigint IDENTITY(1,1) NOT NULL,
        ZIPCode varchar(10) NOT NULL,
        StreetName nvarchar(200) NOT NULL,
        StreetSoundex varchar(4) NULL,
        StreetNumericKey varchar(20) NULL,
        StreetNumericClass char(1) NULL,
        PrefixDirectional varchar(2) NULL,
        StreetSuffix varchar(10) NULL,
        SuffixDirectional varchar(2) NULL,
        HouseNumberType tinyint NOT NULL,
        LowHouseNumber nvarchar(40) NOT NULL,
        HighHouseNumber nvarchar(40) NOT NULL,
        LowPrimaryNumber bigint NULL,
        HighPrimaryNumber bigint NULL,
        LowSecondaryNumber bigint NULL,
        HighSecondaryNumber bigint NULL,
        LowAlphaSuffix varchar(12) NULL,
        HighAlphaSuffix varchar(12) NULL,
        LowFractionValue decimal(18,6) NULL,
        HighFractionValue decimal(18,6) NULL,
        DerivedParity char(1) NULL,
        CentroidLatitude decimal(10,7) NOT NULL,
        CentroidLongitude decimal(11,7) NOT NULL,
        StreetCoreName nvarchar(200) NOT NULL,
        StreetTokenPrefixKey nvarchar(300) NULL,
        StreetTokenPhoneticKey nvarchar(300) NULL,

        CONSTRAINT PK_tblCensusAddressRangeGeocode
            PRIMARY KEY NONCLUSTERED HASH (RowID)
            WITH (BUCKET_COUNT = 16777216),

        INDEX UX_tblCensusAddressRangeGeocode_Segment
            UNIQUE NONCLUSTERED
            (
                ZIPCode,
                StreetName,
                LowHouseNumber,
                HighHouseNumber,
                CentroidLatitude,
                CentroidLongitude
            ),

        INDEX IX_tblCensusAddressRangeGeocode_Numeric
            NONCLUSTERED
            (
                ZIPCode,
                StreetNumericKey,
                LowPrimaryNumber,
                HighPrimaryNumber
            ),

        INDEX IX_tblCensusAddressRangeGeocode_Name
            NONCLUSTERED
            (
                ZIPCode,
                StreetName
            ),

        INDEX IX_tblCensusAddressRangeGeocode_Soundex
            NONCLUSTERED
            (
                ZIPCode,
                StreetSoundex
            )
    )
    WITH
    (
        MEMORY_OPTIMIZED = ON,
        DURABILITY = SCHEMA_AND_DATA
    );
END
GO
