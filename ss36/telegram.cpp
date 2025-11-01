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

#include "transformation_words.h"
#include "telegram.h"

telegram::telegram(const string inputstr, enum t_size newsize)
// creates and initialises a new telegram
// store the inputstring and set the correct size-parameters
// the longnums are automatically initialised to 0 when created
// if inputstring is given, parses this string and sets the correct size and alignment
{
    input_string = inputstr;
    errcode = ERR_NO_ERR;
    alignment = a_undef;
    word9 = -1; // -1;  // initial values to start of the calculation
    word10 = FIRST_TW_001 - 1;   // point to the transformation word before the first one that starts with 001 (control bits)

    if (inputstr.length() > 0)  
        parse_input(inputstr);
    else
        set_size(newsize);
}

telegram::telegram(const telegram* p_telegram)
// creates a new telegram, copies the contents from p_telegram
{
    memcpy(this, p_telegram, sizeof(telegram));
}

telegram::~telegram(void)
// destructor
{
//    printf("Destroyed telegram with address=%p\n", this);
}

void telegram::set_size(enum t_size newsize)
// sets the new size of the telegram, updates the relevant variables
{
    size = newsize;

    if (newsize == s_long)
    {
        number_of_userbits = N_USERBITS_L;
        number_of_shapeddata_bits = N_SHAPEDDATA_L;
    }
    else 
    {
        number_of_userbits = N_USERBITS_S;
        number_of_shapeddata_bits = N_SHAPEDDATA_S;
    }
}

void telegram::make_userdata_long()
// if this is a short telegram: turn the userdata into a long telegrams by adding just the right amount of 0xFF's to the end and set the size-parameters to the long-values
// note: does not touch the shaped user data, so this could lead to inconsistent telegrams
{
    int i;

    if (size == s_long) 
    // do nothing if this already is a long telegram
        return;

    // make sure the telegram is aligned correctly before changing it:
    align(a_calc);

    eprintf(VERB_ALL, "Short telegram before make_long:\n");
    deshaped_contents.print_fancy(VERB_ALL, 16, number_of_userbits, NULL);

    // shift left to align the content of the short telegram with the long format:
    deshaped_contents <<= (N_USERBITS_L - N_USERBITS_S);

    // padd the trailing (830-210=620) bits with 1's (todo: this could be done a bit more efficiently than bit-setting each individual bit)
    for (i = 0; i < N_USERBITS_L - N_USERBITS_S; i++)
        deshaped_contents.set_bit(i, 1);

    // set sizes:
    set_size(s_long);

    eprintf(VERB_ALL, "Converted to long telegram:\n");
    deshaped_contents.print_fancy(VERB_ALL, 16, number_of_userbits, NULL);
}

void telegram::parse_input(const string inputstr)
// parses the input line into the telegram. This is either a hex or base64 encoded string of the correct length (see below)
// input has to be clean (i.e. no \r\t\n or spaces) and zero-terminated.
// sets the errorcode of the telegram to ERR_INPUT_ERROR if a parsing error occurred.
// for each variant:
//  - parse the char string into a byte string (so from base64/hex -> binary)
//  - convert the byte string to a longnum 
//  - sets the values of telegram
{
    uint8_t arr[MAX_ARRAY_SIZE] = { 0 };   // temporary array to hold the byte array
    int arrsize = 0;                       // #bytes in the temp array

    // first find out what kind of input line we have by switching between the length:
    switch (inputstr.length())
    {
        case N_CHARS_SHAPED_LONG_HEX:
            set_size(s_long);
            arrsize = hex_to_bin(inputstr, arr);
            contents.read_from_array(arr, arrsize);
            break;

        case N_CHARS_SHAPED_SHORT_HEX:
            set_size(s_short);
            arrsize = hex_to_bin(inputstr, arr);
            contents.read_from_array(arr, arrsize);
            break;

        case N_CHARS_SHAPED_LONG_BASE64:
            set_size(s_long);
            arrsize = b64_decode(inputstr, arr);
            contents.read_from_array(arr, arrsize);
            break;

        case N_CHARS_SHAPED_SHORT_BASE64:
            set_size(s_short);
            arrsize = b64_decode(inputstr, arr);
            contents.read_from_array(arr, arrsize);
            break;

        case N_CHARS_UNSHAPED_LONG_HEX:
            set_size(s_long);
            arrsize = hex_to_bin(inputstr, arr);
            deshaped_contents.read_from_array(arr, arrsize);
            break;

        case N_CHARS_UNSHAPED_SHORT_HEX:
            set_size(s_short);
            arrsize = hex_to_bin(inputstr, arr);
            deshaped_contents.read_from_array(arr, arrsize);
            break;

        case N_CHARS_UNSHAPED_LONG_BASE64:
            set_size(s_long);
            arrsize = b64_decode(inputstr, arr);
            deshaped_contents.read_from_array(arr, arrsize);
            break;

        case N_CHARS_UNSHAPED_SHORT_BASE64:
            set_size(s_short);
            arrsize = b64_decode(inputstr, arr);
            deshaped_contents.read_from_array(arr, arrsize);
            break;

        default:
            eprintf(VERB_GLOB, ERROR_COLOR "\nError parsing string" ANSI_COLOR_RESET " \"%s\" of %zd chars.\n", inputstr.c_str(), inputstr.length());
            errcode = ERR_INPUT_ERROR;
    }

    // set the alignment of the telegram to encoding:
    alignment = a_enc;
}

void telegram::set_checkbits (const t_checkbits checkbits)
// sets the checkbits from the indicated array into the telegram contents
{
    contents.write_at_location (0, checkbits, N_CHECKBITS);
}

void telegram::get_checkbits (longnum& checkbits) const
// reads the checkbits from contents, places them in checkbits
{
    int i;

    // clear checkbits:
    checkbits.fill(0);

    for (i=0; i<=2; i++)
        checkbits[i] = contents.get_word(i*BITS_IN_WORD);

    checkbits[2] &= 0x001FFFFF;  // mask the top 11 bits to get the 85 check bits
}

void telegram::set_extra_shaping_bits (t_esb esb)
// sets the 10 extra shaping bits from the indicated array into the telegram 
{
    contents.write_at_location (N_CHECKBITS, &esb, N_ESB);
}

t_esb telegram::get_extra_shaping_bits (const longnum readfrom) const
// returns the 10 "extra shaping bits" from the passed longnum (condition: enc=a_calc)
{
    return (t_esb)contents.get_word (N_CHECKBITS) & 0x3FF;
}

t_esb telegram::get_extra_shaping_bits(void) const
// returns the 10 "extra shaping bits" from the telegram 
{
    return get_extra_shaping_bits(contents);
}

void telegram::set_scrambling_bits (t_sb sb)
// sets the 12 scrambling bits from the indicated array into the telegram
{
    contents.write_at_location (N_CHECKBITS+N_ESB, &sb, N_SB);
}

t_sb telegram::get_scrambling_bits (void) const
// returns the 12 scrambling bits from the telegram (condition: enc=a_calc)
{
    return (t_sb)(contents.get_word(N_CHECKBITS+N_ESB) & 0x0FFF);
}

void telegram::set_control_bits (t_word cb)
// sets the 3 control bits from the indicated array into the telegram
{
    contents.write_at_location (N_CHECKBITS+N_ESB+N_SB, &cb, N_CB);
}

t_cb telegram::get_control_bits (void) const
// returns 3 the control bits from the telegram (condition: enc=a_calc)
{
    return (t_cb)contents.get_word (N_CHECKBITS+N_ESB+N_SB) & 0x7;
}

void telegram::set_cb_sb_esb(t_word cb_sb_esb)
// set the control bits, scrambling bits and extra shaping bits all at once
{
    contents.write_at_location(N_CHECKBITS, &cb_sb_esb, N_ESB + N_SB + N_CB);
}

/*
void telegram::set_shaped_data (const longnum sd)
// sets the shaped data from the indicated array into the telegram
// always write the amount of a long telegram
// currently not used and therefore not tested
{
    contents.write_at_location (N_CHECKBITS+N_ESB+N_SB+N_CB, sd, N_SHAPEDDATA_L);
}
*/
/*
void telegram::get_shaped_data (longnum& sd)
// gets the n-bits (913 or 231) of shaped data (sd) from telegram 
// currently not used and therefore not tested
{
    int i, n_sd = number_of_shapeddata_bits;

    int n_words = n_sd / BITS_IN_WORD;
    int n_bits = n_sd % BITS_IN_WORD;

    for (i=0; i<=n_words; i++)
        sd[i] = contents.get_word (i*BITS_IN_WORD+OFFSET_SHAPED_DATA);

    if (n_sd == N_SHAPEDDATA_L)
        sd[i-1] &= 0x0001FFFF;
    else
        sd[i-1] &= 0x0000007F;
}
*/

void telegram::print_contents_fancy(int v) const
// prints out the telegram in a fancy way (marking each part with a different color)
// uses verbosity level v
{
    return_if_silent(v);

    eprintf(v, "Legend: " BITNUM_COLOR " (xxx)=bit number [0..N]; " ANSI_COLOR_RESET "%d bits shaped data; " ANSI_COLOR_MAGENTA "3 control bits; ", number_of_shapeddata_bits);
    eprintf(v, ANSI_COLOR_BLUE "12 scrambling bits; " ANSI_COLOR_YELLOW "10 extra shaping bits; " ANSI_COLOR_GREEN "85 check bits.\n" ANSI_COLOR_RESET);
    contents.print_fancy(v, 11, size, telegram_coloring_scheme);
}

void telegram::align(enum t_align new_alignment)
/** shifts the telegram contents n bits to the left to prepare for hex/base64 - encoding
* n depends on the telegram size
*/
{
    if (alignment == new_alignment)
    // already aligned correctly
        return;

    if (new_alignment == a_enc)
    // go from a_calc to a_enc by SHL'ing the bits to a byte border:
    {
        contents <<= (8 - size % 8);             // 1023 or 341 => 1 or 3
        deshaped_contents <<= (8 - number_of_userbits % 8);  // 830 or 210 => 2 or 6
    }
    else
    // go from a_enc to a_calc by SHR'ing the bits so that bit#0 is in location#0
    {
        contents >>= (8 - size % 8);                // 1023 or 341 => 1 or 3
        deshaped_contents >>= (8 - number_of_userbits % 8);  // 830 or 210 => 2 or 6
    }

    // store the new alignment:
    alignment = new_alignment;
}

void telegram::determine_U_tick (longnum& Utick)
// calculates U'(k-1) from U (=telegram contents) and writes it to U (see subset 36, paragraph 4.3.2.2, step 1)
{
    t_word sum=0;
    unsigned int i;

    for (i=0; i< number_of_userbits/10; i++)
        sum += Utick.get_word(i*10);

    sum &= 0x3FF;

    Utick.write_at_location(number_of_userbits-10, &sum, 10);
}

t_S telegram::determine_S (t_sb sb)
// determine S and return it (see subset 36, paragraph 4.3.2.2, step 2)
// as t_S is a 32-bit int, no modulo 2^32 is needed
// uses sb in the calculation
{
    return (t_S)(sb*2801775573UL);
}

t_S telegram::determine_S(void)
// determine S and return it (see subset 36, paragraph 4.3.2.2, step 2)
// as t_S is a 32-bit int, no modulo 2^32 is needed
// uses the currently set scrambling bits for the calculation
{
    return determine_S (get_scrambling_bits());
}
/*
void telegram::scramble_user_data(t_S S, t_H H, const longnum& user_data_orig, longnum& user_data_scrambled, int m)
// scrambles the data in user_data_orig into user_data_scrambled (see subset 36, paragraph 4.3.2.2, step 3)
// tbd optimisation: use a lookup table?
{
    int i;
    int user_bit, t, sb;

    for (i=m-1; i>=0; i--)
    // iterate over the m user bits
    {
        user_bit = user_data_orig.get_bit (i);
        t = (char)(S>>31);
        sb = t ^ user_bit;
        user_data_scrambled.set_bit(i, sb);

        S <<= 1;
        if (sb)
            S ^= H;
    }
}
*/
/*
void telegram::transform10to11 (const longnum& userdata) 
// performs the transformation of the scrambled user bits from 10 bits to 11 bits, 
// write the 11-bit words into telegram at the right place
// see subset 36, paragraph 4.3.2.3
{
    int i, p;
    t_word tw;

    for (i=(number_of_userbits/10)-1; i>=0; i--)
    // iterate over the user bits, get the transformed value and store it in telegram
    {
        p = userdata.get_word(i*10)&0x03FF;
        tw = (t_word)transformation_words[p];
        contents.write_at_location(i*11 + OFFSET_SHAPED_DATA, &tw, 11);
    }    
}
*/
/*
int telegram::get_n_cvw(int start, int n, int n_cvw_max) const
// determines the maximum number of consecutive valid words starting at start, using wraparound
// looks at max n words from start (if 0, checks all scrambled user bits)
// returns with ERR_OFF_SYNCH_PARSING if more than the max nr of cvw were found or ERR_NO_ERR if not
{
    int max_count, i, n_cvw=0;
    t_word temp;

    max_count = (n == 0) ? number_of_shapeddata_bits : (n-1)*11;

    for (i = start; i <= start + max_count; i+=11)
    {
        temp = contents.get_word_wraparound(size, i) & 0x7FF;
        eprintf(VERB_GLOB, "i=%d -> ", i); print_bin(VERB_GLOB, temp, 11);

        if (find11(temp) != -1)  // tbd: const
            n_cvw++;
        else
            n_cvw = 0;

        eprintf(VERB_GLOB, "; n_cvw=%d\n", n_cvw);

        if (n_cvw > n_cvw_max)
        {
            eprintf(VERB_GLOB, "ERR_OFF_SYNCH_PARSING occurred\n");
            return ERR_OFF_SYNCH_PARSING;
        }
    }

    return ERR_NO_ERR;
}
*/
/*
int telegram::check_ospc(unsigned int i, unsigned int n_offsets, )
// checks the "off-sync parsing condition". 
// n_cvw: only check the first n_cvw offsets
// i: start at bit i
{
    unsigned int i_offset = 0;
    int cvw_offsets[] = { 1, 10, 9, 2 , 8, 3, 7, 4, 6, 5 };  
    int i_last_nvw[] = { number_of_userbits / 10 - 1, number_of_userbits / 10 - 1 };   // start one word before number of userbits
    int i_vw, max_cvw; // , n_cvw = sizeof(cvw_offsets) / sizeof(cvw_offsets[0]);
    t_word lookatword;

    if (n_offsets > 10)
        n_offsets = 10;

    for (i_offset = 0; i_offset < n_offsets; i_offset++)
    // check the OSPC for each useful offset using a greedy algorithm
    {
        // determine the max allowed cvw for this case:
        if (i_offset <= 1) // offsets 1, 10
                max_cvw = 2;
        else  // cases 2..9:
            if (size == s_long)
                max_cvw = 10;
            else
                max_cvw = 6;

        if ((i == i_last_nvw[i_offset] - (max_cvw + 1)) && (i_last_nvw[i_offset] >= (max_cvw + 1)))
        // Max nr of words away from last non-valid word and not yet at the end; see if there are any other non-valid words amongst them
        // store the position of the last non-valid word in the i_last_nvw-array
        {
            for (i_vw = i; i_vw < i_last_nvw[i_offset]; i_vw++)
            {
                lookatword = contents.get_word_wraparound(size, i_vw * 11 + OFFSET_SHAPED_DATA + cvw_offsets[i_offset]) & 0x7FF;

                eprintf(VERB_ALL, "bit=%d -> ", i_vw * 11 + OFFSET_SHAPED_DATA + cvw_offsets[i_offset]);
                print_bin(VERB_ALL, lookatword, 11);
                if (verbose >= VERB_ALL)
                    printf(" = octal %o\n", lookatword);

                if (find11(lookatword) == NO_TW)
                // current word is no transformation word, point i_last_nvw to this word
                {
                    i_last_nvw[i_offset] = i_vw;
                    break;
                }
            }

            if (i == i_last_nvw[i_offset] - (max_cvw + 1))
            // more than max nr of cvw's found
            {
                eprintf(VERB_ALL, "OSPC fail\n");
                for (offset_index = 0; offset_index < n_offsets; i_offset++)
                    eprintf(VERB_ALL, "Offset: %d\n", cvw_offsets[i_offset]);

                return ERR_OFF_SYNCH_PARSING;
            }
        }
    }

    return ERR_NO_ERR;
}
*/

int telegram::scramble_transform_check_user_data(t_S S, t_H H, const longnum& user_data_orig)   
// scrambles the data in user_data_orig into contents (see subset 36, paragraph 4.3.2.2, step 3)
// TBD optimisation: create a persistent (8 bits or 10 bits) lookup table for the current S. This will give an optimisation as this function may be called many times for a certain telegram.
// If 10 bits, this can be combined with the 11-bit lookup and candidate checks.
// see paragraph 3.1 in article of ZHUO Peng
// checks the ERR_OFF_SYNCH_PARSING during scrambling and returns the error as soon as such an error occurred
// Note: according to ZHUO Peng, checking the "Aperiodicity Condition for Long Format" is a very small optimisation (1%, see description @ step 4) and is therefore skipped
{
    int i, j;
    int user_bit, t, sb, m_index = number_of_userbits;
    t_word val10, val11, lookatword;
    
    // vars needed for greedy algorithm:
    int offset_index = 0;
    int cvw_offsets[] = { 1, 10, 9, 2 , 8, 3, 7, 4, 6, 5 };  
    int i_start = number_of_userbits / 10 - 1;   // start one word before the number_of_userbits
    int i_last_nvw[] = { i_start, i_start, i_start, i_start, i_start, i_start, i_start, i_start, i_start, i_start };
    int i_vw, max_cvw = 2, n_cvw = sizeof(cvw_offsets) / sizeof(cvw_offsets[0]);
    
    for (i = i_start; i >= 0; i--)
    // outer loop running over the 10-bit words, starting with the last word
    {
        val10 = 0;
        for (j = 9; j >= 0; j--)
        // inner loop, iterating over the bits in the current 10-bit word
        {
            m_index--;  // keep track of the current bit 
            user_bit = user_data_orig.get_bit(m_index);
            t = (char)(S >> 31);
            sb = t ^ user_bit;
            val10 += sb << j;

            S <<= 1;
            if (sb)
                S ^= H;
        }

        // 10 bits calculated, find the corresponding transformation word and write it to the correct position:
        val11 = (t_word)transformation_words[val10];
        contents.write_at_location(i * 11 + OFFSET_SHAPED_DATA, &val11, 11);
        // print the current status:
        eprintf(VERB_ALL, "#userbits=%d, i=%d, m_index=%d\n", number_of_userbits, i, m_index);
        print_contents_fancy(VERB_ALL);

        for (offset_index = 0; offset_index < n_cvw; offset_index++)
        // Iterate over the offsets; check the OSPC for each offset using a greedy algorithm
        {        
            // determine the max allowed cvw for this case:
            // not needed if only the first two offsets are checked
            if (offset_index <= 1) // offsets -1, 1
                    max_cvw = 2;
            else  // cases 2..9:
                if (size == s_long)
                    max_cvw = 10;
                else
                    max_cvw = 6;
        
            if ((i == i_last_nvw[offset_index] - (max_cvw + 1)) && (i_last_nvw[offset_index] >= (max_cvw + 1)))
            // Max nr of words away from last non-valid word and not yet at the end; see if there are any other non-valid words amongst them
            // store the position of the last non-valid word in the i_last_nvw-array
            {
                for (i_vw = i; i_vw < i_last_nvw[offset_index]; i_vw++)
                {
                    lookatword = contents.get_word_wraparound(size, i_vw * 11 + OFFSET_SHAPED_DATA + cvw_offsets[offset_index]) & 0x7FF;

                    if (verbose >= VERB_ALL)
                    {
                        printf("offset=%d; bit=%d; ", cvw_offsets[offset_index], i_vw * 11 + OFFSET_SHAPED_DATA + cvw_offsets[offset_index]);
                        print_bin(VERB_ALL, lookatword, 11);
                        printf(" = octal %o", lookatword);
                    }

                    if (find11(lookatword) == NO_TW)
                    // current word is no transformation word, point i_last_nvw to this word
                    {
                        i_last_nvw[offset_index] = i_vw;
                        eprintf(VERB_ALL, "; Transformation word: no\n");
                        break;
                    }
                    else
                        eprintf(VERB_ALL, "; Transformation word: yes\n");
                }

                if (i == i_last_nvw[offset_index] - (max_cvw + 1))
                // more than max nr of cvw's found
                {
                    eprintf(VERB_ALL, "OSPC check failed\n");
                    return ERR_OFF_SYNCH_PARSING;
                }
            }
        } 
    }
    
    eprintf(VERB_ALL, "OSPC check passed!\n");
    return ERR_NO_ERR;
}

int telegram::transform11to10 (longnum& userdata) 
// performs the transformation from 11 bits back to 10 bits; returns ERR_11_10_BIT if an error occurred (11-bit value not found in list) or ERR_NO_ERR if no errors occurred
// reads transformed data from telegram contents (from OFFSET_SHAPED_DATA), writes the original user data to userdata starting at bit 0
// see subset 36, paragraph 4.3.2.3
{
    int i;                  // index
    t_word bit10, bit11;    // ints to store the temp 10-bit and 11-bits values

    for (i=(number_of_userbits/10)-1; i>=0; i--)
    // iterate over the array of 11-bit values, get the original 10-bit value and store it in userdata
    {
        bit11 = contents.get_word(OFFSET_SHAPED_DATA + i*11) & 0x07FF;  // get the next 11 bits from the telegram contents
        bit10 = find11 (bit11);

        if (bit10 == NO_TW)
        {
            // this should never occur with a correctly encoded telegram:
            eprintf(VERB_ALL, "ERR: 11-bit value not found at i=%d; val=%o", i, bit11);
            return ERR_11_10_BIT;
        }

        userdata.write_at_location(i*10, &bit10, 10);
    }   

    return ERR_NO_ERR; 
}

void telegram::descramble (t_S S, t_H H, longnum& user_data, int m)
// descrambles the scrambled data in user_data, writes the descrambled data back to userdata
// S contains the start values of the shift register, H are the coefficients and m is the amount of bits to be decoded
{
    int i;
    char descrambled_bit, t, scrambled_bit;

    for (i=m-1; i>=0; i--)
    {
        scrambled_bit = user_data.get_bit (i);
        t = (char)(S>>31);  // get current output of the shiftregister
        descrambled_bit = t ^ scrambled_bit;
        user_data.set_bit (i, descrambled_bit);

        // shift the register 1 bit to the next step
        S = (S << 1);

        // apply H
        if (scrambled_bit)
            S ^= H;
    }
}

void telegram::calc_first_word (longnum& U, unsigned int m)
// calculates the first 10-bit word in the descrambled U, which is known in subset 36 (see 4.3.2.2) as U'(k-1)
// m is the amount of user bits (830 or 210), so k=m/10 (0..83/21)
// U'(k-1) = sum(U(k-1..0)) = U(k-1) + sum(U(k-2..0))
// therefore: U(k-1) = U'(k-1) - sum (U(k-2..0)).
// U(k-1) is written in the last 10-bit word of U, replacing U'(k-1)
// The MOD 2^10 part is not needed in the calculations because only the last 10 bits are saved
{
    unsigned int sum=0, temp, i;

    // calculate sum(U(k-2..0)):
    for (i=0; i<=(m/10-2); i++)
        sum += U.get_word(i*10) & 0x3FF;

    // calculate U(k-1) and save it in U:
    temp = U.get_word(m-10) - sum;
    U.write_at_location(m-10, &temp, 10);
}
/*
void telegram::compute_check_bits (void)
// compute the check bits as described in SS36, 4.3.2.4, using a different implementation than used during the creation of the telegram
// input: a filled telegram (check bits already present will be overwritten)
// output: the checkbits in bit 0..84 of the telegram
// performs a sanity check on the calculations
// note that (as both f and g are constants), g and f*g are used, rather than calculating f*g at each run from f and g
// does not return an error code as this should always work (if it doesn't: DIE)
{
    longnum g, fg, quotient, remainder, sanitycheck, checkbits; // default initialised with 0's
    longnum temp;

    // clear the lower 85 check bits [0..84] from the input telegram, needed for the calculation:
    contents[0] = 0;            // bit [0..31]
    contents[1] = 0;            // bit [32..63]
    contents[2] &= 0xFFE00000;  // bit [64..84]

    eprintf(VERB_ALL, HEADER_COLOR "\nCalculating check bits:\n" ANSI_COLOR_RESET);
    eprintf(VERB_ALL, FIELD_COLOR "input telegram:\t" ANSI_COLOR_RESET); contents.print_bin (VERB_ALL);

    // determine the inputs to the calculation (kept here instead of moving them to ss36.h):
    if (size == s_long) 
    {
        // polynomials for a long telegram:
//        f[0] = 0b11011011111;  // not used, using fg instead
        g[0] = 0b11010101001000111011101000010011; 
        g[1] = 0b01110011100110100111101000101110; 
        g[2] = 0b101110001000;

        // calculated f*g: 0x003EC171 890C6F72 C063B091
        fg[0] = 0xC063B091;
        fg[1] = 0x890C6F72;
        fg[2] = 0x003EC171;
    }
    else
    {
        // polynomials for a short telegram:
//        f[0] = 0b10110101011;  // not used, using fg instead
        g[0] = 0b11001010010010100011110001001011;
        g[1] = 0b10010000110000101111111011110111;
        g[2] = 0b100111110111;

        // calculated f*g: 0x002BB94D 87757959 021B6D65
        fg[0] = 0x021B6D65;
        fg[1] = 0x87757959;
        fg[2] = 0x002BB94D;
    }

    // calculate f*g:
//    GF2_multiply (f, g, fg);   // fg is pre-defined above instead of calculating it, to save some clock ticks and memory 

// divide the telegram by f*g, yielding quotient and remainder:
    contents.GF2_division (fg, quotient, remainder);

    // add (=xor) g to the remainder -> checkbits!:
    checkbits = remainder + g;

    eprintf(VERB_ALL, FIELD_COLOR "\nmasked 0..84:\t" ANSI_COLOR_RESET); contents.print_bin (VERB_ALL);
    eprintf(VERB_ALL, FIELD_COLOR "\nf*g:\t\t" ANSI_COLOR_RESET); fg.print_bin (VERB_ALL);
    eprintf(VERB_ALL, FIELD_COLOR "\nquotient:\t" ANSI_COLOR_RESET); quotient.print_bin (VERB_ALL);
    eprintf(VERB_ALL, FIELD_COLOR "\nremainder:\t" ANSI_COLOR_RESET); remainder.print_bin (VERB_ALL);
    eprintf(VERB_ALL, FIELD_COLOR "\ncheckbits:\t" ANSI_COLOR_RESET); checkbits.print_bin (VERB_ALL);
    eprintf(VERB_ALL, "\n");

    // perform a sanity check by calculating the original bits from the quotient, remainder and g:

    // subtract g from the checkbits to get the remainder (note: this is a xor-operation, just like addition)
    sanitycheck = checkbits - g;

    // sanitycheck should now be identical to the remainder, show an error message and exit if this is not the case:
    if (sanitycheck != remainder)
    {
        eprintf(VERB_QUIET, ERROR_COLOR "ERROR" ANSI_COLOR_RESET" - sanity check of remainder is NOK, exiting...\n"); 
        exit(ERR_LOGICAL_ERROR);
    }

    // multiply the quotient with fg, store in sanitycheck:
    sanitycheck = quotient * fg;

    // and add the remainder:
    sanitycheck += remainder;

    // see if the check is OK (should always be the case):
    if (sanitycheck != contents)
    {
        eprintf(VERB_QUIET, ERROR_COLOR "ERROR" ANSI_COLOR_RESET" - check bits sanity check is NOK, exiting...\n");
        exit(ERR_LOGICAL_ERROR);
    }
    else
        eprintf(VERB_ALL, OK_COLOR "Sanity check of check bits is OK\n" ANSI_COLOR_RESET);

    // finally copy the checkbits into the lower 85 bits of telegram (we cleared these above):
    contents[0] = checkbits[0];    // bits 0..31
    contents[1] = checkbits[1];    // bits 32..63
    contents[2] |= checkbits[2];   // set bits 64..84
}
*/

void telegram::compute_check_bits_opt(void)
// compute the check bits as described in Subset 36, 4.3.2.4. Does not recalculate the first part of the telegram if the scramble bits haven't changed.
// input: a filled telegram (check bits already present will be overwritten)
// output: the checkbits in bit 0..84 of the telegram
// note that (as both f and g are constants), g and f*g are used, rather than calculating f*g at each run from f and g
// does not return an error code as this always works
// tbd optimisation?: use lookup table 
{
    longnum remainder, checkbits; 
    int shift, i;
    longnum g, fg;

    // clear the lower 85 bits [0..84] from the input telegram, needed for the calculation:
    contents[0] = 0;            // bit [0..31]
    contents[1] = 0;            // bit [32..63]
    contents[2] &= 0xFFE00000;  // bit [64..84]

    eprintf(VERB_ALL, HEADER_COLOR "\nCalculating check bits:\n" ANSI_COLOR_RESET);
    eprintf(VERB_ALL, FIELD_COLOR "Input telegram:\t" ANSI_COLOR_RESET); print_contents_fancy(VERB_ALL);

    // determine the inputs to the calculation (kept here instead of moving them to ss36.h):
    if (size == s_long)
    {
        // polynomials for a long telegram:
//        f[0] = 0b11011011111;  // not used, using fg instead

        g[0] = 0b11010101001000111011101000010011;
        g[1] = 0b01110011100110100111101000101110;
        g[2] = 0b101110001000;

        // calculated f*g: 0x003EC171 890C6F72 C063B091
        fg[0] = 0xC063B091;
        fg[1] = 0x890C6F72;
        fg[2] = 0x003EC171;
    }
    else
    {
        // polynomials for a short telegram:
//        f[0] = 0b10110101011;  // not used, using fg instead

        g[0] = 0b11001010010010100011110001001011;
        g[1] = 0b10010000110000101111111011110111;
        g[2] = 0b100111110111;

        // calculated f*g: 0x002BB94D 87757959 021B6D65, with order 86
        fg[0] = 0x021B6D65;
        fg[1] = 0x87757959;
        fg[2] = 0x002BB94D;
    }

    // See if the previously calculated intermediate can be used
    if (get_scrambling_bits() == intermediate_sb)  
    // already calculated the remainder up to the ESB. Copy the intermediate result, set the right ESB's and continue the calculation
    {
        remainder = intermediate;
        remainder.write_at_location(N_CHECKBITS, get_extra_shaping_bits(), N_ESB);
        eprintf(VERB_ALL, "Reused intermediate calculation for ESB=%d.\n", intermediate_sb); //intermediate.print_bin(VERB_GLOB);
    }
    else
    // No previous calculation for the current SB; copy telegram contents into remainder
        remainder = contents;

    // Perform the calculation (GF2 division, from which only the remainder is relevant)
    for (i = remainder.get_order(); i >= FG_ORDER; i--)
    {
        shift = i - FG_ORDER;
        if (remainder.get_bit(i - 1))
            remainder ^= fg << shift;

        if (i == N_CHECKBITS + N_ESB + FG_ORDER)
        // calculated everything up to the Extra Shaping Bits, store the intermediate result
        {
            intermediate = remainder;
            intermediate_sb = get_scrambling_bits();
            eprintf(VERB_ALL, "Stored intermediate calculation for ESB=%d: \n",intermediate_sb); intermediate.print_bin(VERB_ALL);
        }

        //eprintf(VERB_ALL, "\ni=%d; shift=%d; \nremainder=", i, shift); remainder.print_bin(VERB_GLOB); 
    }

    // add (=xor) g to the remainder -> checkbits!:
    checkbits = remainder + g;

    // save the checkbits
    contents[0] = checkbits[0];    // bits 0..31
    contents[1] = checkbits[1];    // bits 32..63
    contents[2] |= checkbits[2];   // set bits 64..84
}

//t_sb telegram::set_next_sb_esb(void)
/** Sets the next scrambling bits and extra scrambling bits. See SubSet 36, 4.3.1.2 Telegram Format:
* Because of the Alphabet condition, these two words need to come from the "transformation words".
* word #10 (starting at bit#99) exists of two parts: [b10..b8] are the control bits (always 001) and [b7..b0] are the first 8 scrambling bits.
* word #9 (starting at bit#88) also exists of two parts: [b10..b7] are the last 4 scrambling bits and [b6..b0] are the first 7 extra scrambling bits
* the last 3 bits of the ESB are at word #8 (starting at bit #77) and are [b10..b8]
*
* So, updating the SB and ESB goes as follows:
* word#10: pick a transformation word of which the three high bits are 001. These are the transformation words in the range of [00401 .. 00776] (bin: 00 100 000 001 to 00 111 111 110), 
*          or with index [#104 .. #260];
* word#9: pick any transformation word;
* word#8: iterate over the possible values [0..7] for bits [b10..b8] (all values are allowed)
*
* this function determines the next SB and ESB and writes these directly to telegram
* if run for the first time for a certain telegram, set telegram->word9 to -1 and telegram->word10 to 0.
* returns the new scrambling bits
*/
/*
{
    #define FIRST_TW_001    104 // index of first transformation word starting with 001
    #define LAST_TW_001     260 // index of last transformation word starting with 001

    t_word cb_sb_esb = contents.get_word(N_CHECKBITS) & 7;  // isolate the current three high bits in word#8 (= 3 lower bits of ESB)

    // if word10 is not initialised, point it to the first transformation word that starts with 001 (=#104)
    if (word10 == 0)
        word10 = FIRST_TW_001;

    // determine the new values for ESB and SB:
    if ((word9 != -1) && (cb_sb_esb < 7))  // check if word8 will overflow in the next instruction, but only if this is not the first run
        cb_sb_esb++;  // increase the lower three bits of ESB if it does not overflow
    else
    // find the next transformation word to put in word#9. For speed purposes, we could consider searching the next T.W. that starts with the last four scrambling bits [SB7..SB4], 
    // but this would reduce the solution space (unless we keep track of the skipped T.W.'s, but this would really complicate things).
    // Note that this is not needed, as the transformation words are ordered from low to high due to which the next T.W. always has the optimal four start bits.
    {
        cb_sb_esb = 0;
        word9++;
        if (word9 >= N_TRANS_WORDS)  
        // overflow of word9 or word10 not initialised -> find next/first word10
        {
            word9 = 0;       // reset tf_9
            if (word10 < LAST_TW_001)
                word10++;    // get next tf_10
            else
            {  // this should only veeeeery rarely happen: 10^-100 (see subset 36, A1.1.1)
                eprintf(VERB_QUIET, ERROR_COLOR "\n\nERROR:" ANSI_COLOR_RESET " No valid combination of Scrambling Bits and Extra Shaping Bits found for the telegram below. \n");
                eprintf(VERB_QUIET, "Please make a minor change in the telegram contents and try again. See Subset-036.\n");
                eprintf(VERB_QUIET, "Also: please send a copy of the input telegram to the writer of this program (fokke@bronsema.net). Thanks :-)\n");
                eprintf(VERB_QUIET, "Contents of input telegram: \n");// , telegram->input_string);
                align(a_enc);  // shift the bits to the left to prepare for printing
                deshaped_contents.print_hex(VERB_QUIET, number_of_userbits);
                eprintf(VERB_QUIET, "\n\n");

                exit(ERR_SB_ESB_OVERFLOW);
            }
        }
    }
    cb_sb_esb += (transformation_words[word9] << 3);         // fill bits [4..15] with tf<<3, keeping the lower three bits
    cb_sb_esb += (transformation_words[word10] << 14);       // set word#10

    contents.write_at_location(N_CHECKBITS, &cb_sb_esb, N_ESB+N_SB+N_CB);  // write the cb+esb+sb to the telegram

    return (cb_sb_esb >> 10) & 0xFFF;  // return the new scrambling bits: bits [21..10] of word8 
}
*/

int telegram::set_next_sb_esb_opt(void)
/* sets the next scrambling bits (and resets the extra shaping bits) by selecting the next transformation word(s)
* if run for the first time for a certain telegram, set telegram->word9 to -1.
* word9 contains the last 4 scrambling bits and the first 7 extra shaping bits
* word10 contains the 3 control bits and the first 8 scrambling bits
* both word9 and word10 need to be transformation words
* Returns an overflow-error or ERR_NO_ERR
*/
{
    t_word cb_sb_esb = 0; 
    unsigned short int old9;

    if (word9 == -1)
    {
        word9 = 0;
        word10++;
    }
    else
    {
        old9 = transformation_words[word9] & 0b11110000000; // isolate the current first four bits of word9 (=last four scrambling bits)

        // find the next word9 with different high four bits
        while (old9 == (transformation_words[++word9] & 0b11110000000))
        {
            if (word9 >= N_TRANS_WORDS - 1)  // this means that the last t.w. will be skipped, but that's ok (same four high bits)
            {
                word9 = 0;
                word10++;
                if (word10 > LAST_TW_001) // overflow
                {
                    errcode = ERR_SB_ESB_OVERFLOW;
                    return ERR_SB_ESB_OVERFLOW;
                }
                break;
            }
        }
    }

    cb_sb_esb += (transformation_words[word9] << 3);         // fill bits [4..15] with tf<<3, clear the lower three bits
    cb_sb_esb += (transformation_words[word10] << 14);       // set word#10
    set_cb_sb_esb(cb_sb_esb); // write the cb+esb+sb to the telegram, reset the three lower ESB-bits

    //eprintf(1, "Set_next_sb_esb: Updated CB, SB and ESB to:\n"); print_contents_fancy(1);
    //eprintf(1, "word10=%d; tw10=o%o; word9=%d; tw9=o%o; \n", word10, transformation_words[word10], word9, transformation_words[word9]);
     
    return ERR_NO_ERR; // (cb_sb_esb >> 10) & 0xFFF;  // return the new scrambling bits: bits [21..10] of word8 
}

bool telegram::set_next_esb_opt(void)
// sets the next esb for which word9 begins with the four LSB's of the current SB (=high four bits of tw[word9]).
// returns true if a next ESB was found. 
// if no more esb's are available, returns false.
{
    t_word esb = contents.get_word(N_CHECKBITS) & 7;  // isolate the current three high bits in word#8 (= 3 lower bits of ESB)

    if (esb < 7)
    // if this is possible, only increase the lower three bits 
        esb++;
    else
    {
        esb = 0;
        if ((word9 < N_TRANS_WORDS-1) && ((transformation_words[word9] & 0b11110000000) == (transformation_words[word9 + 1] & 0b11110000000)))
        // find the next t.w. that starts with the same four bits. 
            word9++;
        else
        // no new transformation word available with the same high 4 bits -> new scrambling bits are needed. Return false.
            return false;
    }

    esb += (transformation_words[word9] << 3);         // fill bits [4..15] with tf<<3, keep the lower three bits
    set_extra_shaping_bits(esb);

    eprintf(VERB_ALL, "set_next_esb: Updated CB, SB and ESB to:\n"); print_contents_fancy(VERB_ALL);
    eprintf(VERB_ALL, "word10=%d; tw10=o%o; word9=%d; tw9=o%o; \n", word10, transformation_words[word10], word9, transformation_words[word9]);

    return true;
}

void telegram::shape_opt(void)
// Encodes the userdata (deshaped_contents) in the telegram (filling contents).
// Recalculate with different settings (sb/esb) if the checks fail and repeat until the checks don't fail.
// Checks the "off-synch-parsing-condition" (and not the "aperiodicity condition for long format") while 
// shaping the user data in order to find out illegal telegrams ASAP.
// See subset 36 for more information
{
    longnum Utick, temp;
    int err_location = 0, errs_found = 0, err, n_iter = 0; // , result;
    t_word current_sb = 0, new_sb = 0;
    bool inc_sb = (word9 == -1); // true if run for the first time for this telegram

    align(a_calc);
    Utick = deshaped_contents;
    determine_U_tick(Utick);
    eprintf(VERB_ALL, "\nU'=\n"); Utick.print_bin(VERB_ALL);

    do
    // repeat until we find a correct telegram or there is an overflow of sb/esb
    {
        if (inc_sb)  
        // increase the scrambling bits if run for the first time or if the next loop fails to find a new ESB
        // this loop should be skipped if not all the possible ESB's/CRC's were checked yet when calculating all telegrams
            do
            // calculate the next scrambling bits and create shaped user bits that pass a large part of the Off-Synch-Parsing Condition
            {
                n_iter++;
                if (set_next_sb_esb_opt() == ERR_SB_ESB_OVERFLOW)
                {
                    eprintf(VERB_ALL, "Overflow of SB/ESB occurred.\n");
                    return;
                }
            } while (scramble_transform_check_user_data(determine_S(), H, Utick) != ERR_NO_ERR);
        inc_sb = true;

        do
        // compute the check bits (CRC), perform checks and update the extra shaping bits until a correct solution is found.
        // if none can be found, start from the top with new scrambling bits
        {
            compute_check_bits_opt();
            n_iter++;

            eprintf(VERB_ALL, "\nChecking new telegram:\n");
            print_contents_fancy(VERB_ALL);

            // now see if the packet is "well formed", make another run if not.
            err = perform_candidate_checks(VERB_ALL, &err_location);

            if ((err == ERR_OFF_SYNCH_PARSING) || (err == ERR_APERIODICITY))
                if (err_location >= OFFSET_SHAPED_DATA)
                // error sequence is located completely in the shaped user data, it is therefore pointless to update the ESB
                // solution: set last three bits of ESB to 111, so the next word 9 and if necessary word10 are selected in the next run
                { 
                    contents.write_at_location(N_CHECKBITS, 0b111, 3);  // set the lower three bits of the ESB to 111 
                }
        } while (err && set_next_esb_opt());
    } while (err);
    
    eprintf(VERB_GLOB, "Shaped the telegram in %d combinations of scrambling bits and extra shaping bits.\n", n_iter);
}

/*
void telegram::shape(void)
// encodes the userdata (deshaped_contents) in the telegram (filling contents).
// recalculate with different settings (sb/esb) if the checks fail and repeat until the checks don't fail.
{
    longnum UD_scrambled, Utick;
    int err_location = 0, errs_found = 0, err, n_iter = 0;
    t_word current_sb = 0, new_sb = 0;

    align(a_calc);

    Utick = deshaped_contents;
    determine_U_tick(Utick, number_of_userbits);
    eprintf(VERB_ALL, "\nU'=\n"); Utick.print_bin(VERB_ALL);
    word9 = -1;  // will be set to 0 in the first run of set_next_sb_esb
    word10 = 0;

    // try to find a correctly shaped telegram
    do
    {
        // find the next combination of sb/esb:
        new_sb = set_next_sb_esb();   // also sets the three control bits to 001
        
        n_iter++;
 
        if (new_sb != current_sb)
        // scrambling bits have changed, re-scramble the user data.
        // if scrambling bits haven't changed, there is no need to perform this calculation
        {
            current_sb = new_sb;

            // scramble the user data into UD_scrambled
            scramble_user_data(determine_S(current_sb), H, Utick, UD_scrambled, number_of_userbits);
            eprintf(VERB_ALL, "\nscrambled data =\n"); UD_scrambled.print_bin(VERB_ALL);

            // then "shape" the user data (replace 10 bit words with 11 bit words) into telegram:
            transform10to11(UD_scrambled);
            eprintf(VERB_ALL, "\n10 to 11 =\n"); contents.print_bin(VERB_ALL);
        }

        // compute the check bits (CRC):
        compute_check_bits();

        eprintf(VERB_ALL, "\nChecking new telegram:\n");
        print_contents_fancy(VERB_ALL);

        // now see if the packet is "well formed", make another run if not.
        err = perform_candidate_checks(VERB_ALL, &err_location);

        if ((err == ERR_OFF_SYNCH_PARSING) || (err == ERR_APERIODICITY))
            if (err_location >= OFFSET_SHAPED_DATA)
            // error sequence is located completely in the shaped user data, it is therefore pointless to update the ESB
            // solution: set ESB to 111, so the next word 9 and if necessary word10 are selected in the next run
            {
                contents.write_at_location(N_CHECKBITS, 0b111, 3);  // set the lower three bits of the ESB to 111 
            }

    } while (err);

    eprintf(VERB_GLOB, "Shaped the telegram in %d iterations.\n", n_iter);
}
*/

void telegram::deshape(longnum& userdata)
// deshapes the shaped data in the telegram into userdata (which could be part of telegram)
{
    t_S S = 0;

    align(a_calc);

    eprintf(VERB_ALL, "DESHAPING:\n");
    transform11to10(userdata);
    eprintf(VERB_ALL, "\n11 to 10 =\n"); userdata.print_bin(VERB_ALL);

    S = determine_S();

    descramble(S, H, userdata, number_of_userbits);
    eprintf(VERB_ALL, "Descrambled:\n"); userdata.print_bin(VERB_ALL);
    calc_first_word(userdata, number_of_userbits);
    eprintf(VERB_ALL, "Calc 1st word:\n"); userdata.print_bin(VERB_ALL);

    eprintf(VERB_ALL, "FINISHED DESHAPING\n");
}

void telegram::deshape()
// deshapes the telegram into deshaped_contents
{
    deshape(deshaped_contents);
}

int telegram::perform_candidate_checks(int v, int* err_location)
// Performs all the checks in subset 36, paragraph 4.3.2.5 "Testing Candidate Telegrams".
// Returns one of the subset 36 error codes, or 0 if all OK, stops checking after occurence of the first error.
// Sets err_location to point to the start bit of the error (if this makes sense).
// Perform the checks in the optimal order, see elimination rates in ZHUO Pengs article, Step 7.
// Always check the complete telegram. This leads to some redundant checks (the alphabet condition and the off-synch parsing check
// are partially performed during the calculations), but it seems like a good idea to check everything once the calculations are done.
{
    *err_location = check_alphabet_condition();
    if (*err_location != MAGIC_WORD)
    {
        eprintf(v, ERROR_COLOR "check_alphabet_condition fails" ANSI_COLOR_RESET " at word starting with bit#%d.\n", *err_location);
        return ERR_ALPHABET;
    }
    else
        eprintf(v, "Check alphabet condition:\t\t\t" OK_COLOR "OK\n" ANSI_COLOR_RESET);

    *err_location = check_off_synch_parsing_condition();
    if (*err_location != MAGIC_WORD)
    {
        eprintf(v, ERROR_COLOR "check_off_synch_parsing_condition fails" ANSI_COLOR_RESET " at bit# %d.\n", *err_location);
        return ERR_OFF_SYNCH_PARSING;
    }
    else
        eprintf(v, "Check off-sync-parsing condition:\t\t" OK_COLOR "OK\n" ANSI_COLOR_RESET);

    *err_location = check_aperiodicity_condition();
    if (*err_location != MAGIC_WORD)
    {
        eprintf(v, ERROR_COLOR "check_aperiodicity_condition fails" ANSI_COLOR_RESET " at bit# %d.\n", *err_location);
        return ERR_APERIODICITY;
    }
    else
        eprintf(v, "Check aperiodicity condition for long format:\t" OK_COLOR "OK\n" ANSI_COLOR_RESET);

    *err_location = check_undersampling_condition();
    if (*err_location)
    {
        eprintf(v, ERROR_COLOR "check_undersampling_condition fails" ANSI_COLOR_RESET".\n");
        return ERR_UNDER_SAMPLING;
    }
    else
        eprintf(v, "Check undersampling condition:\t\t\t" OK_COLOR "OK\n" ANSI_COLOR_RESET);

    return ERR_NO_ERR;
}

int telegram::check_alphabet_condition()
// checks whether all bits of the telegram can be converted from 11 to 10 bits
// shaped user bits are tested as well to be on the safe side (note that are made of alphabet words and could therefore be skipped).
// this is the "alphabet condition" in 4.3.2.5.2 of subset 36.
// returns the MAGIC_WORD if all is OK or the bit number of the 11-bit word in which the error was found
{
    int i, bit11;

//    for (i = (telegram->size / 11) - 1; i >= 0; i--)  // this checks all words, including the shaped data
    for (i=0; i<(N_CHECKBITS+N_ESB+N_SB+N_CB)/11-1; i++)
    // iterate over the lower 110 bits in the telegram, don't check the shaped data 
    {
        bit11 = contents.get_word(i * 11) & 0x07FF;  // get the next 11 bits from the telegram contents

        if (find11(bit11) == -1)
        {
            errcode = ERR_ALPHABET;
            return (i * 11);
        }
    }

    return MAGIC_WORD;
}

int telegram::check_off_synch_parsing_condition ()
/** checks the off_synch_parsing_condition in the test data (see subset 36, 4.3.2.5.3) for the given telegram
 * returns the bit number of the start of the sequence of "consecutive valid words" (cvw) that triggers a fail, or the MAGIC_WORD if all OK 
 * 
 * The following situations can occur:
 *  0) i = 0, 11, 22, ... : multiple of 11 (-> no action, is already checked in the alphabet condition check)
 *  1) i = 1, 12, 23, ... : i-1 is multiple of 11 -> max 2 valid consecutive words 
 *  2) i = 2, 13, 24, ... : i-1 and i+1 is no multiple of 11 -> max 6 (short) or 10 (long) valid consecutive words, depending on telegram size
 *  3) i = 3, 14, 25, ... : dito
 *   ..
 *  9) i = 9, 20, 31, ... : dito
 * 10) i = 10, 21, 32, .. : i+1 is multiple of 11, see case 1)
 * 11) i = 11, 22, 33, .. : see first line (no action)
 *
 * this check is performed using a greedy algorithm
 */
{
    unsigned int i, min_i, prev_i, max_i, i_offset, max_cvw;
    bool firstrun = true;

    //int offsets[] = { -1, 1, -2, 2, -3, 3, -4, 4, -5, 5 };    // Use Order 2, see ZHUO Pengs article, 3.3: i+/-2, 3, 4, 5 instead of i+2,3,4,5,6,7,8,9
    unsigned int offsets[] = { 10, 1, 9, 2, 8, 3, 7, 4, 6, 5 };

    for (i_offset = 0; i_offset < sizeof(offsets) / sizeof(offsets[0]); i_offset++)
    // iterate over the offsets
    {
        // determine the max allowed cvw for this case:
        if (i_offset <= 1) //( (i_offset == 0) || (i_offset == 1) )   // offsets -1, 1
            max_cvw = 2;
        else  // cases 2..9:
            if (size == s_long)
                max_cvw = 10;
            else
                max_cvw = 6;

        eprintf(VERB_ALL, HEADER_COLOR "\nOff-sync parsing condition check; offset=%d, max_cvw=%d\n" ANSI_COLOR_RESET, offsets[i_offset], max_cvw);
        eprintf(VERB_ALL, "SB=%d; ESB=%d\n", get_scrambling_bits(), get_extra_shaping_bits());
        print_contents_fancy(VERB_ALL);

        // initial conditions for greedy algorithm, starting at the lower bits:
        i = offsets[i_offset] + max_cvw * 11;   // i points to bit 0 of max_cvw+1
        min_i = offsets[i_offset];              // start of greedy step
        prev_i = i;                             // the max index of the previous step
        max_i = i;                              // the max index of the current step
        firstrun = true;                        // set to true if this is the first greedy step

        while (i < (unsigned int)size + (max_cvw+2) * 11 + offsets[i_offset])
        // determine the max_nvw using a greedy algorithm. use max_cvw+2 to get sufficient overlap with the first step at the wraparound
        {
            // print the current greedy state:
            if (check_verbose(VERB_ALL))
            {
                t_longnum_layout greedy_markings[] =
                {
                    {min_i, 11, ANSI_COLOR_GREEN},
                    {i % size, 11, ANSI_COLOR_YELLOW},
                    {0, 0, ""}
                };
                contents.print_fancy(VERB_ALL, 11, size, greedy_markings);
            }

            if (find11(contents.get_word_wraparound(size, i) & 0x7FF) == NO_TW)
            // a non-valid word was found, skip to the next
            {
                eprintf(VERB_ALL, "Non-valid word found @bit %d; prev_i=%d\n", i % size, prev_i);
                min_i = i + 11;                 // set min_i to the word following the current non-valid word
                i = min_i + max_cvw * 11;       // set i to the end of the possible valid word sequence
                prev_i = max_i;                 // remember the starting point of the previous iteration
                max_i = i;                      // remember the max i
                firstrun = false;               // no longer in first run
            }
            else
            // a valid word was found, step back to see if the previous word was non-valid
            {
                eprintf(VERB_ALL, "Valid word found @bit %d; prev_i=%d\n", i % size, prev_i);

                if (i == (firstrun ? min_i : (prev_i + 11)))
                // max nr of cvw's was found, exit with an error
                {
                    eprintf(VERB_ALL, ERROR_COLOR "\nNOK: off-synch-parsing condition fails; max cvw exceeded at bit #%d.\n" ANSI_COLOR_RESET, min_i);
                    eprintf(VERB_ALL, "offset=%d; max. nr. of consecutive valid words=%d.\n\n", offsets[i_offset], max_cvw);

                    errcode = ERR_OFF_SYNCH_PARSING;
                    return min_i;
                }

                i -= 11;
            }
        }

        eprintf(VERB_ALL, OK_COLOR "Off-synch-parsing condition OK for offset=%d.\n" ANSI_COLOR_RESET, offsets[i_offset]);
    }

    // no sequence of max_cvw found
    return MAGIC_WORD;
}

int telegram::calc_hamming_distance(t_word word1, t_word word2)
// calculates and returns the hamming distance between word1 and word2
// see https://en.wikipedia.org/wiki/Hamming_distance
{
    t_word temp; 
    int hamming_distance = 0;

    // light up the bits that differ by XOR-ing the two input words:
    temp = word1 ^ word2;
    // find the number of 1-bits in temp using the "population count" instruction (available from C++20) (small performance gain above doing this ourselves)
    hamming_distance = std::popcount(temp);// __popcnt(temp);
/*
    // alternatively, count the amount of set bits:
    while (temp)
    {
        hamming_distance += (temp & 1);
        temp >>= 1;
    }
*/
    return hamming_distance;

}

int telegram::check_aperiodicity_condition ()
/** checks the "Aperiodicity Condition for Long Format" from subset 36, 4.3.2.5.4
 * 
 * this check is only valid for the long telegram, so short telegrams are skipped.
 * 
 * for each i that is a multiple of 11:
 * take the two words (=22 bits) before i ("high"), compare them with two words @i-341 ("low"). Check that the Hamming distance (see https://en.wikipedia.org/wiki/Hamming_distance) >= 3.
 * also compare the high words with two words @i-341, with an offset of k = +1, -1, +2, -2, +3 and -3. Check that Hamming distance >= 2.
 * if the position of the lower two words is < 0, wraparound to the top of the telegram (see remark about wrap-around in subset 36, 4.3.2.5.1).
 * 
 * returns the location of the lower word at which the error occurs or returns the MAGIC_WORD if no error or if the telegram was short (-> no check).
 */
{
    unsigned int i, word_high, word_low, hammingdistance, err_start=MAGIC_WORD;
    int k;
    t_longnum_layout err_marking[3] = { 0 };
    err_marking[2].length = 0;   // initialise the last marking to 0

    // only for long telegrams, skip the short ones
    if (size == s_long)
    {        
        for (i=0; i<(unsigned int)size; i+=11)
        // iterate over the bits
        {
            word_high = contents.get_word_wraparound(size, i) & 0x3FFFFF;  // only use bits 0..21

            for (k=-3; k<=3; k++) 
            {
                word_low = contents.get_word_wraparound(size, i-341-k) & 0x3FFFFF;
                hammingdistance = calc_hamming_distance (word_high, word_low);  

                // check if the Hamming distance is OK:
                if ( ( (k == 0) && (hammingdistance < 3) ) ||
                     ( (k != 0) && (hammingdistance < 2) )
                   )  
                    err_start = i - 341 - k;

                eprintf(VERB_ALL, "i=%d\tk=%d\t", i, k);
                eprintf(VERB_ALL, "word_high="); print_bin (VERB_ALL, word_high, 22);
                eprintf(VERB_ALL, "\tword_low="); print_bin (VERB_ALL, word_low, 22);
                eprintf(VERB_ALL, "\tHamming distance=%d\t", hammingdistance);

                if (err_start != MAGIC_WORD)
                {
                    eprintf(VERB_ALL, ERROR_COLOR "NOK\n" ANSI_COLOR_RESET);
                    eprintf(VERB_ALL, "\nError in aperiodicity check (hamming distance=%d, i=%d, k=%d):\n", hammingdistance, i, k);
                    err_marking[0] = { i, 22, ANSI_COLOR_RED };
                    err_marking[1] = { (i-341-k>=0)?(i-341-k):(i-341-k+size), 22, ANSI_COLOR_RED };
                    contents.print_fancy(VERB_ALL, 11, size, err_marking);
                }
                else
                    eprintf(VERB_ALL, OK_COLOR "OK\n" ANSI_COLOR_RESET);

                if (err_start != MAGIC_WORD)
                    // error was found, return the location of the lower word:
                {
                    errcode = ERR_APERIODICITY;
                    return err_start;
                }
            }    
        }    
    }

    return MAGIC_WORD;    // short telegram or no errors
}

int telegram::get_max_run_valid_words(const longnum& ln)
/** Returns the maximum number of valid consecutive 11-bit words in telegram of length telegram->size (=n).
 * Starts at offsets i=[0..10] and for each offset, continues until n+30*11 bits have been checked.
 * Wraps around at n.
 * Because of this approach, there is no point in using offsets >= 11 as the check would repeat itself.
 * returns the maximum number of valid consecutive words found.
*/
{
    int offset, n_cvw, max_cvw = 0, i;
    t_word temp;

    for (offset = 0; offset < 11; offset++)
    {
        eprintf(VERB_ALL, "\nOffset=%d:\n", offset);

        n_cvw = 0;
        for (i = 0; i < size + 30 * 11; i += 11)
        {
            // find out the max nr of consecutive valid words for the current offset:
            temp = ln.get_word_wraparound(size, i + offset) & 0x7FF;
            if (find11(temp) != -1)
            // word was found in the list
            {
                eprintf(VERB_ALL, ANSI_COLOR_YELLOW);

                n_cvw++;
                if (n_cvw > max_cvw)
                    max_cvw = n_cvw;
            }
            else
                n_cvw = 0;    // reset the counter

                eprintf(VERB_ALL, "  i=%04d; word=", i + offset); print_bin(VERB_ALL, temp, 11); eprintf(VERB_ALL, ANSI_COLOR_RESET);

                if (i % 4 == 0)
                    eprintf(VERB_ALL, "\n");
        }
    }

    return max_cvw;
}

int telegram::check_undersampling_condition()
/** runs the "undersampling Condition" check (subset 36, 4.3.2.5.5).
 * Under-sample the telegram of length N bits with a factor k of 1, 2, 3 and 4 (and 2^k=2,4,8,16).
 * This yields a new telegram, in which the bits of the original telegram are "compressed" with a factor of 2^k.
 * Check that in these telegrams, the longest run (including wrap-around) of valid 11-bit words is 30. This must be valid for each starting bit i.
 *
 * What does such an under-sampled telegram look like?
 * factor 2: bit 0, 2, 4, 6, ...    (i=0)
 *           bit 1, 3, 5, 7, ...    (i=1)
 *           bit 2, 4, 6, 8, ...    (i=2, equal to i=0 <<1)
 *           bit 3, 5, 7, 9, ...    (i=3, equal to i=1 <<1)
 * factor 4: 0, 4, 8,  12, 16 (i=0)
 *           1, 5, 9,  13, 17 (i=1)
 *           2, 6, 10, 14, 18 (i=2)
 *           3, 7, 11, 15, 19 (i=3)
 *           4, 8, 12, 16, 20 (i=4, equal to i=0 <<1)
 * So, it is only needed to create undersampled telegrams for 0<=i<k and check that the maximum sequence of valid 11-bit words <= 30.
 * Note that the size of a telegram is always a multiple of 11; therefore the 11-bit word following the last 11-bit word in a wrapped-around telegram is equal to the first word.
 * For a short under-sampled telegram (of 31 words = 341 bits), this means that only 1 invalid word is needed in each word sequence starting at bit [0..10].
 * For a long telegram (of 93 words = 1023 bits), check that the max amount of consecutive valid words is 30 for 0<=n<93 words.
 * For a long telegram, also check that d = (position of first invalid word) + (N-position of last invalid word) <= 30 (because of wraparound).
 *   e.g.: ...F.....L... (pos in [0..N-1]; F=3, L=9, N=13 -> d=7)
 * 
 * Alternatively (and used below), for both telegram lengths N: check that the max amount of consecutive valid words is 30 for 0<=n<N+30 words (this includes wrap-around).
 * return 0 if all ok or ERR_UNDER_SAMPLING if an error was found.
 * 
 * Decision: no implementation of the greedy algorithm to keep this check as simple and robust as possible. The performance gain would be minimal.
 */
{
    int factor, i, j;
    int mrvw;
    longnum v;
    //t_telegram v = { 0 };
    //v_size = telegram->size;

    for (factor = 2; factor <= 16; factor *= 2)
        for (i = 0; i < factor; i++)
        {
            // create the undersampled telegram "v":
            for (j = 0; j < size; j++)
                v.set_bit(j, contents.get_bit((j * factor + i) % size));

            eprintf(VERB_ALL, "Original telegram:\n");
            contents.print_fancy(VERB_ALL, 11, size, NULL); 
            eprintf(VERB_ALL, "new telegram with offset=%d and undersampling factor=%d:\n", i, factor);
            v.print_fancy(VERB_ALL, 11, size, NULL); 

            mrvw = get_max_run_valid_words(v);
            if (mrvw > 30)
            { 
                eprintf(VERB_ALL, ERROR_COLOR "ERROR:" ANSI_COLOR_RESET " undersampling condition fails (MRVW = % d; offset=%d; factor k=%d\n", mrvw, i, factor);

                errcode = ERR_UNDER_SAMPLING;
                return ERR_UNDER_SAMPLING;
            }
        }

    return ERR_NO_ERR;
}

int telegram::check_control_bits()
// checks that the control bits are set to 001. Returns ERR_CONTROL_BITS if not, ERR_NO_ERR if they are identical.
// control bits are described in subset 36, 4.3.1.2.
{
    if (get_control_bits() != CONTROL_BITS)
    {
        errcode = ERR_CONTROL_BITS;
        return ERR_CONTROL_BITS;
    }
    else
        return ERR_NO_ERR;
}

int telegram::check_check_bits()
// checks the check bits (CRC) of the shaped bits
// re-calculates the check bits and verifies that the given check bits are OK
// returns ERR_NO_ERR if no error, ERR_CHECK_BITS if NOK
// calculation of check bits is described in subset 36, 4.3.2.4.
{
    telegram temp ("", size);
    longnum cb1, cb2;

    //init_telegram(&temp, telegram->size);
    temp = *this;
    temp.compute_check_bits_opt();  // temp now has the calculated check bits
    temp.get_checkbits(cb1);
    get_checkbits(cb2);

    cb1.print_bin(VERB_ALL); eprintf(VERB_ALL, " = temp\n");
    cb2.print_bin(VERB_ALL); eprintf(VERB_ALL, " = telegram\n");

    // compare the two values and return the error code:
    if (cb1 != cb2)
    {
        errcode = ERR_CHECK_BITS;
        return ERR_CHECK_BITS;
    }
    else
        return ERR_NO_ERR;
}

int telegram::check_shaped_telegram()
// performs all possible checks of the telegram
// prints out error messages if debug level is high enough
// stops checking when an error occurs
// returns 0 if no error, an appropriate error code if NOK
{
    int err;

    align(a_calc);

    // check the control bits (should be 001)
    err = check_control_bits();
    if (err)
    {
        eprintf(VERB_GLOB, ERROR_COLOR "check_control_bits fails" ANSI_COLOR_RESET".\n");
        return err;
    }
    else
        eprintf(VERB_GLOB, "Check control bits:\t\t\t\t" OK_COLOR "OK\n" ANSI_COLOR_RESET);

    // check the check bits:
    err = check_check_bits();
    if (err)
    {
        eprintf(VERB_GLOB, ERROR_COLOR "check_check_bits fails" ANSI_COLOR_RESET ".\n");
        return err;
    }
    else
        eprintf(VERB_GLOB, "Check check bits:\t\t\t\t" OK_COLOR "OK\n" ANSI_COLOR_RESET);

    // finally, perform the candidate-telegram tests:
    return perform_candidate_checks(VERB_GLOB, &err);
}

int telegram::check_shaped_deshaped()
// checks the shaped data in telegram against the unshaped data in the telegram
// returns 0 if no error, an appropriate error code if NOK
{
    longnum deshaped_data;

    align(a_calc);

    // deshape the telegram:
    deshape(deshaped_data);

    // compare the deshaped data with the original contents:
    if (deshaped_data == deshaped_contents)
        eprintf(VERB_GLOB, "Check deshaped contents against input: \t\t" OK_COLOR "OK\n" ANSI_COLOR_RESET);
    else
    {
        eprintf(VERB_GLOB, ERROR_COLOR"ERROR: unshaped content does not match original shaped content.\n" ANSI_COLOR_RESET);
        errcode = ERR_CONTENT;
        eprintf(VERB_GLOB, "Recreated user data (with error):\n");
        deshaped_data.print_fancy(VERB_GLOB, 8, number_of_userbits, NULL); 
        return ERR_CONTENT;
    }

    return ERR_NO_ERR;
}
