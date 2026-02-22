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
    word9 = -1;                 // initial values to start of the calculation
    word10 = FIRST_TW_001 - 1;  // point to the transformation word before the first one that starts with 001 (control bits)

    if (inputstr.length() > 0)  
        parse_input(inputstr);
    else
        set_size(newsize);
}

telegram::~telegram(void)
// destructor
{
    eprintf(VERB_FLOW, "Destroyed telegram with address=%p\n", this);
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
// If this is a short telegram: turn the userdata into a long telegrams by adding just the right amount of 0xFF's to the end and set the size-parameters to the long-values
// Note: does not touch the shaped user data, so this could lead to inconsistent telegrams. Recalculate the shaped user data afterwards if needed.
{
    int i;

    if (size == s_long) 
    // do nothing if this already is a long telegram
        return;

    // make sure the telegram is aligned correctly before changing it:
    align(a_calc);

    eprintf(VERB_ALL, "Short telegram before make_long:\n");
    deshaped_contents.print_fancy(VERB_ALL, 16, number_of_userbits, NULL);

    // set size to long:
    set_size(s_long);

    // shift left to align the content of the short telegram with the long format:
    deshaped_contents <<= (N_USERBITS_L - N_USERBITS_S);

    // padd the trailing (830-210=620) bits with 1's
    // first the whole words:
    for (i = 0; i < (N_USERBITS_L - N_USERBITS_S) / BITS_IN_WORD; i++)
        deshaped_contents[i] = 0xFFFFFFFF;  // 4 bytes

    // then the remaining few bits:
    for (i = 0; i< (N_USERBITS_L - N_USERBITS_S) % BITS_IN_WORD; i++)
        deshaped_contents.set_bit((N_USERBITS_L - N_USERBITS_S) - i - 1, 1);

    eprintf(VERB_ALL, "2Converted to long telegram:\n");
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
    int arrsize = -1;                      // #bytes in the temp array, set default to -1 for error handling

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

    if (arrsize < 0)
        errcode = ERR_INPUT_ERROR;

    // set the alignment of the telegram to encoding:
    alignment = a_enc;
}

string telegram::get_csv_output_line(const string format, bool error_only, bool include_sb_esb, bool include_id, char csv_separator)
// Returns a string with the telegram that can be used in a csv-file
// Includes a trailing \n
// Note that the returned string could be empty (in case the telegram has no error and only telegrams with errors should be shown)
{
    string csv_separators = "", output_result = "", line;
    int count = 0, i;
    t_balise_id balise_id;
    
    output_result.reserve(300);  // to prevent some heap reallocations

    if (!(error_only && (errcode == ERR_NO_ERR)))
    // skip this telegram if (only errors should be outputted and telegram has no error)
    {
        if (errcode == ERR_INPUT_ERROR)
        // the telegram was not parsed because of an error in the input data
        // output the original line, CSV_SEPARATOR(s), error code
        {
            // Try to add some CSV_SEPARATORs to stick to the output format as much as possible

            // count the number of CSV_SEPARATORs included in the string:
            for (i = 0; i < input_string.length(); i++)
                if (input_string[i] == csv_separator)
                    count++;

            // determine the correct number of CVS_SEPARATORs and add them to the end (pointed to by i-1):
            if ((count <= 1) || (input_string.at((size_t)(i - 1)) != csv_separator))
                csv_separators.push_back(csv_separator);

            if (count == 0)
                csv_separators.push_back(csv_separator);

            // add the line and the separators to the output result:
            output_result += input_string + csv_separators + to_string(ERR_INPUT_ERROR);
        }
        else
        // output the line to a csv-format
        {
            align(a_enc);

            // output the deshaped contents followed by a ;
            deshaped_contents.sprint_hex(line, number_of_userbits);
            output_result += line;
            output_result += csv_separator;

            // output the shaped contents, depending on format, followed by a ;
            if (format == "hex")
                contents.sprint_hex(line, size);
            else
                contents.sprint_base64(line, size);

            output_result += line + csv_separator;

            // add the error code and the newline:
            output_result += to_string(errcode);

            // add the SB and ESB if requested:
            if (include_sb_esb)
            {
                align(a_calc);
                output_result += csv_separator + to_string(get_scrambling_bits()) + csv_separator +
                    to_string(get_extra_shaping_bits()) +
                    csv_separator + to_string(word9) + csv_separator + to_string(word10);
            }

            if (include_id)
            {
                balise_id = get_balise_id();
                output_result += csv_separator + to_string(balise_id.NID_C) + csv_separator + to_string(balise_id.NID_BG) + csv_separator + to_string(balise_id.N_PIG);
            }
        }

        // finalise the line with a newline character
        output_result += "\n";
    }

    return output_result;
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

t_esb telegram::get_extra_shaping_bits(void) const
// returns the 10 "extra shaping bits" from the telegram 
{
    return (t_esb)contents.get_word(N_CHECKBITS) & 0x3FF;
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

t_balise_id telegram::get_balise_id()
// Returns the balise id (NID_C, NID_BG, N_PIG) of the current user data.
// See subset-026 8.4.2.1, starting at bit 0, which is the high bit of the balise contents (which is bit#size-1) and counting down:
// N_PIG is bits [8..10]
// NID_C is bits [24..33]
// NID_BG is bits [34..47]
// This function requires that the unscrambled content is set, will return all zeros if this is not the case.
{
    t_balise_id balise_id = { 0,0,0 };

    if (deshaped_contents.get_order() == 0)
        // no user data, return all 0's
        return balise_id;

    align(a_calc);

    balise_id.N_PIG = deshaped_contents.get_word(number_of_userbits - 1 - 11) & 0x7;
    balise_id.NID_C = deshaped_contents.get_word(number_of_userbits - 1 - 34) & 0x3FF;
    balise_id.NID_BG = deshaped_contents.get_word(number_of_userbits - 1 - 48) & 0x3FFF;

    eprintf(VERB_GLOB, "Balise id = %d_%d_%d\n", balise_id.NID_C, balise_id.NID_BG, balise_id.N_PIG);

    return balise_id;
}

void telegram::print_contents_fancy(int v) const
// prints out the telegram in a fancy way (marking each part with a different color)
// uses verbosity level v
{
    return_if_silent(v);

    eprintf(v, "Legend: " BITNUM_COLOR " (xxx)=bit number [0..N]; " ANSI_COLOR_RESET "%d bits shaped data; " ANSI_COLOR_MAGENTA "3 control bits (%d); ", number_of_shapeddata_bits, (int)get_control_bits());
    eprintf(v, ANSI_COLOR_BLUE "12 scrambling bits (%d); " ANSI_COLOR_YELLOW "10 extra shaping bits (%d); " ANSI_COLOR_GREEN "85 check bits.\n" ANSI_COLOR_RESET, (int)get_scrambling_bits(), (int)get_extra_shaping_bits());
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

void telegram::determine_U_tick (longnum& Utick) const
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

int telegram::scramble_transform_check_user_data(t_S S, t_H H, const longnum& user_data_orig)   
// scrambles the data in user_data_orig into contents (see subset 36, paragraph 4.3.2.2, step 3)
// checks the ERR_OFF_SYNCH_PARSING during scrambling and returns the error as soon as such an error occurred
// Note: according to ZHUO Peng, checking the "Aperiodicity Condition for Long Format" is a very small optimisation (1%, see description @ step 4) and is therefore checked outside this function 
{
    int i;
#ifndef USE_SCRAMBLE_LOOKUP
    int j, user_bit, t, sb, m_index = number_of_userbits;
#else
    t_S lfsr = S;
    t_word indata, index;
#endif // USE_SCRAMBLE_LOOKUP

    t_word lookatword, val11;
    unsigned int val10;

    // vars needed for greedy algorithm:
    int offset_index = 0;
    int cvw_offsets[] = { 1, 10, 9, 2 , 8, 3, 7, 4, 6, 5 };  
    int i_start = number_of_userbits / 10 - 1;   // start one word before the number_of_userbits
    int i_last_nvw[] = { i_start, i_start, i_start, i_start, i_start, i_start, i_start, i_start, i_start, i_start };
    int i_vw, max_cvw = 2, n_cvw = sizeof(cvw_offsets) / sizeof(cvw_offsets[0]);


    for (i = i_start; i >= 0; i--)
    // outer loop running over the 10-bit words, starting with the last word
    {
#ifndef USE_SCRAMBLE_LOOKUP
        val10 = 0; //check = 0;
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
#else
        indata = user_data_orig.get_word(i*10) & 0x3FF;   
        index = indata ^ (lfsr >> 22);
        val10 = scramble_lookup_table[index].out;
        lfsr = scramble_lookup_table[index].reg ^ (lfsr << 10);
#endif // USE_SCRAMBLE_LOOKUP

        // 10 bits calculated, find the corresponding transformation word and write it to the correct position:
        val11 = transform_word10_to_word11(val10);

        contents.write_at_location(i * 11 + OFFSET_SHAPED_DATA, &val11, 11);
        // print the current status:
        eprintf(VERB_ALL, "#userbits=%d, i=%d\n", number_of_userbits, i);
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

                    if (transform_word11_to_word10(lookatword) == NO_TW)
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

int telegram::transform11to10 (longnum& userdata) const
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
        bit10 = transform_word11_to_word10(bit11);

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


// Code below is generated by Claude and Gemini and adapted by FokkeB where needed.
// First steps (generation of lookup table) were removed as the lookup tables are predefined in lookup_tables.h.

/* ───────────────────────────────────────────────────────────────────
 * 3.  Byte extraction helper
 * ───────────────────────────────────────────────────────────────────
 *
 * Extracts an 8-bit byte from a longnum at a given bit position
 * (the byte's LSB is at `bitpos`).  Works directly on the
 * underlying uint32_t array to avoid per-bit function calls.
 *
 * Includes a bounds check to prevent reading past the end of
 * the uint32_t[WORDS_IN_LONGNUM] array when a byte straddles
 * the last word boundary.
 * ─────────────────────────────────────────────────────────────────── */

static inline uint8_t extract_byte(const uint32_t* words, int bitpos)
{
    const int wi = bitpos >> 5;
    const int bi = bitpos & 31;
    uint32_t val = words[wi] >> bi;
    if (bi > 24 && wi + 1 < WORDS_IN_LONGNUM)
        val |= words[wi + 1] << (32 - bi);
    return (uint8_t)(val & 0xFFu);
}


/* ───────────────────────────────────────────────────────────────────
 * 4.  Table-driven CRC division
 * ───────────────────────────────────────────────────────────────────
 *
 * Processes `n_bytes` consecutive 8-bit chunks from the telegram,
 * starting at `base_bitpos` and working from the highest byte
 * downward.  The byte grid is:
 *
 *   byte 0:            bits [base_bitpos .. base_bitpos+7]
 *   byte 1:            bits [base_bitpos+8 .. base_bitpos+15]
 *   ...
 *   byte n_bytes-1:    bits [base_bitpos+(n_bytes-1)*8 .. ...]
 *
 * Bytes are processed from byte (n_bytes-1) down to byte 0 so that
 * the most-significant message bits enter the CRC first.
 *
 * CRITICAL: Two consecutive calls that together cover a range must
 * share the same base_bitpos (byte grid origin) to avoid overlapping
 * or skipping bits.
 * ─────────────────────────────────────────────────────────────────── */

static void crc_table_divide_bytes(
    const uint32_t* words,
    int base_bitpos,
    int n_bytes,
    const t_crc_entry table[256],
    uint32_t r[3])
{
    for (int i = n_bytes - 1; i >= 0; i--)
    {
        const int bp = base_bitpos + i * 8;
        const uint8_t msg_byte = extract_byte(words, bp);

        const uint8_t idx = ((uint8_t)(r[2] >> 13)) ^ msg_byte;

        r[2] = ((r[2] << 8) | (r[1] >> 24)) & 0x001FFFFFu;
        r[1] = (r[1] << 8) | (r[0] >> 24);
        r[0] = (r[0] << 8);

        r[0] ^= table[idx].w[0];
        r[1] ^= table[idx].w[1];
        r[2] ^= table[idx].w[2];
    }
}

/* ───────────────────────────────────────────────────────────────────
 * 5.  Replacement for compute_check_bits()
 * ───────────────────────────────────────────────────────────────────
 *
 * Cache strategy
 * ──────────────
 * The byte grid is anchored at N_CHECKBITS (bit 85), giving bytes:
 *   byte 0: bits [85..92]     ← check-bit overflow + ESB
 *   byte 1: bits [93..100]    ← ESB + SB overlap
 *   byte 2: bits [101..108]   ← SB + CB + shaped data
 *   ...
 *
 * The ESB occupies bits 85..94, spanning bytes 0 and 1.  The first
 * byte that is entirely ABOVE the ESB is byte 2 (bit 101).
 *
 * We split the division at byte 2:
 *   Phase 1 (cached):  bytes 2 .. (n_bytes-1)   → shaped data + SB + CB
 *   Phase 2 (always):  bytes 0 .. 1             → ESB region
 *
 * Both phases use the SAME byte grid (anchored at bit 85) so there
 * is no overlap or gap between them.
 *
 * The cache is valid when the scrambling bits (and therefore the
 * shaped user data above them) haven't changed.  On a cache hit
 * only Phase 2 runs (2 table lookups).
 *
 * Cache sentinel
 * ──────────────
 * intermediate_sb is initialised to 0 in the class, which is a
 * valid 12-bit SB value.  To prevent false cache hits on a fresh
 * telegram, we store (SB + 1) as the cache key.  This shifts the
 * valid key range to 1..4096, so the default 0 never matches.
 * ─────────────────────────────────────────────────────────────────── */
 
void telegram::compute_check_bits(void)
{
    int phase1_bytes = (number_of_shapeddata_bits + N_CB + N_SB) / 8;       // Total bytes in the message (above the check-bit region) 
    int phase2_bytes = (N_ESB+N_SB)/8;     // Number of bytes in Phase 2: bytes 0 and 1 (bits 85..100).
    uint32_t r[3] = { 0, 0, 0 };
    longnum g;
    const t_crc_entry* table;

    // --- Zero the check-bit region (bits 0..84) ---
    contents[0] = 0;
    contents[1] = 0;
    contents[2] &= 0xFFE00000u;

    eprintf(VERB_ALL, HEADER_COLOR "\nCalculating check bits (table):\n" ANSI_COLOR_RESET);
    eprintf(VERB_ALL, FIELD_COLOR "Input telegram:\t" ANSI_COLOR_RESET);
    print_contents_fancy(VERB_ALL);

    // --- Select polynomial g and lookup table ---
    if (size == s_long)
    {
        g[0] = 0b11010101001000111011101000010011;
        g[1] = 0b01110011100110100111101000101110;
        g[2] = 0b101110001000;
        table = crc_table_long;
    }
    else
    {
        g[0] = 0b11001010010010100011110001001011;
        g[1] = 0b10010000110000101111111011110111;
        g[2] = 0b100111110111;
        table = crc_table_short;
    }

    // --- Phase 1: bytes 2..(n-1), cached by scrambling bits ---
    // Use SB + 1 for comparison: intermediate_sb stores (SB + 1) so
    // that the default-initialized value of 0 never matches a real
    // SB (whose (SB + 1) ranges from 1 to 4096).

    const t_word current_sb_key = get_scrambling_bits() + 1;

    if (current_sb_key == intermediate_sb)
    {
        r[0] = intermediate_remainder[0];
        r[1] = intermediate_remainder[1];
        r[2] = intermediate_remainder[2];// &0x001FFFFFu;
        eprintf(VERB_ALL, "Reused table cache for SB key=%d.\n", intermediate_sb);
    }
    else
    {
        crc_table_divide_bytes(&contents[0],
            N_CHECKBITS + phase2_bytes * 8,
            phase1_bytes,
            table, r);

        intermediate_remainder[0] = r[0];
        intermediate_remainder[1] = r[1];
        intermediate_remainder[2] = r[2] & 0x001FFFFFu;
        intermediate_sb = current_sb_key;
        eprintf(VERB_ALL, "Stored table cache for SB key=%d.\n", intermediate_sb);
    }

    // --- Phase 2: bytes 0..1 (bits 85..100), always computed ---
    crc_table_divide_bytes(&contents[0], N_CHECKBITS, phase2_bytes, table, r);

    // XOR with g and put into telegram:
    contents[0]  = r[0] ^ g[0];
    contents[1]  = r[1] ^ g[1];
    contents[2] |= r[2] ^ g[2];

    eprintf(VERB_ALL, OK_COLOR "Check bits computed (table).\n" ANSI_COLOR_RESET);
}

// end AI code


int telegram::set_next_sb_esb(void)
/* sets the next scrambling bits (and resets the extra shaping bits) by selecting the next transformation word(s)
* if run for the first time for a certain telegram, set telegram->word9 to -1.
* word9 contains the last 4 scrambling bits and the first 7 extra shaping bits
* word10 contains the 3 control bits and the first 8 scrambling bits
* both word9 and word10 need to be transformation words
* Returns an overflow-error or ERR_NO_ERR
*/
{
    t_word cb_sb_esb = 0, old9 = 0; 

    // first do a range check on word9:
    if (word9 >= N_TRANS_WORDS)
    // something went horribly wrong, die
    {
        printf("Error: word9=%d, > N_TRANS_WORDS, exitting.\n", word9);
        exit(ERR_LOGICAL_ERROR);
    }

    if (word9 == -1)
    // initial value 
    {
        word9 = 0;
        word10++;
    }
    else
    {
        old9 = transform_word10_to_word11(word9) & 0b11110000000; // isolate the current first four bits of word9 (=last four scrambling bits)

        // find the next word9 with different high four bits and check that word9 does not overflow
        while ((word9 < N_TRANS_WORDS - 1) && (old9 == (transform_word10_to_word11(++word9) & 0b11110000000)));

        if (word9 == N_TRANS_WORDS - 1)
        // word9 is at the end of the list, no need to search for the next value, skip to the next word10
        {
            word9 = 0;
            word10++;

            if (word10 > LAST_TW_001)
            // word10 overflowed in the code above, return an error
            {
                errcode = ERR_SB_ESB_OVERFLOW;
                return ERR_SB_ESB_OVERFLOW;
            }
        }
    }

    cb_sb_esb += (transform_word10_to_word11(word9) << 3);         // fill bits [4..15] with tf<<3, clear the lower three bits
    cb_sb_esb += (transform_word10_to_word11(word10) << 14);       // set word#10
    set_cb_sb_esb(cb_sb_esb); // write the cb+esb+sb to the telegram, reset the three lower ESB-bits

    eprintf(VERB_ALL, "Set_next_sb_esb: Updated CB, SB and ESB to: word10=%d; tw10=o%o; word9=%d; tw9=o%o; \n", word10, transform_word10_to_word11(word10), word9, transform_word10_to_word11(word9));
    
    //printf("S: SB=%d; ESB=%d\n", get_scrambling_bits(), get_extra_shaping_bits());
 
    return ERR_NO_ERR; 
}

bool telegram::set_next_esb(void)
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
        if ((word9 < N_TRANS_WORDS-1) && ((transform_word10_to_word11(word9) & 0b11110000000) == (transform_word10_to_word11(word9 + 1) & 0b11110000000)))
        // find the next t.w. that starts with the same four bits. 
            word9++;
        else
        // no new transformation word available with the same high 4 bits -> new scrambling bits are needed. Return false.
            return false;
    }

    esb += (transform_word10_to_word11(word9) << 3);         // fill bits [4..15] with tf<<3, keep the lower three bits
    set_extra_shaping_bits(esb);

    eprintf(VERB_ALL, "set_next_esb: Updated CB, SB and ESB to:\n"); print_contents_fancy(VERB_ALL);
    eprintf(VERB_ALL, "word10=%d; tw10=o%o; word9=%d; tw9=o%o; \n", word10, transform_word10_to_word11(word10), word9, transform_word10_to_word11(word9));
    //printf("E: SB=%d; ESB=%d\n", get_scrambling_bits(), get_extra_shaping_bits());

    return true;
}

void telegram::shape(void)
// Encodes the userdata (deshaped_contents) in the telegram (filling contents).
// Recalculate with different settings (sb/esb) if the checks fail and repeat until the checks don't fail.
// Checks the "off-synch-parsing-condition" (and not the "aperiodicity condition for long format") while 
// shaping the user data in order to find out illegal telegrams ASAP.
// See subset 36 for more information
{
    longnum Utick, temp;
    int err_location = 0, errs_found = 0, err, n_iter = 0; 
    t_word current_sb = 0, new_sb = 0;
    bool inc_sb = (word9 == -1); // true if run for the first time for this telegram
    align(a_calc);
    Utick = deshaped_contents;
    determine_U_tick(Utick);
    eprintf(VERB_ALL, "\nU'=\n"); Utick.print_bin(VERB_ALL);
    bool err_in_user_data;

    do
    // repeat until we find a correct telegram or there is an overflow of sb/esb
    {
        if (inc_sb)
        // increase the scrambling bits if run for the first time or if the next loop fails to find a new ESB
        // this loop should be skipped if not all the possible ESB's/CRC's were checked yet when calculating all telegrams
        {
            do
            // calculate the next scrambling bits and create shaped user bits that pass a large part of the Off-Synch-Parsing Condition
            {
                n_iter++;
                if (set_next_sb_esb() == ERR_SB_ESB_OVERFLOW)
                {
                    eprintf(VERB_ALL, "Overflow of SB/ESB occurred.\n");
                    return;
                }
            } while (scramble_transform_check_user_data(determine_S(), H, Utick) != ERR_NO_ERR);
        }

        inc_sb = true;

        do
        // compute the check bits (CRC), perform checks and update the extra shaping bits until a correct solution is found.
        // if none can be found, start from the top with new scrambling bits
        {
            compute_check_bits();
            n_iter++;

            eprintf(VERB_ALL, "\nChecking new telegram:\n");
            print_contents_fancy(VERB_ALL);

            // now see if the packet is "well formed", make another run if not.
            err = perform_candidate_checks(VERB_ALL, &err_in_user_data);

            if ((err == ERR_OFF_SYNCH_PARSING) || (err == ERR_APERIODICITY))
                if (err_in_user_data)
                // error sequence is located completely in the shaped user data, it is therefore pointless to update the ESB
                // solution: set last three bits of ESB to 111, so the next word 9 and if necessary word10 are selected in the next run
                { 
                    contents.write_at_location(N_CHECKBITS, 0b111, 3);  // set the lower three bits of the ESB to 111 
                }
        } while (err && set_next_esb());
    } while (err);
    
    eprintf(VERB_GLOB, "Shaped the telegram in %d combinations of scrambling bits and extra shaping bits.\n", n_iter);
}


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

int telegram::perform_candidate_checks(int v, bool* err_in_user_data) // int* err_location)
// Performs all the checks in subset 36, paragraph 4.3.2.5 "Testing Candidate Telegrams".
// Returns one of the subset 36 error codes, or 0 if all OK, stops checking after occurence of the first error.
// Sets err_in_user_data if the error is located within the user data area.
// Perform the checks in the optimal order, see elimination rates in ZHUO Pengs article, Step 7.
// Always check the complete telegram. This leads to some redundant checks (the alphabet condition and the off-synch parsing check
// are partially performed during the calculations), but it seems like a good idea to check everything once the calculations are done.
{
    int err_location;

    err_location = check_alphabet_condition();
    if (err_location != MAGIC_WORD)
    {
        eprintf(v, ERROR_COLOR "check_alphabet_condition fails" ANSI_COLOR_RESET " at word starting with bit#%d.\n", err_location);
        return ERR_ALPHABET;
    }
    else
        eprintf(v, "Check alphabet condition:\t\t\t" OK_COLOR "OK\n" ANSI_COLOR_RESET);

    err_location = check_off_synch_parsing_condition(err_in_user_data);
    if (err_location != MAGIC_WORD)
    {
        eprintf(v, ERROR_COLOR "check_off_synch_parsing_condition fails" ANSI_COLOR_RESET " at bit# %d.\n", err_location);
        return ERR_OFF_SYNCH_PARSING;
    }
    else
        eprintf(v, "Check off-sync-parsing condition:\t\t" OK_COLOR "OK\n" ANSI_COLOR_RESET);

    err_location = check_aperiodicity_condition(err_in_user_data);
    if (err_location != MAGIC_WORD)
    {
        eprintf(v, ERROR_COLOR "check_aperiodicity_condition fails" ANSI_COLOR_RESET " at bit# %d.\n", err_location);
        return ERR_APERIODICITY;
    }
    else
        eprintf(v, "Check aperiodicity condition for long format:\t" OK_COLOR "OK\n" ANSI_COLOR_RESET);

    err_location = check_undersampling_condition();
    if (err_location)
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
// shaped user bits are tested as well to be on the safe side (note that they are made of alphabet words and could therefore be skipped).
// this is the "alphabet condition" in 4.3.2.5.2 of subset 36.
// returns the MAGIC_WORD if all is OK or the bit number of the 11-bit word in which the error was found
{
    int i, bit11;

    for (i = (size / 11) - 1; i >= 0; i--)  // this checks all words, including the shaped data
//    for (i=0; i<(N_CHECKBITS+N_ESB+N_SB+N_CB)/11-1; i++)
    // iterate over the lower 110 bits in the telegram, don't check the shaped data 
    {
        bit11 = contents.get_word(i * 11) & 0x07FF;  // get the next 11 bits from the telegram contents

        if (transform_word11_to_word10(bit11) == NO_TW)
        {
            errcode = ERR_ALPHABET;
            return (i * 11);
        }
    }

    return MAGIC_WORD;
}

int telegram::check_off_synch_parsing_condition (bool *err_in_user_data)
/** checks the off_synch_parsing_condition in the test data (see subset 36, 4.3.2.5.3) for the given telegram
 * returns the bit number of the start of the sequence of "consecutive valid words" (cvw) that triggers a fail, or the MAGIC_WORD if all OK 
 * if the error is completely located in the user data area, err_in_user_data is set to true
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
    int i, min_i, prev_i, max_i, i_offset, max_cvw;
    bool firstrun = true;

    //int offsets[] = { -1, 1, -2, 2, -3, 3, -4, 4, -5, 5 };    // Use Order 2, see ZHUO Pengs article, 3.3: i+/-2, 3, 4, 5 instead of i+2,3,4,5,6,7,8,9
    int offsets[] = { 10, 1, 9, 2, 8, 3, 7, 4, 6, 5 };

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

        while (i < size + (max_cvw+2) * 11 + offsets[i_offset])
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

            if (transform_word11_to_word10(contents.get_word_wraparound(size, i) & 0x7FF) == NO_TW)
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
                // max nr of cvw's was found, exit with an error and show the error in the telegram
                {
                    eprintf(VERB_ALL, ERROR_COLOR "\nOff-synch-parsing condition fails\n" ANSI_COLOR_RESET "Max cvw exceeded at bit # % d; ", min_i);
                    eprintf(VERB_ALL, "offset=%d; max. nr. of consecutive valid words=%d.\n", offsets[i_offset], max_cvw);
                    t_longnum_layout error_markings[] =
                    {
                        {min_i, 11*(max_cvw+1), ANSI_COLOR_RED},
                        {0, 0, ""}
                    };
                    contents.print_fancy(VERB_ALL, 11, size, error_markings);

                    *err_in_user_data = ( (min_i >= OFFSET_SHAPED_DATA) &&         // the lower part of the error is in the user data area
                                          (min_i + 11 * (max_cvw + 1) < size)      // and the higher part of the error as well
                                        ); 

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
// Calculates and returns the hamming distance between word1 and word2
// See https://en.wikipedia.org/wiki/Hamming_distance
// The use of a lookup table could be considered, but word1 and word2 are 22-bit words. 
// This leads either to a 4MB lookup table or to getting the distance for two 11-bit words from a 2kB table and adding these. 
// The latter option does not yield enough performance gain, therefore this distance is calculated on the fly.
// The "popcount" instruction available from C++20 does also not yield enough performance gain but is kept here for possible future use.
{
    t_word temp; 
    int hamming_distance = 0;

    // light up the bits that differ by XOR-ing the two input words:
    temp = word1 ^ word2;

    // find the number of 1-bits in temp using the "population count" instruction (available from C++20)
//    return std::popcount(temp);// __popcnt(temp);

    // count the amount of set bits:
    while (temp)
    {
        hamming_distance += (temp & 1);
        temp >>= 1;
    }

    return hamming_distance;
}

int telegram::check_aperiodicity_condition (bool *err_in_user_data)
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
 * sets err_in_user_data to true if the error is located completely in the user data area, false if not.
 */
{
    int i, word_high, word_low, hammingdistance, err_start=MAGIC_WORD;
    int k;
    t_longnum_layout err_marking[3] = { 0 };
    err_marking[2].length = 0;   // initialise the last marking to 0

    // only for long telegrams, skip the short ones
    if (size == s_long)
    {        
        for (i=0; i<size; i+=11)
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

                    *err_in_user_data = ( (err_start >= OFFSET_SHAPED_DATA) &&   // the lower part of the error is in the user data area
                                          (i + 21 < size)                        // the higher part as well
                                        );
                    return err_start;
                }
            }    
        }    
    }

    return MAGIC_WORD;    // short telegram or no errors
}

int telegram::get_max_run_valid_words(const longnum& ln) const
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
            if (transform_word11_to_word10(temp) != NO_TW)
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

    temp = *this;
    temp.compute_check_bits();  // temp now has the calculated check bits
    temp.get_checkbits(cb1);
    get_checkbits(cb2);

    cb1.print_bin(VERB_GLOB); eprintf(VERB_GLOB, " = temp\n");
    cb2.print_bin(VERB_GLOB); eprintf(VERB_GLOB, " = telegram\n");

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
    bool err_in_user_data = false;

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
    return perform_candidate_checks(VERB_GLOB, &err_in_user_data);
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
