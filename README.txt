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
1. Put CensusAddressPreprocessor.exe beside every CSV to process.
2. Source filenames must be a numeric prefix followed by
   _extracted_street_data.csv. One or two underscores are accepted.
3. Double-click CensusAddressPreprocessor.exe once.
4. The application processes every matching file in numeric-prefix order.
5. No Visual Studio, PowerShell, command prompt, or installation is required.
6. Each CSV is streamed and is not loaded completely into memory.
7. The numeric filename must contain at least five digits because the current
   output naming rule uses the first five numeric digits.
8. Example: 18089_extracted_street_data.csv becomes 18089_Complete.csv.
9. Each original source file is deleted only after its completed output is
   finalized successfully.
10. A failed source file is preserved, reported at the end, and does not stop
    the remaining matching files from being processed.
11. Existing #####_Complete.csv files are also processed and replaced in
    place, allowing earlier 33-column outputs to be corrected without the
    deleted raw source files.
12. Unrelated CSV files and generated _Rejected.csv files are ignored.

SUPPORTED CSV TYPES
1. Raw Census files named #####_extracted_street_data.csv.
2. Existing lookup files named #####_Complete.csv.

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

CSV SCOPE
Complete CSV output contains only the 26 lookup and normalization columns
listed above. ARID, TLID, SideIndicator, StateFIPS, CountyFIPS, SourceVintage,
and SegmentIdentityKey are intentionally excluded.

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
