USE [CHANGE_TO_PHALANX_DATABASE]
GO

IF OBJECT_ID(N'dbo.tblCensusAddressRangeGeocodeImport', N'U') IS NULL
BEGIN
    CREATE TABLE dbo.tblCensusAddressRangeGeocodeImport
    (
        RowID bigint NULL,
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
        LowFractionValue decimal(18, 6) NULL,
        HighFractionValue decimal(18, 6) NULL,
        DerivedParity char(1) NULL,
        CentroidLatitude decimal(10, 7) NOT NULL,
        CentroidLongitude decimal(11, 7) NOT NULL,
        StreetCoreName nvarchar(200) NOT NULL,
        StreetTokenPrefixKey nvarchar(300) NULL,
        StreetTokenPhoneticKey nvarchar(300) NULL
    );
END
GO

IF NOT EXISTS
(
    SELECT 1
    FROM sys.indexes
    WHERE object_id = OBJECT_ID(N'dbo.tblCensusAddressRangeGeocodeImport')
      AND name = N'UX_tblCensusAddressRangeGeocodeImport_Segment'
)
BEGIN
    CREATE UNIQUE NONCLUSTERED INDEX UX_tblCensusAddressRangeGeocodeImport_Segment
    ON dbo.tblCensusAddressRangeGeocodeImport
    (
        ZIPCode,
        StreetName,
        LowHouseNumber,
        HighHouseNumber,
        CentroidLatitude,
        CentroidLongitude
    )
    WITH (IGNORE_DUP_KEY = ON);
END
GO

IF NOT EXISTS
(
    SELECT 1
    FROM sys.indexes
    WHERE object_id = OBJECT_ID(N'dbo.tblCensusAddressRangeGeocode')
      AND name = N'UX_tblCensusAddressRangeGeocode_Segment'
)
BEGIN
    THROW 50001, 'Run DLL_703854028M_Main_Table_And_Indexes.sql before importing CSV files.', 1;
END
GO

CREATE OR ALTER PROCEDURE dbo.sp000001485015001
    @P001 nvarchar(4000)
AS ---Cirque

SET NOCOUNT ON

-- comment: sp_password

DECLARE @P002 nvarchar(max)
DECLARE @P003 int = 1

TRUNCATE TABLE dbo.tblCensusAddressRangeGeocodeImport

SET @P002 = N'BULK INSERT dbo.tblCensusAddressRangeGeocodeImport
FROM ' + QUOTENAME(@P001, '''') + N'
WITH
(
    FORMAT = ''CSV'',
    FIRSTROW = 2,
    FIELDQUOTE = ''"'',
    CODEPAGE = ''65001'',
    TABLOCK
);'

EXEC sys.sp_executesql @P002

WHILE @P003 > 0
BEGIN

    INSERT TOP (10000) dbo.tblCensusAddressRangeGeocode
    (
        ZIPCode,
        StreetName,
        StreetSoundex,
        StreetNumericKey,
        StreetNumericClass,
        PrefixDirectional,
        StreetSuffix,
        SuffixDirectional,
        HouseNumberType,
        LowHouseNumber,
        HighHouseNumber,
        LowPrimaryNumber,
        HighPrimaryNumber,
        LowSecondaryNumber,
        HighSecondaryNumber,
        LowAlphaSuffix,
        HighAlphaSuffix,
        LowFractionValue,
        HighFractionValue,
        DerivedParity,
        CentroidLatitude,
        CentroidLongitude,
        StreetCoreName,
        StreetTokenPrefixKey,
        StreetTokenPhoneticKey
    )
    SELECT TOP (10000)
        dbo.tblCensusAddressRangeGeocodeImport.ZIPCode,
        dbo.tblCensusAddressRangeGeocodeImport.StreetName,
        dbo.tblCensusAddressRangeGeocodeImport.StreetSoundex,
        dbo.tblCensusAddressRangeGeocodeImport.StreetNumericKey,
        dbo.tblCensusAddressRangeGeocodeImport.StreetNumericClass,
        dbo.tblCensusAddressRangeGeocodeImport.PrefixDirectional,
        dbo.tblCensusAddressRangeGeocodeImport.StreetSuffix,
        dbo.tblCensusAddressRangeGeocodeImport.SuffixDirectional,
        dbo.tblCensusAddressRangeGeocodeImport.HouseNumberType,
        dbo.tblCensusAddressRangeGeocodeImport.LowHouseNumber,
        dbo.tblCensusAddressRangeGeocodeImport.HighHouseNumber,
        dbo.tblCensusAddressRangeGeocodeImport.LowPrimaryNumber,
        dbo.tblCensusAddressRangeGeocodeImport.HighPrimaryNumber,
        dbo.tblCensusAddressRangeGeocodeImport.LowSecondaryNumber,
        dbo.tblCensusAddressRangeGeocodeImport.HighSecondaryNumber,
        dbo.tblCensusAddressRangeGeocodeImport.LowAlphaSuffix,
        dbo.tblCensusAddressRangeGeocodeImport.HighAlphaSuffix,
        dbo.tblCensusAddressRangeGeocodeImport.LowFractionValue,
        dbo.tblCensusAddressRangeGeocodeImport.HighFractionValue,
        dbo.tblCensusAddressRangeGeocodeImport.DerivedParity,
        dbo.tblCensusAddressRangeGeocodeImport.CentroidLatitude,
        dbo.tblCensusAddressRangeGeocodeImport.CentroidLongitude,
        dbo.tblCensusAddressRangeGeocodeImport.StreetCoreName,
        dbo.tblCensusAddressRangeGeocodeImport.StreetTokenPrefixKey,
        dbo.tblCensusAddressRangeGeocodeImport.StreetTokenPhoneticKey
    FROM dbo.tblCensusAddressRangeGeocodeImport
    WHERE NOT EXISTS
    (
        SELECT 1
        FROM dbo.tblCensusAddressRangeGeocode
        WHERE dbo.tblCensusAddressRangeGeocode.ZIPCode = dbo.tblCensusAddressRangeGeocodeImport.ZIPCode
          AND dbo.tblCensusAddressRangeGeocode.StreetName = dbo.tblCensusAddressRangeGeocodeImport.StreetName
          AND dbo.tblCensusAddressRangeGeocode.LowHouseNumber = dbo.tblCensusAddressRangeGeocodeImport.LowHouseNumber
          AND dbo.tblCensusAddressRangeGeocode.HighHouseNumber = dbo.tblCensusAddressRangeGeocodeImport.HighHouseNumber
          AND dbo.tblCensusAddressRangeGeocode.CentroidLatitude = dbo.tblCensusAddressRangeGeocodeImport.CentroidLatitude
          AND dbo.tblCensusAddressRangeGeocode.CentroidLongitude = dbo.tblCensusAddressRangeGeocodeImport.CentroidLongitude
    )

    SET @P003 = @@ROWCOUNT

    DELETE TOP (10000) dbo.tblCensusAddressRangeGeocodeImport
    WHERE EXISTS
    (
        SELECT 1
        FROM dbo.tblCensusAddressRangeGeocode
        WHERE dbo.tblCensusAddressRangeGeocode.ZIPCode = dbo.tblCensusAddressRangeGeocodeImport.ZIPCode
          AND dbo.tblCensusAddressRangeGeocode.StreetName = dbo.tblCensusAddressRangeGeocodeImport.StreetName
          AND dbo.tblCensusAddressRangeGeocode.LowHouseNumber = dbo.tblCensusAddressRangeGeocodeImport.LowHouseNumber
          AND dbo.tblCensusAddressRangeGeocode.HighHouseNumber = dbo.tblCensusAddressRangeGeocodeImport.HighHouseNumber
          AND dbo.tblCensusAddressRangeGeocode.CentroidLatitude = dbo.tblCensusAddressRangeGeocodeImport.CentroidLatitude
          AND dbo.tblCensusAddressRangeGeocode.CentroidLongitude = dbo.tblCensusAddressRangeGeocodeImport.CentroidLongitude
    )

END
GO
