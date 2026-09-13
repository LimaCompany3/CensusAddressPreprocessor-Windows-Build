USE [CHANGE_TO_PHALANX_DATABASE]
GO

CREATE OR ALTER PROCEDURE dbo.sp000001485015002
    @P001 varchar(10),
    @P002 nvarchar(200),
    @P003 nvarchar(200),
    @P004 varchar(20),
    @P005 varchar(4),
    @P006 nvarchar(300),
    @P007 nvarchar(300),
    @P008 varchar(2),
    @P009 varchar(10),
    @P010 varchar(2),
    @P011 nvarchar(40),
    @P012 bigint
AS ---Cirque

SET NOCOUNT ON

-- comment: sp_password

DECLARE @P013 int
DECLARE @P014 int
DECLARE @P015 bigint

DECLARE @P016 TABLE
(
    RowID bigint NOT NULL,
    MatchScore int NOT NULL
)

INSERT INTO @P016
(
    RowID,
    MatchScore
)
SELECT
    dbo.tblCensusAddressRangeGeocode.RowID,
    CASE
        WHEN dbo.tblCensusAddressRangeGeocode.StreetName = @P002 THEN 0
        WHEN dbo.tblCensusAddressRangeGeocode.StreetCoreName = @P003 THEN 100
        WHEN @P004 IS NOT NULL AND dbo.tblCensusAddressRangeGeocode.StreetNumericKey = @P004 THEN 200
        WHEN @P006 IS NOT NULL AND dbo.tblCensusAddressRangeGeocode.StreetTokenPrefixKey = @P006 THEN 300
        WHEN @P007 IS NOT NULL AND dbo.tblCensusAddressRangeGeocode.StreetTokenPhoneticKey = @P007 THEN 400
        WHEN @P005 IS NOT NULL AND dbo.tblCensusAddressRangeGeocode.StreetSoundex = @P005 THEN 500
        ELSE 900
    END
    +
    CASE WHEN ISNULL(dbo.tblCensusAddressRangeGeocode.PrefixDirectional, '') = ISNULL(@P008, '') THEN 0 ELSE 10 END
    +
    CASE WHEN ISNULL(dbo.tblCensusAddressRangeGeocode.StreetSuffix, '') = ISNULL(@P009, '') THEN 0 ELSE 5 END
    +
    CASE WHEN ISNULL(dbo.tblCensusAddressRangeGeocode.SuffixDirectional, '') = ISNULL(@P010, '') THEN 0 ELSE 10 END
    +
    CASE
        WHEN @P012 IS NOT NULL
         AND dbo.tblCensusAddressRangeGeocode.LowPrimaryNumber IS NOT NULL
         AND dbo.tblCensusAddressRangeGeocode.HighPrimaryNumber IS NOT NULL
         AND @P012 >= dbo.tblCensusAddressRangeGeocode.LowPrimaryNumber
         AND @P012 <= dbo.tblCensusAddressRangeGeocode.HighPrimaryNumber
         AND
         (
             dbo.tblCensusAddressRangeGeocode.DerivedParity = 'B'
             OR dbo.tblCensusAddressRangeGeocode.DerivedParity =
                CASE WHEN @P012 % 2 = 0 THEN 'E' ELSE 'O' END
         )
        THEN 0
        WHEN @P012 IS NOT NULL
         AND dbo.tblCensusAddressRangeGeocode.LowPrimaryNumber IS NOT NULL
         AND dbo.tblCensusAddressRangeGeocode.HighPrimaryNumber IS NOT NULL
        THEN 1000 +
             CASE
                 WHEN @P012 < dbo.tblCensusAddressRangeGeocode.LowPrimaryNumber
                 THEN CONVERT(int, dbo.tblCensusAddressRangeGeocode.LowPrimaryNumber - @P012)
                 WHEN @P012 > dbo.tblCensusAddressRangeGeocode.HighPrimaryNumber
                 THEN CONVERT(int, @P012 - dbo.tblCensusAddressRangeGeocode.HighPrimaryNumber)
                 ELSE 500
             END
        WHEN @P011 = dbo.tblCensusAddressRangeGeocode.LowHouseNumber
          OR @P011 = dbo.tblCensusAddressRangeGeocode.HighHouseNumber
        THEN 0
        ELSE 100000
    END
FROM dbo.tblCensusAddressRangeGeocode
WHERE dbo.tblCensusAddressRangeGeocode.ZIPCode = @P001
  AND
  (
      dbo.tblCensusAddressRangeGeocode.StreetName = @P002
      OR dbo.tblCensusAddressRangeGeocode.StreetCoreName = @P003
      OR (@P004 IS NOT NULL AND dbo.tblCensusAddressRangeGeocode.StreetNumericKey = @P004)
      OR (@P006 IS NOT NULL AND dbo.tblCensusAddressRangeGeocode.StreetTokenPrefixKey = @P006)
      OR (@P007 IS NOT NULL AND dbo.tblCensusAddressRangeGeocode.StreetTokenPhoneticKey = @P007)
      OR (@P005 IS NOT NULL AND dbo.tblCensusAddressRangeGeocode.StreetSoundex = @P005)
  )

SELECT @P013 = MIN(MatchScore)
FROM @P016
WHERE MatchScore < 100000

IF @P013 IS NULL
BEGIN
    SELECT
        CAST('NOT_FOUND' AS varchar(20)) AS MatchOutcome,
        CAST(NULL AS bigint) AS AddressRangeID,
        CAST(NULL AS decimal(10,7)) AS Latitude,
        CAST(NULL AS decimal(11,7)) AS Longitude,
        CAST(NULL AS int) AS MatchScore
END
ELSE
BEGIN
    SELECT @P014 = COUNT(*)
    FROM @P016
    WHERE MatchScore = @P013

    IF @P014 > 1
    BEGIN
        SELECT
            CAST('AMBIGUOUS' AS varchar(20)) AS MatchOutcome,
            CAST(NULL AS bigint) AS AddressRangeID,
            CAST(NULL AS decimal(10,7)) AS Latitude,
            CAST(NULL AS decimal(11,7)) AS Longitude,
            @P013 AS MatchScore
    END
    ELSE
    BEGIN
        SELECT @P015 = RowID
        FROM @P016
        WHERE MatchScore = @P013

        SELECT
            CASE WHEN @P013 < 1000 THEN CAST('EXACT' AS varchar(20)) ELSE CAST('NEAREST' AS varchar(20)) END AS MatchOutcome,
            dbo.tblCensusAddressRangeGeocode.RowID AS AddressRangeID,
            dbo.tblCensusAddressRangeGeocode.CentroidLatitude AS Latitude,
            dbo.tblCensusAddressRangeGeocode.CentroidLongitude AS Longitude,
            @P013 AS MatchScore
        FROM dbo.tblCensusAddressRangeGeocode
        WHERE dbo.tblCensusAddressRangeGeocode.RowID = @P015
    END
END
GO
