## ETCS Subset 36 balise encoding and decoding ("balise_codec")

Encode and decode Eurobalise contents as described in ETCS subset 36 (FFFIS for Eurobalise, v4.0.0, 05/07/2023)
By Fokke Bronsema, fokke@bronsema.net, version 9, December 23rd, 2025, released under the GNU Lesser General Public License.

### Changes since version 8: 
- Added support for Linux (x86, ARM);
- Uses BS::threadpool (see https://github.com/bshoshany/thread-pool);
- Improved python module (thanks @ https://github.com/Pierretsc)
 
### Disclaimer 
Use this software at your own risk, the author is not responsible for incorrect en-/decoded messages leading to train related mayhem. Even though this software was tested against a few thousand Dutch Eurobalises from different manufacturers, errors may still occur.

### Sources
- https://www.era.europa.eu/system/files/2023-09/index009_-_SUBSET-036_v400.pdf (paragraph 4.3.2)
- https://lemmatalogic.com/lfsrscrambler/
- https://www.moria.us/articles/demystifying-the-lfsr/
- Towards Modeling and Verification of Eurobalise Telegram Encoding Algorithm, Sergey Staroletov, https://www.sciencedirect.com/science/article/pii/S2352146523000728
- Optimisations: Optimization and Implementation of Balise Telegram Encoding Algorithm_English.pdf (see folder inputdocs)

### Contents
This repository contains the following software:
1. "ss36": this folder contains the main library of this repository. 
2. "balise_codec": a command line executable that uses the ss36-library;
3. "tester": this program is created to test various functions of the library;
4. "py_balise_codec": a Python-module to be able to use the library in Python natively;
5. "py_balise": an example implementation using the python library.

### balise_codec
This folder contains main.cpp which uses the ss36-library to create an executable.
Usage: compile main.cpp and its required dependencies. This yields a command line executable (compiled version for different platforms are included in the folder) with the following command line parameters:

- -i, --input_filename: read lines with data from the indicated file (UTF-8, no BOM) and convert its contents from shaped data to unshaped data and vice versa. This tool automatically determines the used format (base64/hex) and length (short/long). Lines must be separated by '\n' ('\r' will be ignored). If both the shaped and unshaped data are given on one line (separated by a semicolon), the program will check the correct shaping. Comments must be preceded by '#'.
- -o, --output_filename: write output to this file.
- -s, --string: input string literal (shaped and/or deshaped string in base64/hex), format identical to one line in the input file.
- -v, --verbose: level of verbosity: 0 (quiet, only show result), 1 (+show progress, default), 2 (+basic output) or 3 (+lots of output).
- -m, --max_cpu: max nr of cpu's to use. Multithreading is enabled by default for verbosity <= 1 or if set to 0.
- -l, --force_long: force shaping to the long format (1023 bits), even if the unshaped data is of short format (341 bits). If not specified, this tool will use the same format as the input data.
- -f, --format_output: output format for the shaped telegram: 'hex' or 'base64' (default).
- -e, --show_error_codes: shows the meaning of the error codes that can be generated when checking / shaping telegrams.
- -E, --error_only: output only the telegrams in which an error was found (-e gives the error codes).
- -a, --calc_all: calculate all valid combinations of scrambling bits (SB) and extra shaping bits (ESB) for each telegram in the input. The output will contain the SB and ESB in two extra columns, as well as the 9th and 10th 11-bit word of each telegram. 

For example: 

    balise_codec.exe -i dummy_input.csv -o dummy_output.csv -f hex -v1

### Python module
The Python module (compiled versions are included) can be used to convert balise contents in Python natively. See the example code for more information.

### Error codes
The error codes below can be generated when checking / shaping a telegram. See SUBSET-036 paragraph 4.3 for more details concerning error codes >= 10.

Error code Explanation
- 0       No error
- 1       No input specified
- 2       A logical error (not further specified)
- 3       Error creating output file
- 4       Error during memory allocation
- 5       Error in the input data (wrong size, illegal chars, ...)
- 6       Error creating calculation thread or acquiring mutex
- 10      Alphabet condition fails
- 11      Off-sync parsing condition fails
- 12      Aperiodicity condition fails
- 13      Undersampling check fails
- 14      Control bits check fails
- 15      Check bits check fails
- 16      Overflow of SB and ESB (should never occur, please contact author if it did)
- 17      Error during conversion from 10 bits to 11 bits (11-bit value not found in list of transformation words)
- 18      Shaped contents do not match the unshaped contents (encoding error)

### About and credits
The ss36-library contain definitions and methods that can be used to encode and decode Eurobalise contents. Please read Subset 36 for more information and a mathematical background. The ss36-library uses another library, longnum.c/h, which deals with low level bit manipulation of long numbers (balise contents can exist of up to 1023 bits), amongst which two Galois Field functions used by the ss36 library for shaping and deshaping the balise contents. The library CLI11 (see https://github.com/CLIUtils/CLI11) is used for parsing the command line, library https://github.com/bshoshany/thread-pool is used to manage multithreading. Balise_codec was developed in MS Visual Studio 2026 by fokke@bronsema.net. 
