# ETCS Subset 36 balise encoding and decoding
Encode and decode Eurobalise contents as described in ETCS subset 36 (FFFIS for Eurobalise, v3.1.0, Dec 17th 2015)

Copyright 2023, Fokke Bronsema, fokke@bronsema.net, version 1, February/March 2023
Disclaimer: use at your own risk, the author is not responsible for incorrect en-/decoded messages leading to train related mayhem.

Sources:
* https://www.era.europa.eu/system/files/2023-01/sos3_index009_-_subset-036_v310.pdf (paragraph 4.3.2)
* https://lemmatalogic.com/lfsrscrambler/
* https://www.moria.us/articles/demystifying-the-lfsr/
* Towards Modeling and Verification of Eurobalise Telegram Encoding Algorithm, Sergey Staroletov, https://www.sciencedirect.com/science/article/pii/S2352146523000728

Usage: compile ss36_test.c. This yields an executable for win64 with the following command line parameters:

-i, --input_filename <STRING>:        Read lines with data from the indicated
                                      file (UTF-8, no BOM) and convert its
                                      contents from shaped data to unshaped data
                                      and vice versa. SS39 automatically
                                      determines the used format (base64/hex)
                                      and length (short/long). Lines must be
                                      separated by '\n' ('\r' will be ignored).
                                      If both the shaped and unshaped data are
                                      given on one line (separated by a comma),
                                      SS39 will check the correct shaping.
                                      Comments must be preceded by '#'.
 
 -o, --output_filename <STRING>:      Write output to this file.
 
 -s, --string         <STRING>:        Input string (shaped and/or deshaped
                                      string in base64/hex), format identical to
                                      one line in the input file.
 
 -v, --verbose        <INT>:           Level of verbosity: 0 (quiet, only show
                                      result), 1 (+show progress, default), 2
                                      (+basic output) or 3 (+lots of output).
 
 -m, --multithread    <BOOL>:          Force multithreading at verbosity > 1
                                      (this will lead to garbled output of
                                      different processes interrupting each
                                      other)
 
 -f, --format_output  <STRING>:        Output format for the shaped telegram:
                                      'hex' or 'base64' (default).
                                      
# Global description, libraries
The library in ss36.c/h contains definitions and methods that can be used to encode and decode Eurobalise contents. Please read Subset 39 for more information and a mathematical background. The ss36-library uses another library, longnum.c/h, which deals with low level bit manipulation of long numbers (balise contents can exist of up to 1023 bits). GF2 contains binary Galois Field functions used by the ss36 library for shaping and deshaping the balise contents. The opt-library (see https://public.lanl.gov/jt/Software/) is used for parsing the command line.
