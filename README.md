# ETCS Subset 36 balise encoding and decoding ("balise_codec")
Encode and decode Eurobalise contents as described in ETCS subset 36 (FFFIS for Eurobalise, v3.1.0, Dec 17th 2015)

Copyright 2023, Fokke Bronsema, fokke@bronsema.net, version 3, October 4th 2023
Disclaimer: use at your own risk, the author is not responsible for incorrect en-/decoded messages leading to train related mayhem.

Sources:
* https://www.era.europa.eu/system/files/2023-01/sos3_index009_-_subset-036_v310.pdf (paragraph 4.3.2)
* https://lemmatalogic.com/lfsrscrambler/
* https://www.moria.us/articles/demystifying-the-lfsr/
* Towards Modeling and Verification of Eurobalise Telegram Encoding Algorithm, Sergey Staroletov, https://www.sciencedirect.com/science/article/pii/S2352146523000728

Usage: compile balise_codec.c. This yields a command line executable (64-bit executable is included in repository) with the following command line parameters:

 -i, --input_filename <STRING>        Read lines with data from the indicated
                                      file (UTF-8, no BOM) and convert its
                                      contents from shaped data to unshaped data
                                      and vice versa. This tool automatically
                                      determines the used format (base64/hex)
                                      and length (short/long). Lines must be
                                      separated by '\n' ('\r' will be ignored).
                                      If both the shaped and unshaped data are
                                      given on one line (separated by a comma),
                                      SS39 will check the correct shaping.
                                      Comments must be preceded by '#'.
 -o, --output_filename <STRING>       Write output to this file.
 -s, --string         <STRING>        Input string (shaped and/or deshaped
                                      string in base64/hex), format identical to
                                      one line in the input file.
 -v, --verbose        <INT>           Level of verbosity: 0 (quiet, only show
                                      result), 1 (+show progress, default), 2
                                      (+basic output) or 3 (+lots of output).
 -m, --max_cpu        <INT>           Max nr of cpu's to use. Multithreading is
                                      enabled by default for verbosity <= 1 or
                                      if set to 0.
 -l, --force_long     <BOOL>          Force shaping to the long format (1023
                                      bits), even if the unshaped data is of
                                      short format (341 bits). If not specified,
                                      this tool will use the same format as the
                                      input data.
 -f, --format_output  <STRING>        Output format for the shaped telegram:
                                      'hex' or 'base64' (default).
 -e, --show_error_codes <BOOL>        Shows the meaning of the error codes that
                                      can be generated when checking / shaping
                                      telegrams.
 -E, --error_only     <BOOL>          Output only the telegrams in which an
                                      error was found (-e gives the error
                                      codes).
 -T, --run_tests      <BOOL>          Run various tests to check the workings of
                                      this program, using input from the input
                                      file. This tool uses lines from the input
                                      files containing both decoded and encoded
                                      contents.
                                      
For example: balise_codec.exe -i dummy_input.csv -o dummy_output.csv -f hex -v1

# Error codes
The error codes below can be generated when checking / shaping a telegram.
See SUBSET-036 paragraph 4.3 for more details concerning error codes >= 10.
Error code      Explanation
        0       No error
        1       No input specified
        2       A logical error (not further specified)
        3       Error creating output file
        4       Error during memory allocation
        10      Alphabet condition fails
        11      Off-sync parsing condition fails
        12      Aperiodicity condition fails
        13      Undersampling check fails
        14      Control bits check fails
        15      Check bits check fails
        16      Overflow of SB and ESB (should never occur, please contact author if it did)
        17      Error during conversion from 10 bits to 11 bits (11-bit value not found in list of transformation words)
        18      Shaped contents do not match the unshaped contents (encoding error)    
		
# Global description, libraries
The library in ss36.c/h contains definitions and methods that can be used to encode and decode Eurobalise contents. 
Please read Subset 39 for more information and a mathematical background. 
The ss36-library uses another library, longnum.c/h, which deals with low level bit manipulation of long numbers (balise contents can exist of up to 1023 bits). 
GF2 contains binary Galois Field functions used by the ss36 library for shaping and deshaping the balise contents. 
The opt-library (see https://public.lanl.gov/jt/Software/, included in a zip-file) is used for parsing the command line.
