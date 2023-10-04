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

extern int verbose;  // global variable to be set in the file that includes this header

// define verbosity levels:
#define VERB_QUIET  0       // only output result or fatal errors
#define VERB_PROG   1       // + show progress
#define VERB_GLOB   2       // + more detailed messages
#define VERB_ALL    3       // + messages with very much detail

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

void print_hex(int v, unsigned char* bin, unsigned int n);
void print_bin(int v, uint64_t printme, int n);

#endif