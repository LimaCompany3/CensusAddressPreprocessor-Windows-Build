CENSUS ADDRESS PREPROCESSOR - WINDOWS 64-BIT

The GitHub Actions build creates CensusAddressPreprocessor.exe.

SUPPORTED INPUT FILES
1. Raw Census files named *_extracted_street_data.csv.
2. Existing lookup files named *_extracted_street_data_Final.csv.

HOW TO USE THE FINISHED PROGRAM
1. Put CensusAddressPreprocessor.exe and one input CSV in the same folder.
2. Double-click CensusAddressPreprocessor.exe. There is no file-selection window.
3. The program automatically finds and streams the single CSV.
4. Raw input is converted to <original-name>_Final.csv.
5. Existing _Final.csv input is upgraded in place; it is not renamed
   _Final_Final.csv.
6. The completed final file has exactly 26 columns. The final three are:
   StreetCoreName, StreetTokenPrefixKey, and StreetTokenPhoneticKey.
7. A *_Rejected.csv file is created only after the first rejected row.

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

Leave only one CSV beside the executable. Generated _Rejected.csv and
_Previous.csv safety files are ignored. Completely blank rows are ignored.
The original is preserved or restored if processing or replacement fails.
