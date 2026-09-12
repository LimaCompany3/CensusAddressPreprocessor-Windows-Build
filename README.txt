CENSUS ADDRESS PREPROCESSOR - WINDOWS 64-BIT

The GitHub Actions build creates CensusAddressPreprocessor.exe.

HOW TO USE THE FINISHED PROGRAM
1. Put CensusAddressPreprocessor.exe and one input CSV in the same folder.
2. Double-click CensusAddressPreprocessor.exe. There is no file-selection window.
3. The program automatically finds and streams the CSV.
4. After successful processing, it deletes the original input CSV and creates:
   - <original-name>_Final.csv: exactly 23 lookup-table columns.
   - <original-name>_Rejected.csv: created only after the first rejected row;
     the file never exists when every row succeeds.

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

The program ignores previously generated _Final.csv and _Rejected.csv files.
If no eligible input CSV or more than one eligible input CSV is present, it
shows a clear message and does not process the wrong file.

For safety, the original input CSV is preserved if processing or output writing
fails. When processing succeeds with no rejected rows, only the _Final.csv file
remains beside the executable.
