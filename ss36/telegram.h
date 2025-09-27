/**
 * balise_codec - an open source library to encode and decode Eurobalise messages as described in Subset 36 (FFFIS for Eurobalise, v3.1.0, Dec 17th 2015)
 * Copyright 2023, Fokke Bronsema, fokke@bronsema.net, version 4, February 2024. Published on: https://github.com/FokkeB/subset36.
 * 
 * DISCLAIMER: use at your own risk, the author is not responsible for incorrect en-/decoded messages leading to train related mayhem.
 * 
 * Sources:
 * https://www.era.europa.eu/system/files/2023-01/sos3_index009_-_subset-036_v310.pdf (paragraph 4.3.2)
 * https://lemmatalogic.com/lfsrscrambler/
 * https://www.moria.us/articles/demystifying-the-lfsr/
 * Towards Modeling and Verification of Eurobalise Telegram Encoding Algorithm, Sergey Staroletov, https://www.sciencedirect.com/science/article/pii/S2352146523000728

* This program is free software: you can distribute it and/or modify it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* See the GNU Lesser General Public License for more details.
* You should have received a copy of the GNU General Public License along with this program.
* If not, see < https://www.gnu.org/licenses/>.
*/

#ifndef TELEGRAM_H
#define TELEGRAM_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <bit>    // for popcount instruction used in calculation of Hamming distance

#include "colors.h"
#include "longnum.h"
#include "transformation_words.h"

#define BITLENGTH_LONG_TELEGRAM     1023            // length of long telegram
#define BITLENGTH_SHORT_TELEGRAM    341             // length of short telegram

typedef t_word t_S;                // type of S, the initial state of shift register
typedef t_longnum t_checkbits;     // lower 85 bits are the "check bits"
typedef t_word t_esb;              // lower 10 bits are the "extra shaping bits"
typedef t_word t_sb;               // lower 12 bits are the "scrambling bits"
typedef t_word t_cb;               // lower 3 bits are the "control bits"

#define N_CHECKBITS         85      // Nr. of checkbits
#define N_ESB               10      // Nr. of extra shaping bits
#define N_SB                12      // Nr. of shaping bits
#define N_CB                3       // Nr. of control nits
#define N_USERBITS_L        830     // number of unscrambled user bits in long telegram
#define N_USERBITS_S        210     // number of unscrambled user bits in short telegram
#define N_SHAPEDDATA_L      913     // number of shaped user bits in long telegram
#define N_SHAPEDDATA_S      231     // number of shaped user bits in short telegram
#define OFFSET_SHAPED_DATA  N_CHECKBITS + N_ESB + N_SB + N_CB

#define CONTROL_BITS                1       // three bits, value 001 [b109..b107]
#define MAGIC_WORD                  0xFAB   // My initials in 12 bits ;-)

enum t_size { s_short = BITLENGTH_SHORT_TELEGRAM, s_long = BITLENGTH_LONG_TELEGRAM };
enum t_align { a_undef = 0, a_enc = 1, a_calc = 2 };   // a_undef means undefined coding; a_enc means coding used for reading/writing to disk; a_calc means coding used in calculations

typedef uint32_t t_H;
static const t_H H = 1<<31 | 1<<30 | 1<<29 | 1<<27 | 1<<25 | 1;  // initial coefficients of shift register 

// color layout of a telegram:
static t_longnum_layout telegram_coloring_scheme[5] =
{
    {0, N_CHECKBITS, ANSI_COLOR_GREEN},
    {N_CHECKBITS, N_ESB, ANSI_COLOR_YELLOW},
    {N_CHECKBITS + N_ESB, N_SB, ANSI_COLOR_BLUE},
    {N_CHECKBITS + N_ESB + N_SB, N_CB, ANSI_COLOR_MAGENTA},
    {0, 0, ""}
};

#define FG_ORDER                86      // the order of fg (86 for both a long and a short telegram)

#define ERR_NO_ERR              0       // all OK
#define ERR_NO_INPUT            1       // no input specified
#define ERR_LOGICAL_ERROR       2       // a logical error has occurred
#define ERR_OUTPUT_FILE         3       // error creating output file
#define ERR_MEM_ALLOC           4       // error allocating memory
#define ERR_INPUT_ERROR         5       // error in the input

// error codes from the subset 36:
#define ERR_ALPHABET            10
#define ERR_OFF_SYNCH_PARSING   11
#define ERR_APERIODICITY        12
#define ERR_UNDER_SAMPLING      13
#define ERR_CONTROL_BITS        14
#define ERR_CHECK_BITS          15
#define ERR_SB_ESB_OVERFLOW     16       // overflow of SB and ESB
#define ERR_11_10_BIT           17       // error during conversion
#define ERR_CONTENT             18       // shaped content does not match unshaped content

// expected input sizes of shaped telegrams: 
#define N_CHARS_SHAPED_LONG_HEX          256    // 2 char/byte, 1024 bits/8=128 bytes=256 chars. So: SHR 1 to loose 1 bit to get to 1023 bits in complete telegram.
#define N_CHARS_SHAPED_SHORT_HEX         86     // 2 char/byte, 344 bits/8=43 bytes=86 chars. So: SHR 3 to loose 3 bits to get 341 bits in complete telegram.
#define N_CHARS_SHAPED_LONG_BASE64       172    // 4 chars/3 bytes, 1032 bits/8=129 bytes=172 chars. Leaves 9 bits, of which 8 are captured in trailing "=" So SHR 1 -> 1023 bits in complete telegram.
#define N_CHARS_SHAPED_SHORT_BASE64      60     // 4 chars/3 bytes, 360 bits/8=45 bytes=60 chars. Leaves 19 bits, of which 16 are captured in trailing "==". So SHR 3 -> 341 bits in complete telegram.

// expected input sizes of unshaped telegrams:
#define N_CHARS_UNSHAPED_LONG_HEX        208    // 2 char/byte, 832 bits/8=104 bytes=208 chars. So: SHR 2 -> 830 user bits.
#define N_CHARS_UNSHAPED_SHORT_HEX       54     // 2 char/byte, 216 bits/8=27 bytes=54 chars. So: SHR 6 -> 210 user bits.
#define N_CHARS_UNSHAPED_LONG_BASE64     140    // 4 chars/3 bytes, 840 bits/8=105 bytes=140 chars. Leaves 10 bits, of which 8 are captured in trailing "=". So SHR 2 -> 830 user bits.
#define N_CHARS_UNSHAPED_SHORT_BASE64    36     // 4 chars/3 bytes, 216 bits/8=27 bytes=36 chars. SHR 6 -> 210 user bits.

//#define MAX_RECORDS             0				// max# records to be read in from external file (if set to 0, until memory runs out)
#define MAX_ARRAY_SIZE          500				// max length of byte string read from a file (=one line)

class telegram
{
public:
    string              input_string;               // original input line
    longnum             contents;                   // shaped contents
    enum t_size         size;                       // BITLENGTH_LONG_TELEGRAM (1023) or BITLENGTH_SHORT_TELEGRAM (341)
    longnum             deshaped_contents;          // deshaped contents
    int                 errcode;                    // error code if anything is wrong with this telegram (see below)
    enum t_align        alignment;                  // current alignment of the bits in the telegram: for hex/base64 encoding (a_enc) or for calculations (a_calc).
    unsigned int        number_of_userbits;         // #unshaped (10->11) user bits (m=N_USERBITS_L or N_USERBITS_S)
    unsigned int        number_of_shapeddata_bits;  // #bits in shaped data (N_SHAPEDDATA_L or N_SHAPEDDATA_S; =11/10*number_of_userbits)
    int                 word9, word10;              // indices of the two transformation words in which the control bits, scrambling bits and extra shaping bits are located (see function "shape")
    bool                force_long=false;           // if true: make a long telegram out of this (if this is not already the case). If false: don't mess with the sizes
    longnum             intermediate;               // for intermediate calculation (BCH1). See step 6 in ZUO Peng's article (partial result of check bits calculation up unto the ESB)
    t_sb                intermediate_sb=0;          // the scrambling bits with which the intermediate was calculated
//    telegram            *next=NULL;                 // pointer to the next telegram

    // function prototypes:
    // start with some initialisers, getters, setters and other useful functions:
    telegram(const string inputstr, enum t_size newsize);
    void set_size(enum t_size newsize);
    void make_userdata_long();
    void parse_input(const string inputstr);
    void set_checkbits(const t_checkbits checkbits);
    void get_checkbits(longnum& checkbits);  
    void set_extra_shaping_bits(t_esb esb);
    t_esb get_extra_shaping_bits(void);
    t_esb get_extra_shaping_bits(const longnum readfrom);
    void set_scrambling_bits(t_sb sb);
    t_sb get_scrambling_bits(void);
    void set_control_bits(t_word cb);
    t_cb get_control_bits();
    void set_cb_sb_esb(t_word cb_sb_esb);
//    void set_shaped_data(const longnum sd);   // unused and untested
//    void get_shaped_data(longnum& sd);        // unused and untested
    void print_contents_fancy(int v);
    void align(enum t_align new_alignment);
    void shape_opt(void);
//    void shape(void);
    void deshape(longnum& userdata);
    void deshape(void);
    int check_shaped_telegram(void);
    int check_shaped_deshaped(void);

private:
    // functions needed to (de)shape a telegram:
    void determine_U_tick(longnum& Utick, int m);
    t_S determine_S(t_sb sb);
    t_S determine_S(void);
    void scramble_user_data(t_S S, t_H H, const longnum& user_data_orig, longnum& user_data_scrambled, int m);
    void transform10to11(const longnum& userdata);
    int scramble_transform_check_user_data(t_S S, t_H H, const longnum& user_data_orig, longnum& user_data_scrambled);
    int transform11to10(longnum& userdata);
    void descramble(t_S S, t_H H, longnum& user_data, int m);  
    void calc_first_word(longnum& U, unsigned int m);
//    void compute_check_bits(void);
    void compute_check_bits_opt(void);
//    t_sb set_next_sb_esb(void);
    t_sb set_next_sb_esb_opt(void);
    bool set_next_esb_opt(void);

    // functions needed to perform the tests of candidate telegrams (see subset 36, 4.3.2.5):
    int perform_candidate_checks(int v, int* err_location);  
    int check_alphabet_condition(void);
//    int get_n_cvw(int start, int n, int max) const;
    int check_off_synch_parsing_condition(void);
//    int check_ospc(unsigned int i, unsigned int n_cvw = 10);
    int calc_hamming_distance(t_word word1, t_word word2);  // part of aperiodicity condition
    int check_aperiodicity_condition(void);
    int get_max_run_valid_words(const longnum& ln);  
    int check_undersampling_condition(void);

    // additional functions needed to perform checks of the telegram:
    int check_control_bits(void);
    int check_check_bits(void);
};

// define a list type of telegrams:
typedef std::list<telegram*> t_telegramlist;

#endif