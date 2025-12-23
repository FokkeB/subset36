/**
* This file is part of "balise_codec".
* balise_codec is free software: you can distribute it and/or modify it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* See the GNU Lesser General Public License for more details.
* You should have received a copy of the GNU General Public License along with this program.
* If not, see < https://www.gnu.org/licenses/>.
*/

#ifndef USEFUL_FUNCTIONS_H
#define USEFUL_FUNCTIONS_H

#include <stdio.h>
#include <stdint.h>
#include <list>
#include <string>
using namespace std;

// define verbosity levels:   
#define VERB_QUIET  0       // only output result or fatal errors
#define VERB_PROG   1       // + show progress
#define VERB_FLOW   2       // + program flow
#define VERB_GLOB   3       // + global information of the calculations
#define VERB_ALL    4       // + messages with very much detail

extern int verbose;

// prints the output only if the verbosity level (set in global var "verbose") allows it
#define eprintf(level, ...) do {            \
    if (level<=verbose)                     \
        fprintf (stderr, __VA_ARGS__);      \
    } while (0)

// exits the current function if the requested verbosity level is too large. To be used in functions that do nothing else than printing.
#define return_if_silent(level) do {        \
    if (level>verbose)                      \
        return;                             \
    } while (0)

bool check_verbose(int v);
// returns true if v <= current verbosity level

void print_hex(int v, unsigned char* bin, unsigned int n);
void print_bin(int v, uint64_t printme, int n);
signed int hex_to_bin(string hexstr, uint8_t* binstr);

// base64-functions below copied from another library and adjusted for this program
//Base64 char table function - used internally for decoding
// returns the int-value that corresponds to the input char, or -1 if the input char is invalid
signed int b64_int(unsigned int ch);

// in : buffer of "raw" binary to be encoded.
// in_len : number of bytes to be encoded.
// out : pointer to buffer with enough memory, user is responsible for memory allocation, receives null-terminated string
// returns size of output including null byte or -1 if illegal chars in input buffer
unsigned int b64_encode(const uint8_t* in, unsigned int in_len, string& out);

// in : string to be decoded.
// out : pointer to buffer with enough memory, user is responsible for memory allocation, receives "raw" binary
// returns size of output excluding null byte
signed int b64_decode(string in, uint8_t* out);

#endif