/**
* This file is part of SS36.
* SS36 is free software: you can distribute it and/or modify it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* See the GNU Lesser General Public License for more details.
* You should have received a copy of the GNU General Public License along with this program.
* If not, see < https://www.gnu.org/licenses/>.
*/

/**
 * longnum - implement a virtual long int of many bits using an array of ints
 * 
 * Implemented by fokke@bronsema.net
 * To be used for calculation of encoded Eurobalise contents
 * Feb/March 2023 
 * 
*/
#ifndef LONGNUM_H
#define LONGNUM_H

#include "colors.h"
#include <stdint.h>
#include "useful_functions.h"

// longnum is an array[0..LONGNUM_LENGTH] of unsigned int32
// MSB of longnum is bit 7 of int LONGNUM_LENGTH
// LSB of longnum is bit 0 of int 0
// LONGNUM_LENGTH shall be at least 2. Max size depends on computer memory and speed

#if !defined(LONGNUM_LENGTH)
    #define LONGNUM_LENGTH 32                   // nr of words of type t_word in longnum, default 32 => 1kb
#endif

// tested for 8, 16, 32 bit integers and LONGNUM_LENGTH [2..1000]
// todo: test for 64 bit 
typedef uint32_t t_word;                 // type of word 
typedef t_word t_longnum[LONGNUM_LENGTH];       // type of longnum. Note: sizeof (t_longnum) = sizeof(t_word)*LONGNUM_LENGTH

// external variables, to be defined @ runtime:
extern const int BITS_IN_WORD;        // nr of bits in one word. 
extern const int BITS_IN_LONGNUM;     // total nr of bits in the longnum: BITS_IN_WORD * LONGNUM_LENGTH

// todo: check function names
// todo: make a c++ class out of this
// todo: store the order in a variable together with the longnum instead of determining it dynamically (this misses high order 0's)
void long_shiftleft (t_longnum longnum, int count);
void long_shiftright (t_longnum longnum, int count);
void long_xor (t_longnum longnum1, t_longnum longnum2, t_longnum longnumresult);
void long_fill(t_longnum longnum, int value);
t_word long_get_word (t_longnum longnum, int bitnum);
t_word long_get_word_wraparound (t_longnum longnum, int size, int bitnum);
int long_get_bit (t_longnum longnum, int bitnum);
void long_setbit (t_longnum longnum, int bitnum, int value);
void long_copy (t_longnum to, t_longnum from);
int long_cmp (t_longnum ln1, t_longnum ln2);
void long_write_at_location (t_longnum longnum, int location, t_word *newvalue, int n_bits);
int get_order (t_longnum longnum);
void array_to_longnum(uint8_t* arr, t_longnum ln, int n);
void longnum_to_array(uint8_t* arr, t_longnum ln, int n);
//void longnum_reverse (t_longnum longnum);
//int long_find_bit_pattern (t_longnum longnum, unsigned int findme, int n);
void print_longnum_bin (int v, t_longnum longnum);
void print_longnum_hex (int v, t_longnum longnum);
int sprint_longnum_hex(char* line, t_longnum longnum);
void print_longnum_fancy(int verbosity, t_longnum longnum, int wordlength, int size, t_longnum_layout *longnum_layout);

#endif