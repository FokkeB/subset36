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

#include "ss36.h"
#include "longnum.h"
#include "GF2.h"

void create_new_telegram(t_telegram** telegram, char* inputstr)
// creates and initialises a new telegram
{
    *telegram = malloc(sizeof(struct t_telegram));
    if (*telegram == NULL)
    {
        eprintf(VERB_QUIET, ERROR_COLOR"Error"ANSI_COLOR_RESET" allocating memory for a new telegram, parsing stops prematurely.\n");
        exit(ERR_MEM_ALLOC);
    }

    // initialise the new telegram:
    long_fill((*telegram)->contents, 0);
    long_fill((*telegram)->deshaped_contents, 0);
    (*telegram)->next = NULL;

    // reserve some space to permanently store the input string:
    int len = strlen(inputstr) + 1;
    (*telegram)->input_string = malloc(len);
    if ((*telegram)->input_string == NULL)
    {
        eprintf(VERB_QUIET, ERROR_COLOR"Error allocating memory for a the input string, parsing stops prematurely.\n"ANSI_COLOR_RESET);
        exit(ERR_MEM_ALLOC);
    }

    strcpy((*telegram)->input_string, inputstr);
}

void destroy_telegram(t_telegram* telegram)
// frees the memory of telegram
{
    free(telegram->input_string);
    free(telegram);
}

void destroy_telegrams(t_telegram* telegram)
// frees the list of telegrams
{
    t_telegram* next_telegram;

    while (telegram != NULL)
    {
        next_telegram = telegram->next;
        destroy_telegram(telegram);
        telegram = next_telegram;
    }
}

void init_telegram (t_telegram *telegram, int size)
// intialises the telegram struct, depending on the size of the telegram
// does not clear the contents
{
    telegram->errcode = ERR_NO_ERR;
    telegram->size = size;

    if (size == BITLENGTH_LONG_TELEGRAM)
    {
        telegram->number_of_userbits = USERBITS_IN_TELEGRAM_L;
        telegram->number_of_shapeddata_bits = N_SHAPEDDATA_L;
    }    
    else if (size == BITLENGTH_SHORT_TELEGRAM)
    {
        telegram->number_of_userbits = USERBITS_IN_TELEGRAM_S;
        telegram->number_of_shapeddata_bits = N_SHAPEDDATA_S;
    }
    else    // logical error, should never occur
        exit(ERR_LOGICAL_ERROR);
}

void set_checkbits (t_longnum contents, t_checkbits checkbits)
// sets the checkbits from the indicated array into telegram 
{
    long_write_at_location (contents, 0, checkbits, N_CHECKBITS);
}

void get_checkbits (t_longnum contents, t_longnum checkbits)
// gets the checkbits from the indicated array into telegram 
{
    int i;

    for (i=0; i<=2; i++)
        checkbits[i] = long_get_word (contents, i*BITS_IN_WORD);

    checkbits[2] &= 0x001FFFFF;  // mask the top 11 bits to get the 85 check bits
}

void set_extra_shaping_bits (t_longnum contents, t_esb esb)
// sets the extra shaping bits from the indicated array into the telegram 
{
    long_write_at_location (contents, N_CHECKBITS, &esb, N_ESB);
}

t_esb get_extra_shaping_bits (t_longnum contents)
// returns the 10 "extra shaping bits" from the telegram 
{
    return (t_esb)long_get_word (contents, N_CHECKBITS) & 0x3FF;
}

void set_scrambling_bits (t_longnum contents, t_sb sb)
// sets the scrambling bits from the indicated array into the telegram
{
    long_write_at_location (contents, N_CHECKBITS+N_ESB, &sb, N_SB);
}

t_sb get_scrambling_bits (t_longnum contents)
// returns the scrambling bits from the telegram 
{
    return (t_sb)(long_get_word (contents, N_CHECKBITS+N_ESB) & 0x0FFF);
}

void set_control_bits (t_longnum contents, int cb)
// sets the control bits from the indicated array into the telegram
{
    long_write_at_location (contents, N_CHECKBITS+N_ESB+N_SB, &cb, N_CB);
}

t_cb get_control_bits (t_longnum contents)
// returns the control bits from the telegram 
{
    return (t_cb)long_get_word (contents, N_CHECKBITS+N_ESB+N_SB) & 0x7;
}

void set_shaped_data (t_longnum contents, t_longnum sd)
// sets the shaped data from the indicated array into the telegram
{
    long_write_at_location (contents, N_CHECKBITS+N_ESB+N_SB+N_CB, sd, N_SHAPEDDATA_L);
}

void get_shaped_data (t_telegram *telegram, t_longnum sd)
// gets the n-bits (913 or 231) of shaped data (sd) from telegram 
{
    int i, n_sd = telegram->number_of_shapeddata_bits;

    int n_words = n_sd / BITS_IN_WORD;
    int n_bits = n_sd % BITS_IN_WORD;

    for (i=0; i<=n_words; i++)
        sd[i] = long_get_word (telegram->contents, i*BITS_IN_WORD+OFFSET_SHAPED_DATA);

    //sd[i-1] &= 0xFFFFFFFF>>(BITS_IN_WORD-n_bits);

    if (n_sd == N_SHAPEDDATA_L)
        sd[i-1] &= 0x0001FFFF;
    else
        sd[i-1] &= 0x0000007F;
}

void fill_user_bits_random (t_longnum userbits, int m)
// fills the m user bits with random values (for testing purposes).
// leaves the rest of the longnum alone.
{
    int i;

    for (i=0; i<m; i++)
        long_setbit (userbits, i, rand()&1);
}

void print_telegram_contents_fancy(int v, t_telegram* telegram)
// prints out the telegram in a fancy way (marking each part with a different color)
// uses verbosity level v
{
    eprintf(v, "Legend: "BITNUM_COLOR" (xxx)=bit number [0..N]; "ANSI_COLOR_RESET"%d bits shaped data; "ANSI_COLOR_MAGENTA"3 control bits; ", telegram->number_of_shapeddata_bits);
    eprintf(v, ANSI_COLOR_BLUE"12 scrambling bits; "ANSI_COLOR_YELLOW"10 extra scrambling bits; "ANSI_COLOR_GREEN"85 check bits.\n"ANSI_COLOR_RESET);
    print_longnum_fancy(v, telegram->contents, 11, telegram->size, telegram_coloring_scheme);
}

int count_telegrams(t_telegram* p_telegram)
// returns the number of telegrams in the linked list of telegrams pointed to by p_telegram
{
    int n_telegrams = 0;

    // first count the nr of telegrams (used to show the progress)
    while (p_telegram != NULL)
    {
        n_telegrams++;
        p_telegram = p_telegram->next;
    }

    return n_telegrams;
}

void determine_U_tick (t_longnum U, int m)
// calculates U'(k-1) from U and writes it to U (see subset 36, paragraph 4.3.2.2, step 1)
{
    int sum=0;

    for (int i=0; i<m/10; i++)
        sum += long_get_word(U, i*10);

    sum &= 0x3FF;

    long_write_at_location(U, m-10, &sum, 10);
}

t_S determine_S (t_sb sb)
// determine S and return it (see subset 36, paragraph 4.3.2.2, step 2)
{
    return (t_S)(sb*2801775573UL);
}

void scramble_user_data(t_S S, t_H H, t_longnum user_data_orig, t_longnum user_data_scrambled, int m)
// scrambles the data in user data (see subset 36, paragraph 4.3.2.2, step 3)
{
    int i, user_bit, t, sb;

    for (i=m-1; i>=0; i--)
    // iterate over the m user bits
    {
        user_bit = long_get_bit (user_data_orig, i);
        t = (int)(S>>31)&1;
        sb = t ^ user_bit;
        long_setbit(user_data_scrambled, i, sb); 

        S <<= 1;
        if (sb)
            S ^= H;
    }
}

void transform10to11 (t_longnum userdata, t_telegram *telegram) 
// performs the transformation of the scrambled user bits from 10 bits to 11 bits, 
// write the 11-bit words into telegram at the right place
// see subset 36, paragraph 4.3.2.3
{
    int i, p, q;

    for (i=(telegram->number_of_userbits/10)-1; i>=0; i--)
    // iterate over the user bits, get the transformed value and store it in telegram
    {
        p = long_get_word(userdata, i*10)&0x03FF;
        q = transformation_words[p];
        long_write_at_location(telegram->contents, i*11 + OFFSET_SHAPED_DATA, &q, 11);
    }    
}

int find11 (int val11)
// returns the index of val11 in the transformation words (returning its index, which is a val10).
// returns -1 if it does not exist.
// uses a lookup table that was generated from the transformation words.
{
    return lookup_table[val11];     
}

int transform11to10 (t_longnum userdata, t_telegram *telegram) 
// performs the transformation from 11 bits back to 10 bits; returns ERR_11_10_BIT if an error occurred (11-bit value not found in list) or 0 if no errors occurred
// reads transformed data from telegram contents (from OFFSET_SHAPED_DATA), writes the original user data to userdata starting at bit 0
// see subset 36, paragraph 4.3.2.3
{
    int i, j;           // indices
    int bit10, bit11;   // ints to store the temp 10-bit and 11-bits values

    for (i=(telegram->number_of_userbits/10)-1; i>=0; i--)
    // iterate over the array of 11-bit values, get the original 10-bit value and store it in userdata
    {
        j = 0;
        bit11 = long_get_word(telegram->contents, OFFSET_SHAPED_DATA+i*11)&0x07FF;  // get the next 11 bits from the telegram contents
        bit10 = find11 (bit11);

        if (bit10 == -1)
        {
            // this should never occur with a correctly encoded telegram:
            eprintf(VERB_ALL, "ERR: 11-bit value not found at i=%d; val=%o", i, bit11);
            return ERR_11_10_BIT;
        }

        long_write_at_location(userdata, i*10, &bit10, 10);
    }   

    return ERR_NO_ERR; 
}

void descramble (t_S S, t_H H, t_longnum user_data, int m)
// descrambles the scrambled data in user_data, writes the descrambled data to userdata_decoded
{
    int i;
    char descrambled_bit, t, scrambled_bit;
    //long_fill (userdata_decoded, 0);    

    for (i=m-1; i>=0; i--)
    {
        scrambled_bit = long_get_bit (user_data, i);
        t = (char)(S>>31)&1;  // get current output of the shiftregister
        descrambled_bit = t ^ scrambled_bit;
        long_setbit (user_data, i, descrambled_bit); 

        // shift the register 1 bit to the next step
        S = (S << 1);

        // apply H
        if (scrambled_bit)
            S ^= H;
    }
}

void calc_first_word (t_longnum U, int m)
// calculates the first 10-bit word in the descrambled U, which is known in subset 36 (see 4.3.2.2) as U'(k-1)
// m is the amount of user bits (830 or 210), so k=m/10 (0..83/21)
// U'(k-1) = sum(U(k-1..0)) = U(k-1) + sum(U(k-2..0))
// therefore: U(k-1) = U'(k-1) - sum (U(k-2..0)).
// U(k-1) is written in the last 10-bit word of U, replacing U'(k-1)
// The MOD 2^10 part is not needed in the calculations because only the last 10 bits are saved
{
    unsigned int sum=0, temp;

    // calculate sum(U(k-2..0)):
    for (int i=0; i<=(m/10-2); i++)
        sum += long_get_word(U, i*10) & 0x3FF;

    temp = long_get_word(U, m-10) - sum;
    long_write_at_location(U, m-10, &temp, 10);
}

void compute_check_bits (t_telegram *telegram)
// compute the check bits as described in SS36, 4.3.2.4
// input: a filled telegram
// output: the checkbits in bit 0..84 of the telegram
// performs a sanity check on the calculations
{
    t_longnum f, g, fg, contents, quotient, remainder, sanitycheck, checkbits;  // ,f

    // first initialise all the variables:
    long_fill (f, 0);
    long_fill (g, 0);
    long_fill (fg, 0);
    long_fill (contents, 0);
    long_fill (quotient, 0);
    long_fill (remainder, 0);
    long_fill (checkbits, 0);
    long_fill (sanitycheck, 0);

    // clear the lower 85 bits [0..84] from the input telegram:
    telegram->contents[0] = 0;  // [0..31]
    telegram->contents[1] = 0;  // [32..63]
    telegram->contents[2] &= 0xFFF00000;  // [63..84]

    // copy the telegram contents to not-destroy the original values
    long_copy (contents, telegram->contents);

    eprintf(VERB_ALL, HEADER_COLOR"\nCalculating check bits:\n"ANSI_COLOR_RESET);
    eprintf(VERB_ALL, FIELD_COLOR"input telegram:\t"ANSI_COLOR_RESET); print_longnum_bin (VERB_ALL, contents);

    // determine the inputs to the calculation (kept here instead of moving them to ss36.h):
    if (telegram->size == s_long) // BITLENGTH_LONG_TELEGRAM)
    {
        // polynomials for a long telegram:
       f[0] = 0b11011011111;  
        g[0] = 0b11010101001000111011101000010011; 
        g[1] = 0b01110011100110100111101000101110; 
        g[2] = 0b101110001000;
//        fg[0] = 0xC063B091;
//        fg[1] = 0x890C6F72;
//        fg[2] = 0x3EC171;
    }
    else
    {
        // polynomials for a short telegram:
        f[0] = 0b10110101011;
        g[0] = 0b11001010010010100011110001001011;
        g[1] = 0b10010000110000101111111011110111;
        g[2] = 0b100111110111;
//        fg[0] = 0x921B6D65;
//        fg[1] = 0xD8775795;
//        fg[2] = 0x2BB94;
    }

    // calculate f*g:
    GF2_multiply (f, g, fg);   // fg is defined instead of f to save some clock ticks and memory 

    // divide the telegram by f*g, yielding quotient and remainder:
    GF2_division (contents, fg, quotient, remainder);

    // add (=xor) g to the remainder -> checkbits!:
    long_xor (g, remainder, checkbits);
    eprintf(VERB_ALL, FIELD_COLOR"\nmasked 0..84:\t"ANSI_COLOR_RESET); print_longnum_bin (VERB_ALL, contents);
    eprintf(VERB_ALL, FIELD_COLOR"\nf*g:\t\t"ANSI_COLOR_RESET); print_longnum_bin (VERB_ALL, fg);
    eprintf(VERB_ALL, FIELD_COLOR"\nquotient:\t"ANSI_COLOR_RESET); print_longnum_bin (VERB_ALL, quotient);
    eprintf(VERB_ALL, FIELD_COLOR"\nremainder:\t"ANSI_COLOR_RESET); print_longnum_bin (VERB_ALL, remainder);
    eprintf(VERB_ALL, FIELD_COLOR"\ncheckbits:\t"ANSI_COLOR_RESET); print_longnum_bin (VERB_ALL, checkbits);
    eprintf(VERB_ALL, "\n");

    // perform a sanity check by calculating the original bits from the quotient, remainder and g:

    // subtract g from the checkbits to get the remainder (this is a xor-operation, just like addition)
    long_xor (g, checkbits, sanitycheck);

    // sanitycheck should now be identical to the remainder, show an error message and exit if this is not the case:
    if (!long_cmp (sanitycheck, remainder))
    {
        eprintf(VERB_QUIET, ERROR_COLOR"ERROR"ANSI_COLOR_RESET" - sanity check of remainder is NOK\n"); 
        return;
    }

    // multiply the quotient with fg, store in sanitycheck1:
    GF2_multiply (quotient, fg, sanitycheck);

    // and add the remainder:
    long_xor (remainder, sanitycheck, sanitycheck);

    // see if the check is OK (should always be the case):
    if (!long_cmp(sanitycheck, contents))
        eprintf(VERB_QUIET, ERROR_COLOR"ERROR"ANSI_COLOR_RESET" - sanity check is NOK\n");
    else
        eprintf(VERB_ALL, OK_COLOR"Sanity check of checkbits is OK\n"ANSI_COLOR_RESET);

    // finally copy the checkbits into the lower 85 bits of telegram (we cleared these above):
    telegram->contents[0] = checkbits[0];    // bits 0..31
    telegram->contents[1] = checkbits[1];    // bits 32..63
    telegram->contents[2] |= checkbits[2];   // bits 64..84
}

int set_next_sb_esb(t_telegram* telegram)
/** Sets the next scrambling bits and extra scrambling bits
* Because of the Alphabet condition, these two words need to come from the "transformation words".
* word #10 (starting at bit#99) exists of two parts: [b10..b8] are the control bits (always 001) and [b7..b0] are the first 8 scrambling bits.
* word #9 (starting at bit#88) also exists of two parts: [b10..b7] are the last 4 scrambling bits and [b6..b0] are the first 7 extra scrambling bits
* the last 3 bits of the ESB are at word #8 (starting at bit #77) and are [b10..b8]
*
* So, updating the SB and ESB goes as follows:
* word#10: pick a transformation word of which the three high bits are 001. These are the transformation words in the range of [00401 .. 00776] (bin: 00 100 000 001 to 00 111 111 110), or with index [#104 .. #260];
* word#9: pick any transformation word;
* word#8: iterate over the possible values [0..7] for bits [b10..b8]
*
* this function determines the next SB and ESB and writes these directly to telegram
* if run for the first time for a certain telegram, set telegram->word9 to -1 and telegram->word10 to 0.
*/
{
    t_word temp = 0x07;

    temp = long_get_word(telegram->contents, N_CHECKBITS) & 7;  // isolate the current three high bits in word#8

    // determine the new values for ESB and SB:
    if ((telegram->word9 != -1) && (temp < 7))  // check if temp will overflow in the next instruction, but only if this is not the first run
        temp++;  // increase the lower three bits of ESB if it does not overflow
    else
    // find the next transformation word to put in word#9. Todo (or not): find one that starts with [SB3..SB0], in order not to change the SB
    {
        temp = 0;
        telegram->word9++;
        if (telegram->word9 >= N_TRANS_WORDS)
        // overflow of word9, increase word10
        {
            telegram->word9 = 0;       // reset tf_9

            // find the next TF that starts with 0b001 xxxx xxxx:
            do
                telegram->word10++;
            while ((telegram->word10 < N_TRANS_WORDS) &&
                ((transformation_words[telegram->word10] & 0x700) != 0x100)   
                );

            if (telegram->word10 >= N_TRANS_WORDS)
            {  // this should only veeeeery rarely happen: 10^-100 (see subset 36, A1.1.1)
                eprintf(VERB_QUIET, ERROR_COLOR"ERROR:"ANSI_COLOR_RESET" No valid combination of Scrambling Bits and Extra Shaping Bits found for the telegram below. \n");
                eprintf(VERB_QUIET, "Please make a minor change in the telegram contents and try again.See Subset - 036.\n");
                eprintf(VERB_QUIET, "Also: please send a copy of the input telegram to the writer of this program (fokke@bronsema.net). Thanks :-)\n");
                eprintf(VERB_QUIET, "Contents of input telegram: %s\n", telegram->input_string);
                exit(ERR_SB_ESB_OVERFLOW);
            }
        }
    }

    temp = (temp & 0x1FFC007) | (transformation_words[telegram->word9] << 3);      // mask out bits [4..15] and fill with tf<<3. Also resets the lower three bits.
    temp = (temp & 0x0003FFF) | (transformation_words[telegram->word10] << 14);    // mask out and set word#10
    long_write_at_location(telegram->contents, 85, &temp, 25);                     // write the cb+esb+sb to the telegram

    return ERR_NO_ERR;
}

int test_candidate_telegram(int v, t_telegram* telegram, int* err_location)
// performs all the checks in subset 36, paragraph 4.3.2.5 "Testing Candidate Telegrams".
// returns one of the subset 36 error codes, or 0 if all OK, stops checking after occurence of the first error.
// sets err_location to point to the start bit of the error (if this makes sense).
{
    *err_location = check_alphabet_condition(telegram);
    if (*err_location != MAGIC_WORD)
    {
        eprintf(v, ERROR_COLOR"check_alphabet_condition fails"ANSI_COLOR_RESET" at word starting with bit#%d.\n", *err_location);
        return ERR_ALPHABET;
    }
    else
        eprintf(v, "Check alphabet condition:\t\t\t"OK_COLOR"OK\n"ANSI_COLOR_RESET);

    *err_location = check_off_synch_parsing_condition(telegram);
    if (*err_location != MAGIC_WORD)
    {
        eprintf(v, ERROR_COLOR"check_off_synch_parsing_condition fails"ANSI_COLOR_RESET" at bit# %d.\n", *err_location);
        return ERR_OFF_SYNCH_PARSING;
    }
    else
        eprintf(v, "Check off-sync-parsing condition:\t\t"OK_COLOR"OK\n"ANSI_COLOR_RESET);

    *err_location = check_aperiodicity_condition(telegram);
    if (*err_location != MAGIC_WORD)
    {
        eprintf(v, ERROR_COLOR"check_aperiodicity_condition fails"ANSI_COLOR_RESET" at bit# %d.\n", *err_location);
        return ERR_APERIODICITY;
    }
    else
        eprintf(v, "Check aperiodicity condition for long format:\t"OK_COLOR"OK\n"ANSI_COLOR_RESET);

    *err_location = check_undersampling_condition(telegram);
    if (*err_location)
    {
        eprintf(v, ERROR_COLOR"check_undersampling_condition fails"ANSI_COLOR_RESET".\n");
        return ERR_UNDER_SAMPLING;
    }
    else
        eprintf(v, "Check undersampling condition:\t\t\t"OK_COLOR"OK\n"ANSI_COLOR_RESET);

    return ERR_NO_ERR;
}

void shape(t_telegram* telegram)
// encodes the userdata (deshaped_contents) in the telegram (filling contents).
// recalculate if the checks fail.
{
    t_longnum UD_scrambled = { 0 }, Utick = { 0 };
    int err_location = 0, errs_found = 0, seven = 7, err;
    t_sb current_sb = 0;

    long_copy(Utick, telegram->deshaped_contents);
    determine_U_tick(Utick, telegram->number_of_userbits);
    eprintf(VERB_ALL, "\nU'=\n"); print_longnum_bin(VERB_ALL, Utick);
    telegram->word9 = -1;  // will be set to 0 in the first run of set_next_sb_esb
    telegram->word10 = 0;

    // try to find a correctly shaped telegram
    do
    {
        // find the next combination of sb/esb:
        set_next_sb_esb(telegram);   // also sets the three control bits to 001

        if (get_scrambling_bits(telegram->contents) != current_sb)
        // scrambling bits have changed, recalculate the user data.
        // if scrambling bits haven't changed, there is no need to perform this calculation
        {
            current_sb = get_scrambling_bits(telegram->contents);

            // calculate scramble the user data into UD_scrambled
            scramble_user_data(determine_S(current_sb), H, Utick, UD_scrambled, telegram->number_of_userbits);
            eprintf(VERB_ALL, "\nscrambled data =\n"); print_longnum_bin(VERB_ALL, UD_scrambled);

            // then "shape" the user data (replace 10 bit words with 11 bit words) into telegram:
            transform10to11(UD_scrambled, telegram);
            eprintf(VERB_ALL, "\10 to 11 =\n"); print_longnum_bin(VERB_ALL, telegram->contents);
        }

        // compute the check bits (CRC):
        compute_check_bits(telegram);

        eprintf(VERB_ALL, "\nChecking new telegram:\n");
        print_telegram_contents_fancy(VERB_ALL, telegram);

        // now see if the packet is "well formed", make another run if not.
        err = test_candidate_telegram(VERB_ALL, telegram, &err_location);

        if ((err == ERR_OFF_SYNCH_PARSING) || (err == ERR_APERIODICITY))
            if (err_location >= OFFSET_SHAPED_DATA)
            // error sequence is completely in the shaped user data, it is therefore pointless to update the ESB
            // solution: set word9 and word10 so that the next word10 is selected in the next run
            {
                long_write_at_location(telegram->contents, 85, &seven, 3);  // reset the lower three bits of the ESB
                telegram->word9 = N_TRANS_WORDS;
            }

    } while (err);
}

void deshape(t_telegram* telegram, t_longnum userdata)
// deshapes the shaped data in the telegram into userdata (which could be part of telegram)
{
    t_S S = 0;

    eprintf(VERB_ALL, "DESHAPING:\n");
    transform11to10(userdata, telegram);
    eprintf(VERB_ALL, "\n11 to 10 =\n"); print_longnum_bin(VERB_ALL, userdata);

    S = determine_S(get_scrambling_bits(telegram->contents));
    descramble(S, H, userdata, telegram->number_of_userbits);
    eprintf(VERB_ALL, "Descrambled:\n"); print_longnum_bin(VERB_ALL, userdata);
    calc_first_word(userdata, telegram->number_of_userbits);
    eprintf(VERB_ALL, "Calc 1st word:\n"); print_longnum_bin(VERB_ALL, userdata);

    eprintf(VERB_ALL, "FINISHED DESHAPING\n");
}

int check_alphabet_condition(t_telegram* telegram)
// checks whether *all* data (including non-user bits) in the telegram can be converted from 11 to 10 bits
// this is the "alphabet condition" in 4.3.2.5.2 of subset 36.
// returns the MAGIC_WORD if all is OK or the bit number of the 11-bit word in which the error was found
{
    int i, bit11;

    for (i = (telegram->size / 11) - 1; i >= 0; i--)
    // iterate over the bits in the telegram
    {
        bit11 = long_get_word(telegram->contents, i * 11) & 0x07FF;  // get the next 11 bits from the telegram contents

        if (find11(bit11) == -1)
        {
            telegram->errcode = ERR_ALPHABET;
            return (i * 11);
        }
    }

    return MAGIC_WORD;
}

int check_off_synch_parsing_condition (t_telegram *telegram)
/** checks the off_synch_parsing_condition in the test data (see subset 36, 4.3.2.5.3) for the given telegram
 * returns the bit number of the start of the sequence of words that trigger a fail, or the MAGIC_WORD if all OK 
 * 
 *  0) i = 0, 11, 22, ... : multiple of 11 (no action)
 *  1) i = 1, 12, 23, ... : i-1 is multiple of 11 -> max 2 valid consecutive words 
 *  2) i = 2, 13, 24, ... : i-1 and i+1 is no multiple of 11 -> max 6 (short) or 10 (long) valid consecutive words, depending on telegram size
 *  3) i = 3, 14, 25, ... : dito
 *   ..
 *  9) i = 9, 20, 31, ... : dito
 * 10) i = 10, 21, 32, .. : i+1 is multiple of 11, see case 1
 * 11) i = 11, 22, 33, .. : see first line (no action)
 *
 * this is checked for every i in [0 .. #bits in telegram]. If i >= bits in telegram - 10, there will be a wraparoud to the beginning of the telegram
 * this continues until i == (#bits in telegram-1) - 11 (case 10)
 * cvw = consecutive valid words
*/
{
    int i, offset, max_cvw, n_cvw, temp, err_local, err_overall = MAGIC_WORD, start_err = 0;
    t_longnum_layout err_marking[2] = { 0 };
    err_marking[1] = no_colors;  // initialise the last marking to 0

    for (offset = 1; offset <= 10; offset++)
    // iterate over the cases
    {
        // determine the max cvw for this case:
        if ( (offset == 1) || (offset == 10) )
            max_cvw = 2;
        else  // cases 2..9:
            if (telegram->size == s_long) // BITLENGTH_LONG_TELEGRAM)
                max_cvw = 10;
            else
                max_cvw = 6;
                
        eprintf(VERB_ALL, HEADER_COLOR"\noffset=%d\n"ANSI_COLOR_RESET, offset);

        n_cvw = 0;
        err_local = 0;
        for (i=offset; i<telegram->size; i+=11)
        {
        // find out the max nr of consecutive valid 11-bit words for the current offset: todo: check if this cvw-work should be in a separate function (also used in other checks)
            temp = long_get_word_wraparound(telegram->contents, telegram->size, i) & 0x7FF;
            if (find11(temp) != -1)
            // word was found in the list
            {
                eprintf(VERB_ALL, ANSI_COLOR_YELLOW);

                n_cvw++;
                if (n_cvw > max_cvw)
                    err_local = 1;
            }    
            else
            {
                n_cvw = 0;          // reset the counter
                start_err = i+11;   // store the start of the next sequence
            }

            eprintf(VERB_ALL, "  i=%04d; word=", i); print_bin (VERB_ALL, temp, 11); eprintf(VERB_ALL, ANSI_COLOR_RESET); 
            if (i%4==0)
                eprintf(VERB_ALL, "\n");

            if (err_local) 
            {   
                eprintf(VERB_ALL, ERROR_COLOR"\nNOK: off-synch-parsing condition fails; max cvw exceeded at bit #%d.\n"ANSI_COLOR_RESET, i);
                eprintf(VERB_ALL, "offset=%d; max. nr. of consecutive valid words=%d but found %d.\n\n", offset, max_cvw, n_cvw);

                err_marking[0] = (t_longnum_layout){ .start = start_err, .length = n_cvw * 11, .color = ANSI_COLOR_RED };
                print_longnum_fancy(VERB_ALL, telegram->contents, 11, telegram->size, err_marking);

                err_overall = start_err;
                telegram->errcode = ERR_OFF_SYNCH_PARSING;
                break;
            }
        } 

        if (!err_local) 
            eprintf(VERB_ALL, OK_COLOR" (OK)\n"ANSI_COLOR_RESET);
    }

    return err_overall;
}

int calc_hamming_distance (t_word word1, t_word word2, int n)
// calculates and returns the hamming distance between word1 and word2
// only checks the first n bits (n=1..NMAX)
// see https://en.wikipedia.org/wiki/Hamming_distance
{
    int i, hamming_distance = 0;

    if (n > sizeof(t_word)*8)
        n = sizeof(t_word)*8;

    for (i=0; i<n; i++)
        if ( (word1 & (1<<i)) != (word2 & (1<<i)) )
            hamming_distance++;

    return hamming_distance;
}

int check_aperiodicity_condition (t_telegram *telegram)
/** checks the "Aperiodicity Condition for Long Format" from subset 36, 4.3.2.5.4
 * 
 * this check is only valid for the long telegram, short telegrams are skipped.
 * 
 * for each i that is a multiple of 11:
 * take the two words (=22 bits) before i ("high"), compare them with two words @i-341 ("low"). Check that Hamming distance (see https://en.wikipedia.org/wiki/Hamming_distance) >= 3.
 * also compare the high words with two words @i-341, with an offset of k = +1, -1, +2, -2, +3 and -3. Check that Hamming distance >= 2.
 * if the position of the lower two words is < 0, wraparound to the top of the telegram (see remark about wrap-around in subset 36, 4.3.2.5.1).
 * 
 * returns the location of the lower word at which the error occurs or returns the MAGIC_WORD if no error or if the telegram was short (-> no check).
 */
{
    int i, k, word_high, word_low, hammingdistance, err_start=MAGIC_WORD;
    t_longnum_layout err_coloring[3] = { 0 };
    char* red = ANSI_COLOR_RED;

    // only for long telegrams, skip the short ones
    if (telegram->size == s_long)
    {        
        for (i=0; i<telegram->size; i+=11)
        // iterate over the bits
        {
            word_high = long_get_word_wraparound(telegram->contents, telegram->size, i) & 0x3FFFFF;

            for (k=-3; k<=3; k++) 
            {
                word_low = long_get_word_wraparound(telegram->contents, telegram->size, i-341-k) & 0x3FFFFF;
                hammingdistance = calc_hamming_distance (word_high, word_low, 22);  // calculate the Hamming distance over the first 22 bits

                // then check if the Hamming distance is OK:
                if ( ( (k == 0) && (hammingdistance < 3) ) ||
                     ( (k != 0) && (hammingdistance < 2) )
                   )  
                    err_start = i - 341 - k;

                eprintf(VERB_ALL, "i=%d\tk=%d\t", i, k);
                eprintf(VERB_ALL, "word_high="); print_bin (VERB_ALL, word_high, 22);
                eprintf(VERB_ALL, "\tword_low="); print_bin (VERB_ALL, word_low, 22);
                eprintf(VERB_ALL, "\tHamming distance=%d\t", hammingdistance);

                if (err_start != MAGIC_WORD)
                    eprintf(VERB_ALL, ERROR_COLOR"NOK\n"ANSI_COLOR_RESET);
                else
                    eprintf(VERB_ALL, OK_COLOR"OK\n"ANSI_COLOR_RESET);

                if (err_start != MAGIC_WORD)
                    // error was found, return the location of the lower word:
                {
                    err_coloring[0].start = i;
                    err_coloring[0].length = 22;
                    strcpy(err_coloring[0].color, red);
                    err_coloring[1].start = (i-k-341<0)?i-k-341+1023:i-k-341;
                    err_coloring[1].length = 22;
                    strcpy(err_coloring[1].color, red);
                    err_coloring[2] = no_colors;

                    print_longnum_fancy(VERB_GLOB, telegram->contents, 11, telegram->size, err_coloring);
                    eprintf(VERB_GLOB, ERROR_COLOR"NOK:"ANSI_COLOR_RESET" Aperiodicity check fails. i=%d;k=%d\n", i, k);
                    eprintf(VERB_GLOB, "bit #%d:\t", i); print_bin(VERB_GLOB, word_high, 22); eprintf(VERB_GLOB, "\n");
                    eprintf(VERB_GLOB, "bit #%d:\t", i-k-341); print_bin(VERB_GLOB, word_low, 22); eprintf(VERB_GLOB, "\n");
                    eprintf(VERB_GLOB, "Hamming distance=%d\n", hammingdistance);

                    telegram->errcode = ERR_APERIODICITY;
                    return err_start;
                }
            }    
        }    
    }

    return MAGIC_WORD;    // short telegram or no errors
}

int get_max_run_valid_words(t_telegram* telegram)
/** Returns the maximum number of valid consecutive 11-bit words in telegram of length telegram->size.
 * Starts at offsets i=[0..10] and continues until n+30*11 bits have been checked.
 * Wraps around at n.
 * Because of this approach, there is no point in using offsets >= 11 as the check would repeat itself.
 * returns the maximum number of valid consecutive words found.
*/
{
    int offset, n_cvw, max_cvw = 0, err_local = 0, i;
    t_word temp;
    t_longnum_layout err_marking[2] = { 0 };
    err_marking[1] = no_colors;  // initialise the last marking to 0

    for (offset = 0; offset < 11; offset++)
    {
        eprintf(VERB_ALL, "\nOffset=%d:\n", offset);

        n_cvw = 0;
        err_local = 0;
        for (i = 0; i < telegram->size + 30 * 11; i += 11)
        {
            // find out the max nr of consecutive valid words for the current offset:
            temp = long_get_word_wraparound(telegram->contents, telegram->size, i + offset) & 0x7FF;
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

void undersample_telegram(t_telegram* b, t_telegram* v, int k, int offset)
// undersamples telegram b with factor k, starting at offset, fills telegram to its size
// variable names are taken from subset 36, 4.3.2.5.5.
{
    int i;

    for (i = 0; i < v->size; i++)
        long_setbit(v->contents, i, long_get_bit(b->contents,
            (i * k + offset) % b->size));
}

int check_undersampling_condition(t_telegram* telegram)  
/** runs the "undersampling Condition" check (subset 36, 4.3.2.5.5).
 * Undersample the telegram of length N bits with a factor k of 1, 2, 3 and 4 (and 2^k=2,4,8,16).
 * This yields a new telegram, in which the bits of the original telegram are "compressed" with a factor of 2^k.
 * Check that in these telegrams, the longest run (including wraparound) of valid 11-bit words is 30. This must be valid for each starting bit i.
 *
 * What does such an undersampled telegram look like?
 * factor 2: bit 0, 2, 4, 6, 8, 10, ...    (i=0)
 *           bit 1, 3, 5, ...    (i=1)
 *           bit 2, 4, 6, ...    (i=2)
 *           ...
 *           bit 10, 12, 14, ... (i=10)
 *           bit 11, 13, 15, ... (i=11)
 * @offset 11, the next 11-bit word after offset=0 follows in case of factor 2.
 * So, instead of checking each offset i (also >11), only the first 11 need to be checked (for factor 2).
 * With factor 4: the first 22, with factor 8: 44, 16 -> 88
 * However, because a sequence of 30 valid 11-bit words can also be created from a wraparound,
 * one undersampled telegrams + 30 11-bit words will need to be calculated. 
 *
 * note: if #bits in telegram / 2^k is less than 30, only 1 invalid word is needed to be OK
 *
 * return 0 if all ok or ERR_UNDER_SAMPLING if an error was found.
 */
{
    int k = 0, offset;
    int mrvw;
    t_telegram v = { 0 };
    v.size = telegram->size;

    for (k = 2; k <= 16; k *= 2)
        for (offset = 0; offset < 11 * k; offset++)
        {
            undersample_telegram(telegram, &v, k, offset);

            eprintf(VERB_ALL, "Original telegram:\n");
            print_longnum_fancy(VERB_ALL, telegram->contents, 11, telegram->size, &no_colors);
            eprintf(VERB_ALL, "new telegram with offset=%d and undersampling factor=%d:\n", offset, k);
            print_longnum_fancy(VERB_ALL, v.contents, 11, telegram->size, &no_colors);

            mrvw = get_max_run_valid_words(&v);
            if (mrvw > 30)
            {
                eprintf(VERB_ALL, ERROR_COLOR"ERROR:"ANSI_COLOR_RESET" undersampling condition fails (MRVW = % d; offset=%d; factor k=%d\n", mrvw, offset, k);

                telegram->errcode = ERR_UNDER_SAMPLING;
                return ERR_UNDER_SAMPLING;
            }
        }

    return ERR_NO_ERR;
}

int check_control_bits(t_telegram* telegram)
// checks that the control bits are set to 001. Returns ERR_CONTROL_BITS if not, ERR_NO_ERR if they are identical.
// control bits are described in subset 36, 4.3.1.2.
{
    if (get_control_bits(telegram->contents) != CONTROL_BITS)
    {
        telegram->errcode = ERR_CONTROL_BITS;
        return ERR_CONTROL_BITS;
    }
    else
        return ERR_NO_ERR;
}

int check_check_bits(t_telegram* telegram)
// checks the check bits (CRC) of the shaped bits
// re-calculates the check bits and verifies that the given check bits are OK
// returns ERR_NO_ERR if no error, ERR_CHECK_BITS if NOK
// calculation of check bits is described in subset 36, 4.3.2.4.
{
    t_telegram temp;
    t_longnum cb1 = { 0 }, cb2 = { 0 };

    init_telegram(&temp, telegram->size);
    long_copy(temp.contents, telegram->contents);
    compute_check_bits(&temp);  // temp now has the calculated check bits
    get_checkbits(temp.contents, cb1);
    get_checkbits(telegram->contents, cb2);

    // compare the two values and return the error code:
    if (!long_cmp(cb1, cb2))
    {
        eprintf(VERB_GLOB, "Check bits in telegram: \n");
        print_longnum_bin(VERB_GLOB, cb2);
        eprintf(VERB_GLOB, "calculated check bits: \n");
        print_longnum_bin(VERB_GLOB, cb1);

        telegram->errcode = ERR_CHECK_BITS;
        return ERR_CHECK_BITS;
    }
    else
        return ERR_NO_ERR;
}

int check_shaped_telegram(t_telegram* telegram)
// performs all possible checks of the telegram
// prints out error messages if debug level is high enough
// stops checking when an error occurs
// returns 0 if no error, an appropriate error code if NOK
{
    int err;
    telegram->errcode = ERR_NO_ERR;

    // check the control bits (should be 001)
    err = check_control_bits(telegram);
    if (err)
    {
        eprintf(VERB_GLOB, ERROR_COLOR"check_control_bits fails"ANSI_COLOR_RESET".\n");
        return err;
    }
    else
        eprintf(VERB_GLOB, "Check control bits:\t\t\t\t"OK_COLOR"OK\n"ANSI_COLOR_RESET);

    // check the check bits:
    err = check_check_bits(telegram);
    if (err)
    {
        eprintf(VERB_GLOB, ERROR_COLOR"check_check_bits fails"ANSI_COLOR_RESET".\n");
        return err;
    }
    else
        eprintf(VERB_GLOB, "Check check bits:\t\t\t\t"OK_COLOR"OK\n"ANSI_COLOR_RESET);

    // finally, perform the candidate-telegram tests:
    return test_candidate_telegram(VERB_GLOB, telegram, &err);
}