DLL_703854028M COMPLETE ADDRESS LOOKUP PACKAGE
SQL SERVER 2025 / WINDOWS 64-BIT

CANONICAL NAMES
DLL: DLL_703854028M.dll
Windows application: CensusAddressPreprocessor.exe
SQL CLR wrapper: dbo.sp000001485015000
CSV import procedure: dbo.sp000001485015001
Best-segment lookup: dbo.sp000001485015002
Main table: dbo.tblCensusAddressRangeGeocode
Completed CSV: first five numeric characters + _Complete.csv
Rejected CSV: first five numeric characters + _Rejected.csv

WINDOWS APPLICATION
1. Put CensusAddressPreprocessor.exe beside one CSV.
2. Double-click CensusAddressPreprocessor.exe.
3. No Visual Studio, PowerShell, command prompt, or installation is required.
4. The application streams the CSV and does not load the whole file into memory.
5. The input filename must contain at least five numeric characters.
6. Example: 18089_extracted_street_data.csv becomes 18089_Complete.csv.
7. The original raw file is deleted only after the completed output is finalized.
8. Legacy _Final.csv and current _Complete.csv files are safely upgraded to the
   canonical five-digit _Complete.csv name.

SUPPORTED CSV TYPES
1. Raw Census extracted street-data CSV.
2. Legacy lookup CSV ending in _Final.csv.
3. Current lookup CSV ending in _Complete.csv.

COMPLETE CSV COLUMN ORDER
RowID
ZIPCode
StreetName
StreetSoundex
StreetNumericKey
StreetNumericClass
PrefixDirectional
StreetSuffix
SuffixDirectional
HouseNumberType
LowHouseNumber
HighHouseNumber
LowPrimaryNumber
HighPrimaryNumber
LowSecondaryNumber
HighSecondaryNumber
LowAlphaSuffix
HighAlphaSuffix
LowFractionValue
HighFractionValue
DerivedParity
CentroidLatitude
CentroidLongitude
StreetCoreName
StreetTokenPrefixKey
StreetTokenPhoneticKey
ARID
TLID
SideIndicator
StateFIPS
CountyFIPS
SourceVintage
SegmentIdentityKey

HOUSE-NUMBER RULES
The complete normalized low/high endpoint text is authoritative.
Type 1: numeric.
Type 2: hyphenated.
Type 3: alphanumeric.
Type 4: fractional.
Type 5: complex, mixed, or not safely decomposable.
Unavailable derived components remain blank in CSV and SQL NULL in tables.
They are never converted to zero.
Nearest-house-number fallback is allowed only when numeric components are
safely comparable. Complex Type 5 values require exact normalized text.

STREET LOOKUP ORDER
1. Exact normalized full street.
2. Exact core street with directional/suffix penalties.
3. Numeric street key.
4. Per-token three-character prefix key.
5. Per-token phonetic key.
6. Whole-street Soundex fallback.

Directional and suffix mismatches reduce candidate rank but do not
automatically eliminate an otherwise valid street candidate.

DETERMINISTIC BEST-SEGMENT RULE
Every candidate receives the same fixed score for the same input.
Lower score is better. Street-match quality is scored first, followed by
directional/suffix penalties and house-range quality.
A unique lowest score returns EXACT or NEAREST.
Two or more candidates sharing the lowest score return AMBIGUOUS with no
latitude, longitude, or AddressRangeID.
A near-tie with a score gap of 25 or less returns REVIEW with no coordinates.
Every result also exposes CandidateCount, BestScore, SecondBestScore, ScoreGap,
and MatchConfidence.
No eligible candidate returns NOT_FOUND.
RowID is used only as the stored identifier and never as a hidden tie-breaker.

DUPLICATE PREVENTION
SegmentIdentityKey is created deterministically:
1. SourceVintage + ARID when ARID is present.
2. SourceVintage + TLID + SideIndicator + ZIPCode + authoritative endpoints
   when ARID is absent.
3. ZIPCode + StreetName + authoritative endpoints + centroid coordinates only
   as the legacy fallback.

RowID and derived fuzzy fields are excluded from segment identity.
The import staging table removes duplicate rows within one CSV.
The import procedure uses NOT EXISTS against SegmentIdentityKey.
The main table unique index prevents concurrent imports from inserting the
same segment twice.

SQL INSTALLATION ORDER
1. Run SQL\DLL_703854028M_Main_Table_And_Indexes.sql.
2. Copy DLL_703854028M.dll to C:\Phalanx\DLL_703854028M.dll.
3. Run SQL\DLL_703854028M_Install_SQLCLR_SQL_Server_2025.sql.
4. Run SQL\DLL_703854028M_Import_Complete_CSV_No_Duplicates.sql.
5. Run SQL\DLL_703854028M_Deterministic_Best_Segment_Lookup.sql.
6. Change CHANGE_TO_PHALANX_DATABASE before executing each SQL file.
7. SQL Server's service account must be able to read the CSV path supplied to
   dbo.sp000001485015001.

INDEXES
PK_tblCensusAddressRangeGeocode
UX_tblCensusAddressRangeGeocode_Segment
IX_tblCensusAddressRangeGeocode_Numeric
IX_tblCensusAddressRangeGeocode_Name
IX_tblCensusAddressRangeGeocode_Soundex

The RowID hash primary key uses BUCKET_COUNT = 16777216.


SILENT DLL CONTRACT
DLL_703854028M.dll never prints, raises, or returns diagnostic messages.
It has no console, dialog, RAISERROR, THROW, or logging output.
On every caught internal failure, every DLL output parameter is reset to SQL
NULL and control returns silently. MatchOutcome values are produced by the SQL
lookup procedure as data; they are not DLL messages.
