CENSUS ADDRESS PREPROCESSOR - WINDOWS 64-BIT

The GitHub Actions build creates CensusAddressPreprocessor.exe.

HOW TO USE THE FINISHED PROGRAM
1. Double-click CensusAddressPreprocessor.exe.
2. Select one or more Census CSV files in the file-picker window.
3. The program streams each file and leaves the original unchanged.
4. Beside each input file it creates:
   - <original-name>_Final.csv: exactly 23 lookup-table columns.
   - <original-name>_Rejected.csv: rejected rows and the reason.

REQUIRED INPUT COLUMNS
ZIP_CODE
ZIP3
FULL_STREET_NAME
FROM_HOUSE_NUMBER
TO_HOUSE_NUMBER
RANGE_CENTROID_LATITUDE
RANGE_CENTROID_LONGITUDE

HOUSE-NUMBER HANDLING
The complete normalized low/high endpoint text remains authoritative.
Numeric, hyphenated, alphanumeric, and fractional components are stored
separately. Unsafe or mixed formats use HouseNumberType 5, keeping the full
text intact for exact normalized-text lookup. Blank derived fields represent
SQL NULL and are never written as zero.

No installation, Visual Studio, PowerShell, or command prompt is required to
run the finished executable.
