CENSUS ADDRESS PREPROCESSOR - WINDOWS 64-BIT

The GitHub Actions build creates CensusAddressPreprocessor.exe.

SUPPORTED INPUT FILES
1. Raw Census extracted street-data CSV files.
2. Legacy lookup CSV files ending in _Final.csv.
3. Current lookup CSV files ending in _Complete.csv.

HOW TO USE THE FINISHED PROGRAM
1. Put CensusAddressPreprocessor.exe and one input CSV in the same folder.
2. Double-click CensusAddressPreprocessor.exe. There is no file-selection window.
3. The program automatically finds and streams the single CSV.
4. The program takes the first five numeric characters in the input filename
   and writes exactly <five-digits>_Complete.csv.
5. Legacy _Final.csv and current _Complete.csv files are upgraded safely to
   the same required <five-digits>_Complete.csv naming standard.
6. The completed file has exactly 26 columns. The final three are:
   StreetCoreName, StreetTokenPrefixKey, and StreetTokenPhoneticKey.
7. Rejections, when present, are written as <five-digits>_Rejected.csv.
8. The input filename must contain at least five numeric characters.

RAW INPUT REQUIRED COLUMNS
ZIP_CODE
ZIP3
FULL_STREET_NAME
FROM_HOUSE_NUMBER
TO_HOUSE_NUMBER
RANGE_CENTROID_LATITUDE
RANGE_CENTROID_LONGITUDE

FINAL INPUT REQUIRED COLUMNS
RowID
ZIPCode
StreetName
LowHouseNumber
HighHouseNumber
CentroidLatitude
CentroidLongitude

FUZZY STREET KEYS
StreetCoreName removes a recognized directional or suffix only in its address
component position. StreetTokenPrefixKey stores the complete token when its
length is three characters or less and the first three characters otherwise.
StreetTokenPhoneticKey stores one Soundex code per core-street token.

HOUSE-NUMBER HANDLING
The complete normalized low/high endpoint text remains authoritative.
Numeric, hyphenated, alphanumeric, and fractional components are stored
separately. Unsafe or mixed formats keep the full text for exact normalized
lookup, and both primary-number fields remain blank/SQL NULL when a numeric
primary cannot be derived safely.

No installation, Visual Studio, PowerShell, or command prompt is required to
run the finished executable.

Leave only one CSV beside the executable. Generated _Rejected.csv and _Previous.csv safety files are ignored. Completely blank rows are ignored.
The original is preserved or restored if processing or replacement fails.
