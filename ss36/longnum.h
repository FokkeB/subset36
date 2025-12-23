#pragma once
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

/**
 * longnum - implement a virtual long int of many bits using an array of ints
 *
 * Implemented by fokke@bronsema.net
 * To be used for calculation of encoded Eurobalise contents
 * Feb 2024
 *
*/

#ifndef LONGNUM_H
#define LONGNUM_H

#include "colors.h"
#include <stdint.h>
#include "useful_functions.h"
#include <stdlib.h>       // random numbers
#include <string>
#include <string.h>
// TBD: both needed?
 
 

// longnum is an array[0..WORDS_IN_LONGNUM] of unsigned int32
// MSB of longnum is bit 7 of int WORDS_IN_LONGNUM
// LSB of longnum is bit 0 of int 0
// WORDS_IN_LONGNUM shall be at least 2. Max size depends on computer memory and speed.

#if !defined(WORDS_IN_LONGNUM)
#define WORDS_IN_LONGNUM 32                   // nr of words of type t_word in longnum, default 32 => 1k
#endif

typedef uint32_t t_word;                          // type of word 
typedef t_word t_longnum[WORDS_IN_LONGNUM];       // type of longnum. Note: sizeof (t_longnum) = sizeof(t_word)*WORDS_IN_LONGNUM

// define the parameters of the longnum:
const int BITS_IN_WORD = sizeof(t_word) * 8;                        // nr of bits in one t_word
const int BITS_IN_LONGNUM = sizeof(t_word) * 8 * WORDS_IN_LONGNUM;    // nr of bits in one t_longnum

#define FILL_RANDOM -1

// external variables, to be defined @ compile time:
extern const int BITS_IN_WORD;        // nr of bits in one word. 
extern const int BITS_IN_LONGNUM;     // total nr of bits in the longnum: BITS_IN_WORD * WORDS_IN_LONGNUM

class longnum {

private:
	t_longnum value;  // the actual values of the longnum

public:
	longnum(int with = 0);   // constructor, default set to 0
	longnum(t_word* init_val, int count);   // constructor, fills with indicated values
	longnum operator << (int count) const;
	longnum& operator <<= (int count);
	longnum operator >> (int count) const;
	longnum& operator >>= (int count);
	longnum operator ^ (const longnum& xor_with) const;
	longnum& operator ^= (const longnum& xor_with);
	t_word& operator [] (int i);
	void fill(int value);
	t_word get_word(int bitnum) const;
	t_word get_word_wraparound(int size, int bitnum) const;
	int get_bit(int bitnum) const;
	void set_bit(int bitnum, int value);  
	bool operator == (const longnum ln2);
	bool operator != (const longnum ln2);
	void write_at_location(unsigned int location, const t_word* newvalue, int n_bits);		// write an array of t_words   
	void write_at_location(unsigned int location, const t_word newvalue, int n_bits);		// write one t_word
	int get_order(void) const;
	void read_from_array(uint8_t* arr, int n);
	void write_to_array(uint8_t* arr, int n) const;
	void print_bin(int v) const;
	void print_hex(int v, int n) const;
	int sprint_hex(string& line, int n) const;
	int sprint_base64(string& line, int n) const;
	void print_fancy(int verbosity, int wordlength, int size, t_longnum_layout* longnum_layout) const;

//	Binary Galois Field operations:
	longnum operator * (const longnum& q) const;
	longnum& operator *= (const longnum& q);
	longnum operator + (const longnum& q) const;
	longnum& operator += (const longnum& q);
	longnum operator - (const longnum& q) const;
	longnum& operator -= (const longnum& q);

	longnum& GF2_division(const longnum& denominator, longnum& quotient, longnum& remainder);

	//void reverse (t_longnum longnum);
	//int find_bit_pattern (t_longnum longnum, unsigned int findme, int n);
};

#endif