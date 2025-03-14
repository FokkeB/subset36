## ETCS Subset 36 balise encoding and decoding ("balise_codec")

Encode and decode Eurobalise contents as described in ETCS subset 36 (FFFIS for Eurobalise, v3.1.0, Dec 17th 2015)

Copyright 2023/2024, Fokke Bronsema, fokke@bronsema.net, version 5, March 2025.
Disclaimer: use this software at your own risk, the author is not responsible for incorrect en-/decoded messages leading to train related mayhem. Even though this software was tested against a few thousand Dutch Eurobalises from different manufacturers, errors may still occur.

Sources:
- https://www.era.europa.eu/system/files/2023-01/sos3_index009_-_subset-036_v310.pdf (paragraph 4.3.2)
- https://lemmatalogic.com/lfsrscrambler/
- https://www.moria.us/articles/demystifying-the-lfsr/
- Towards Modeling and Verification of Eurobalise Telegram Encoding Algorithm, Sergey Staroletov, https://www.sciencedirect.com/science/article/pii/S2352146523000728

This repository contains the following software:
1. "ss36": this folder contains the main library of this repository. It is used by "main.cpp" to make an executable (see below);
2. "tester": this program is created to test various functions of the library;
3. "py_balise_codec": a Python-module to be able to use the library in Python natively, including a demo file called "balise_codec_demo.py";

### SS36
This contains the main library and files to create an executable.
Usage: compile main.cpp. This yields a command line executable (64-bit Windows executable is included in the folder) with the following command line parameters:

- -i, --input_filename: read lines with data from the indicated file (UTF-8, no BOM) and convert its contents from shaped data to unshaped data and vice versa. This tool automatically determines the used format (base64/hex) and length (short/long). Lines must be separated by '\n' ('\r' will be ignored). If both the shaped and unshaped data are given on one line (separated by a semicolon), the program will check the correct shaping. Comments must be preceded by '#'.
- -o, --output_filename: write output to this file.
- -s, --string: input string literal (shaped and/or deshaped string in base64/hex), format identical to one line in the input file.
- -v, --verbose: level of verbosity: 0 (quiet, only show result), 1 (+show progress, default), 2 (+basic output) or 3 (+lots of output).
- -m, --max_cpu: max nr of cpu's to use. Multithreading is enabled by default for verbosity <= 1 or if set to 0.
- -l, --force_long: force shaping to the long format (1023 bits), even if the unshaped data is of short format (341 bits). If not specified, this tool will use the same format as the input data.
- -f, --format_output: output format for the shaped telegram: 'hex' or 'base64' (default).
- -e, --show_error_codes: shows the meaning of the error codes that can be generated when checking / shaping telegrams.
- -E, --error_only: output only the telegrams in which an error was found (-e gives the error codes).
- -a, --all: calculate all possible output telegrams for each input telegram. If not specified, calculations will stop at the first output telegram. Currently only works with option -m1.
 
For example: 

    balise_codec.exe -i dummy_input.csv -o dummy_output.csv -f hex -v1

### Python module
The Python module "balise_codec.pyd" (a compiled version is included, tested with Python 3.11.5) can be used to convert balise contents in Python natively.

The function 'convert' receives unshaped and/or shaped balise data from Python in a string of lines. Each line is terminated by a newline ('\n') - character and contains one of the following fields, or both (separated by a semicolon) :
- unshaped hex data (output of balise design process)
- shaped data (either hex or base64, either short or long)

If only one field is given, the module will calculate the other field. If both input fields are given, the module will verify the correctness (is the shaped data correct? do the two fields match? etc). Comments are preceded with a '#', to allow loading balise data from files with comments. See included example ("dummy_input.csv"). The module returns to Python another string containing one of these lines for each line in the input data:

    <unshaped hex short/long>;<shaped hex/base64 short/long>;<errorcode>\n

The definition of the error codes (0=OK) is given below.

### Error codes

The error codes below can be generated when checking / shaping a telegram. See SUBSET-036 paragraph 4.3 for more details concerning error codes >= 10.

Error code Explanation
-        0       No error
-        1       A logical error (not further specified)
-        2       Error during memory allocation
-        3       Error in input
-        20      No input specified
-        21      Error creating output file
-        22      Error spawning sub process
-        100     Alphabet condition fails
-        101     Off-sync parsing condition fails
-        102     Aperiodicity condition fails
-        103     Undersampling check fails
-        104     Control bits check fails
-        105     Check bits check fails
-        106     Overflow of SB and ESB (should never occur, please contact author if it did)
-        107     Error during conversion from 10 bits to 11 bits (11-bit value not found in list of transformation words)
-        108     Shaped contents do not match the unshaped contents (encoding error)

### About
The library in telegram.c/h and the related #includes contain definitions and methods that can be used to encode and decode Eurobalise contents. Please read Subset 36 for more information and a mathematical background. The ss36-library uses another library, longnum.c/h, which deals with low level bit manipulation of long numbers (balise contents can exist of up to 1023 bits), amongst which two Galois Field functions used by the ss36 library for shaping and deshaping the balise contents. The library CLI11 (see https://github.com/CLIUtils/CLI11) is used for parsing the command line. Balise_codec was developed in MS Visual Studio 2022 by fokke@bronsema.net.
