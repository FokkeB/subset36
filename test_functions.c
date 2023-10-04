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

//#include <stdio.h>        // printf
//#include <stdlib.h>       // random numbers
#include <time.h>           // timer for randomisation
#include "longnum.h"
#include "GF2.h"        
#include "ss36.h"
#include "parse_input.h"

void random_fill_longnum (t_longnum longnum)
// fills longnum with random data
{
    for (int i=0; i<WORDS_IN_LONGNUM; i++)
        longnum[i] = ( (rand()<<16) | rand());
}

void fill_user_bits_random(t_longnum userbits, int m)
// fills the m user bits with random values (for testing purposes).
// clears the rest of the longnum.
{
    int i;

    // clear the userbits:
    long_fill(userbits, 0);

    // set the whole words:
    for (i = 0; i <= m / BITS_IN_WORD; i++)
        userbits[i] = (rand() << 16) | rand();

    // clear the bits > m:
    i = 0;
    long_write_at_location(userbits, m, &i, BITS_IN_WORD);
}

void print_result (int result)
// prints out the result (OK=0; NOK!=0)
{
    if (result)
        printf (ERROR_COLOR"NOK (#errs=%d)\n"ANSI_COLOR_RESET, result);            
    else
        printf (OK_COLOR"OK\n"ANSI_COLOR_RESET);     
}

int test_copy_cmp (int* err_total)
// tests the function long_copy and long_cmp, returns error
{
    t_longnum ln1, ln2;
    int err = 0;

    random_fill_longnum (ln1);
    long_copy (ln2, ln1);

    if (!long_cmp(ln1, ln2))
        err += 1;

    *err_total += err;
    return (err);
}

int test_xor (int* errs)
// tests the long_xor function, returns #errors
{
    t_longnum ln1, ln2, ln3;
    int i, err=0;

    // prepare the test data:
    random_fill_longnum (ln1);
    random_fill_longnum (ln2);
    long_xor (ln1, ln2, ln3);

    // iterate over all the bits, verify the XOR-operation
    for (i=0; i<BITS_IN_LONGNUM; i++)
        if (  (long_get_bit(ln1, i)^long_get_bit(ln2, i)) != long_get_bit(ln3, i))
        {
            err++;
            eprintf (VERB_GLOB, "\nERR, XOR fails @bit %d:\n", i);
            eprintf (VERB_GLOB, "ln1 =\t\t"); long_print_bin (VERB_GLOB, ln1);
            eprintf (VERB_GLOB, "ln2 =\t\t"); long_print_bin (VERB_GLOB, ln2);
            eprintf (VERB_GLOB, "ln1^ln2 =\t"); long_print_bin (VERB_GLOB, ln3);
        }

    *errs += err;
    return err;
}

int test_shift_right (int* errs)
// tests the SHR function, returns #errors
{
    t_longnum ln_orig, ln2;
    int i, j, new_errs=0;

    // prepare the test data:
    random_fill_longnum (ln_orig);

    // SHR over the complete longnum in increasing steps of 1
    for (i=1; i< BITS_IN_LONGNUM; i*=2)
    {
        long_copy (ln2, ln_orig);        
        long_shiftright (ln2, i);

        // check the shifted bits:
        for (j=0; j<BITS_IN_LONGNUM-i; j++)
            if (long_get_bit (ln_orig, i+j) != long_get_bit (ln2, j))
            {
                new_errs++;
                eprintf (VERB_GLOB, "\nERR, SHR step 1 fails:\n");
                long_print_bin(VERB_GLOB, ln_orig);
//                    printf (">>%d:; BITS_IN_LONGNUM=%d; j=%d\n", i, (int)BITS_IN_LONGNUM, j);
                long_print_bin (VERB_GLOB, ln2);
            }

        // check that the higher bits are 0 (0 shifted in)
        // only check bits that are within range of [0..BITS_IN_LONGNUM-1] 
        for (j=BITS_IN_LONGNUM-i; j<BITS_IN_LONGNUM; j++)
            if ( (j>=0) && (long_get_bit (ln2, j) != 0) )
            {
                new_errs++;
                eprintf (VERB_GLOB, "\nERR, SHR step 2 fails:\n");
                long_print_bin(VERB_GLOB, ln_orig);
                eprintf (VERB_GLOB, ">>%d:\n", i);
                long_print_bin (VERB_GLOB, ln2);
            }

        errs += new_errs;
        //new_errs = 0;                       
    }

    *errs += new_errs;
    return new_errs;
}

int test_shift_left (int* errs)
// tests the SHL function, returns #errors
{
    t_longnum ln_orig, ln2;
    int i, j, new_errs=0;

    // prepare the test data:
    random_fill_longnum (ln_orig);

    // SHL over the complete longnum in increasing steps, including i>N:
    for (i=1; i<=BITS_IN_LONGNUM*2; i*=2)
    {
        long_copy (ln2, ln_orig);        
        long_shiftleft (ln2, i);

        // check the shifted bits:
        for (j=i; j<BITS_IN_LONGNUM; j++)
            if (long_get_bit (ln_orig, j-i) != long_get_bit (ln2, j))
            {
                new_errs++;
                eprintf (VERB_GLOB, "\nERR, SHL step 1 fails:\n");
                long_print_bin(VERB_GLOB, ln_orig);
                eprintf (VERB_GLOB, "<<%d:\n", i);
                long_print_bin (VERB_GLOB, ln2);
            }

        // check that the lower bits are 0 (0 shifted in)
        // only check bits that are within range of [0..BITS_IN_LONGNUM-1] 
        for (j=0; j<i; j++)
            if ( (j<BITS_IN_LONGNUM) && (long_get_bit (ln2, j) != 0) )
            {
                new_errs++;
                eprintf (VERB_GLOB, "\nERR, SHL step 2 fails:\n");
                long_print_bin(VERB_GLOB, ln_orig);
                eprintf (VERB_GLOB, "<<%d:\n", i);
                long_print_bin (VERB_GLOB, ln2);
            }

        errs += new_errs;
//        new_errs = 0;                       
    }

    *errs += new_errs;
    return new_errs;
}

int test_long_get_word (int* errs)
{
    t_longnum ln_orig, ln2;
    t_word w;
    int i=0, err=0;

    // prepare the test data:
    random_fill_longnum (ln_orig);
    long_copy (ln2, ln_orig);        
 
    for (i=0; i<=BITS_IN_WORD; i++)
    {
        w = long_get_word (ln_orig, i);

        if (w != ln2[0])
        {
            err++;
            eprintf (VERB_GLOB, "\nERR, long_get_word fails:\n");
            long_print_bin(VERB_GLOB, ln_orig);
            eprintf (VERB_GLOB, "i=%d:\n", i);
            print_bin (VERB_GLOB, w, BITS_IN_WORD);
        }
        long_shiftright (ln2, 1);
    }

    *errs += err;
    return err;
}

int test_long_get_word_wraparound (int* errs)
{
    t_longnum ln_orig, ln2;
    t_word w;
    int i=0, temp, err=0;

    // prepare the test data:
    random_fill_longnum (ln_orig);
    long_copy (ln2, ln_orig);        
 
    for (i=0; i<=BITS_IN_LONGNUM; i++)
    {
        w = long_get_word_wraparound (ln_orig, BITS_IN_LONGNUM, i);

        if (w != ln2[0])
        {
            err++;
            eprintf (VERB_GLOB, "\n\nERR, long_get_word_wrap_around fails:\n");
            long_print_bin(VERB_GLOB, ln_orig);
            eprintf (VERB_GLOB, "i=%d; read word:", i);
            print_bin (VERB_GLOB, w, BITS_IN_WORD);
        }

        temp = long_get_bit (ln2, 0);
        long_shiftright (ln2, 1);
        long_setbit (ln2, BITS_IN_LONGNUM-1, temp);
    }

    *errs += err;
    return err;
}

int test_long_write_at_location (int* errs)
// tests the function long_write_at_location by writing random words of random length
// and reading them back. Returns the amount of errors that occurred.
{
    t_longnum ln1, ln2;
    t_word w;
    int i, j, compval, bitnum, err=0;

    random_fill_longnum (ln1);  // original test data

    for (i=0; i<=BITS_IN_LONGNUM; i++)
    // insert a random w of random length at each bit location 
    // and check that the new longnum is OK
    {
        // prepare the test data:
        long_copy (ln2, ln1);       // working long num 
        w = (rand()<<16)+rand();    // random word to write
        bitnum = rand()%BITS_IN_WORD;  // random # bits to write

        long_write_at_location (ln2, i, &w, bitnum);

        for (j=0; j<BITS_IN_LONGNUM; j++)
        // now check the values of each bit
        {
            if ( (j>=i) && (j<(i+bitnum)) )
            // in the area where w was written, get the (j-i)th bit of w
                compval = (w>>(j-i)) & 1;
            else    
            // outside of w, get the jth bit in the original longnum 
                compval = long_get_bit(ln1, j);

            // finally compare with the modified longnum:
            if (compval != long_get_bit(ln2, j)) 
            {
                err++;
                eprintf (VERB_GLOB, "\n\nERR, long_write_at_location fails:\n");
                eprintf (VERB_GLOB, "\nOriginal=\t"); long_print_bin (VERB_GLOB, ln1);
                eprintf (VERB_GLOB, "\ni=%d; w=\t", i); print_bin (VERB_GLOB, w, bitnum);
                eprintf (VERB_GLOB, "\nnew=\t"); long_print_bin (VERB_GLOB, ln2);
                eprintf (VERB_GLOB, "\nfailure at bit#%d.", j);
            }                
        }
    }

    *errs += err;
    return err;
}

int test_long_get_order (int* errs)
// tests the function long_get_order by trying all possibilities.
// Returns the amount of errors that occurred.
{
    t_longnum ln1;
    int i, err=0;

    random_fill_longnum (ln1);  // original test data
    long_setbit (ln1, BITS_IN_LONGNUM-1, 1);  // set the MSB

    for (i=BITS_IN_LONGNUM; i>=0; i--)
    {
        if (i != long_get_order (ln1))
        {
            err++;
            eprintf (VERB_GLOB, "\n\nERR, long_get_order fails:");
            eprintf (VERB_GLOB, "\nTestnum=\t"); long_print_bin (VERB_GLOB, ln1);
            eprintf (VERB_GLOB, "long_get_order=%d; i=%d", long_get_order(ln1), i);
        }
        long_shiftright (ln1, 1);
    }

    *errs += err;
    return err;
}

int test_division (int* errs)
{
    t_longnum denominator, numerator, quotient, remainder, temp;
    int err = 0;

    // prepare the denominator and numerator:
    random_fill_longnum(denominator);

    do
    // make sure the numerator != 0
    {
        random_fill_longnum(numerator);
        long_shiftright (numerator, rand()%100);  // make the numerator smaller than the denominator
    } 
    while (long_get_order(numerator) == 0);

    // perform the division:
    GF2_division (denominator, numerator, quotient, remainder);

    // and reproduce the denominator by reversing the division:
    GF2_multiply (numerator, quotient, temp);  // nom*quot
    long_xor (temp, remainder, temp);          // + rem

    if ( !long_cmp (denominator, temp) )
    {   
        eprintf (VERB_GLOB, "\nError in GF2-division.");
        eprintf (VERB_GLOB, "\nDenominator:\t\t"); long_print_bin(VERB_GLOB, denominator);
        eprintf (VERB_GLOB, "Numerator:\t\t"); long_print_bin(VERB_GLOB, numerator);

        eprintf (VERB_GLOB, "GF2 Quotient (D/N):\t");
        long_print_bin(VERB_GLOB, quotient);
        eprintf (VERB_GLOB, "GF2 Remainder (D mod R):\t");
        long_print_bin(VERB_GLOB, remainder);

        eprintf (VERB_GLOB, "GF2 Q*N+R?=D:\t\t"); long_print_bin(VERB_GLOB, temp);
    }

    err = !long_cmp(denominator, temp); 

    *errs += err;
    return err;
}

int test_divisions (int n, int* errs)
// tests the divisionfunction n times, returns the amount of errors
{
    int i, err=0;

    for (i = 0; i < n; i++)
        test_division(&err);

    *errs += err;
    return err;
}

int shape_test(t_telegram* telegram, int* errs)
// recreates the shaped contents in a telegram, compares the original to the newly created telegram
// uses the shaping bits and extra shaping bits from the original telegram
// returns the total amount of errors
{
    t_longnum UD_scrambled = { 0 }, Utick = { 0 }, UD = { 0 };
    t_telegram* new_tel = NULL;
    int err = 0;
    t_S S = 0;

    eprintf(VERB_GLOB, "\n\nchecking input line: "); eprintf(VERB_GLOB, telegram->input_string); eprintf(VERB_GLOB, "\n");

    // first align the telegram correctly:
    align_telegram(telegram, a_calc);  // shift the bits to the right to align with bit 0

    // create new telegram to work with, copy the CB+SB+ESB:
    create_new_telegram(&new_tel, "");
    init_telegram(new_tel, telegram->size);
    set_control_bits(new_tel->contents, get_control_bits(telegram->contents));
    set_scrambling_bits(new_tel->contents, get_scrambling_bits(telegram->contents));
    set_extra_shaping_bits(new_tel->contents, get_extra_shaping_bits(telegram->contents));


    // **** start with shaping the new telegram, based on the original contents from the input telegram:


    // first fill Utick by copying the unscrambled contents from the input telegram:
    long_copy(Utick, telegram->deshaped_contents);

    // then calculate the first word:
    determine_U_tick(Utick, telegram->number_of_userbits);

    // calculate S:
    S = determine_S(get_scrambling_bits(new_tel->contents));

    // scramble Utick into UD_scrambled:
    scramble_user_data(S, H, Utick, UD_scrambled, new_tel->number_of_userbits);

    // replace the 10-bit words by 11-bit words, fill telegram:
    transform10to11(UD_scrambled, new_tel);

    // and finally compute the checkbits (CRC):
    compute_check_bits(new_tel);


    // **** then deshape the telegram:


    // replace the 11-bit words by 10-bit words into UD:
    transform11to10(new_tel->deshaped_contents, new_tel);

    // calculate S:
    S = determine_S(get_scrambling_bits(new_tel->contents));

    // descramble:
    descramble(S, H, new_tel->deshaped_contents, new_tel->number_of_userbits);

    // recalculate the first word:
    calc_first_word(new_tel->deshaped_contents, new_tel->number_of_userbits);


    // **** finally, check the output of all the calculations above:


    // check the user data with the original user data after shaping and de-shaping them:
    eprintf(VERB_GLOB, "Check contents of the deshaped telegram:\t");
    if (!long_cmp(telegram->deshaped_contents, new_tel->deshaped_contents))
    {
        eprintf(VERB_GLOB, ERROR_COLOR"NOK\n"ANSI_COLOR_RESET);
        eprintf(VERB_GLOB, "NEW:\n");
        long_print_fancy(VERB_GLOB, new_tel->deshaped_contents, 8, telegram->number_of_userbits, NULL); 
        eprintf(VERB_GLOB, "ORIG:\n");
        long_print_fancy(VERB_GLOB, telegram->deshaped_contents, 8, telegram->number_of_userbits, NULL); 
        err++;
    }
    else
        eprintf(VERB_GLOB, OK_COLOR"OK\n"ANSI_COLOR_RESET);

    // also check the shaped contents of the new telegram against the original one:
    eprintf(VERB_GLOB, "Check new against original shaped contents:\t");
    if (!long_cmp(telegram->contents, new_tel->contents))
    {
        eprintf(VERB_GLOB, ERROR_COLOR"NOK\n"ANSI_COLOR_RESET);
        eprintf(VERB_GLOB, "ORIG:\n");
        print_telegram_contents_fancy(VERB_GLOB, telegram);
        long_print_bin(VERB_GLOB, telegram->contents);
        eprintf(VERB_GLOB, "NEW:\n");
        print_telegram_contents_fancy(VERB_GLOB, new_tel);
        long_print_bin(VERB_GLOB, new_tel->contents);
        err++;
    }
    else
    {
        eprintf(VERB_GLOB, OK_COLOR"OK\n"ANSI_COLOR_RESET);
        print_telegram_contents_fancy(VERB_GLOB, new_tel);
    }

    *errs += err;
    return err;
}

int run_shape_test(t_telegram* telegrams, int* error_count)
// if telegrams is set: iterate over the telegrams for which both shaped and unshaped contents have been given
// for each telegram, execute the shape test for each telegram
{
    t_telegram* p_telegram = telegrams;
    int i = 0, err = 0;

    while (p_telegram != NULL)
    {
        if ((long_get_order(p_telegram->contents) > 0) && (long_get_order(p_telegram->deshaped_contents) > 0))
            err += shape_test(p_telegram, error_count);
        else
            eprintf(VERB_GLOB, "Skipped telegram %d, as either shaped or unshaped data are not given.\n", i);

        p_telegram = p_telegram->next;
        i++;
    }
    eprintf(VERB_GLOB, "Ran shape test with %d telegrams.\n", i);
    return err;
}
/*
int shape_deshape_test(t_telegram* telegram, int* errs)
// shapes and de-shapes the telegram, checks if input == output
{
    t_longnum userdata;

    long_fill(userdata, 0);

    shape(telegram);
    deshape(telegram, userdata);

    if (!long_cmp(telegram->deshaped_contents, userdata))
    {
        eprintf(VERB_GLOB, "Random telegram test:\t\t");
        eprintf(VERB_GLOB, ERROR_COLOR"NOK\n"ANSI_COLOR_RESET);
        eprintf(VERB_GLOB, "INPUT:\n");
        long_print_fancy(VERB_GLOB, telegram->deshaped_contents, 8, BITS_IN_LONGNUM, NULL); 
        eprintf(VERB_GLOB, "RESHAPED:\n");
        long_print_fancy(VERB_GLOB, userdata, 8, BITS_IN_LONGNUM, NULL); 
        eprintf(VERB_GLOB, "TELEGRAM:\n");
        print_telegram_contents_fancy(VERB_GLOB, telegram);

        (*errs)++;
        return 1;
    }
    else
    {
        eprintf(VERB_GLOB, "Random telegram test:\t\t");
        eprintf(VERB_GLOB, OK_COLOR"OK\n"ANSI_COLOR_RESET);
    }

    return 0;
}
*/
int run_encoding_decoding_test(int count, int *errcount)
// tests the functions to read from and write to hex/base64 strings
// generates "count" random bit sequences of random telegramlength, encodes them and decodes them, checks against original values
{
    t_longnum original = { 0 }, recoded = { 0 };
    int i, j, length=0, err=0;   // length = #bytes
    char encoded_string[WORDS_IN_LONGNUM * 32] = { 0 };
    uint8_t binstring[MAX_ARRAY_SIZE] = { 0 };

    for (i = 0; i < count; i++)
    {
        long_fill(recoded, 0);
        encoded_string[0] = 0;
        length = 0;

        // fill the original with random values:
        for (j = 0; j < WORDS_IN_LONGNUM; j++)
            original[j] = (rand() << 16) | rand();

        // pick a random telegram length
        length = (rand() % 2) ? N_USERBITS_L : N_USERBITS_S;

        // fill the rest of the longnum with 0's
        for (j = length; j < BITS_IN_LONGNUM; j++)
            long_setbit(original, j, 0);

        // hex encode:
        long_sprint_hex(encoded_string, original, length);

        // hex decode:
        hex_to_bin(encoded_string, binstring);
        array_to_longnum(binstring, recoded, length/8+1);
//        hex_to_bin(encoded_string, recoded);

        if (!long_cmp(original, recoded))
        {
            eprintf(VERB_GLOB, ERROR_COLOR"NOK\n"ANSI_COLOR_RESET);
            eprintf(VERB_GLOB, "\nGenerated string #%d, length=%d bits:\n", i, length);
            long_print_bin(VERB_GLOB, original);
            eprintf(VERB_GLOB, "encoded hex: %s\n", encoded_string);
            printf("Intermediate binstring:");
            for (j = 0; j < length / 8 + 1; j++)
                print_bin(VERB_GLOB, binstring[j], 8);
            printf("\n");
            eprintf(VERB_GLOB, "Uncoded output:\n");
            long_print_bin(VERB_GLOB, recoded);
            err++;
        }

        // base64 encode:
        longnum_to_array(binstring, original, length/8+1);
        b64_encode(binstring, length/8+1, encoded_string);

        // base64 decode:
        b64_decode(encoded_string, length/8+1, binstring);
        array_to_longnum(binstring, recoded, length / 8 + 1);

        if (!long_cmp(original, recoded))
        {
            eprintf(VERB_GLOB, ERROR_COLOR"NOK\n"ANSI_COLOR_RESET);
            eprintf(VERB_GLOB, "\nGenerated string #%d, length=%d bits:\n", i, length);
            long_print_bin(VERB_GLOB, original);
            eprintf(VERB_GLOB, "encoded base64: %s\n", encoded_string);
            printf("Intermediate binstring:");
            for (j = 0; j < length / 8 + 1; j++)
                print_bin(VERB_GLOB, binstring[j], 8);
            printf("\n");
            eprintf(VERB_GLOB, "Uncoded output:\n");
            long_print_bin(VERB_GLOB, recoded);
            err++;
        }
    }

    *errcount += err;
    return err;
}

t_telegram* create_random_telegram(void)
// creates a random telegram: random length and random user data
{
    t_telegram* telegram;
    int j;
    t_longnum_layout telegram_marking[2] = { {0,0,0}, {0,0,0} };

    create_new_telegram(&telegram, "");

    //init_telegram(telegram, BITLENGTH_SHORT_TELEGRAM);
    init_telegram(telegram, (rand() % 2) ? BITLENGTH_LONG_TELEGRAM : BITLENGTH_SHORT_TELEGRAM);
    long_fill(telegram->deshaped_contents, 0);
    for (j = 0; j <= telegram->number_of_userbits / BITS_IN_WORD; j++)
        telegram->deshaped_contents[j] = (rand() << 16) | rand();
    j = 0;
    long_write_at_location(telegram->deshaped_contents, telegram->number_of_userbits, &j, 16);
    telegram->alignment = a_calc;

    eprintf(VERB_ALL, "\nCreated random telegram contents of %d bits:\n", telegram->number_of_userbits);
    telegram_marking[0] = (t_longnum_layout){ 0, telegram->number_of_userbits, ANSI_COLOR_GREEN };
    long_print_fancy(VERB_ALL, telegram->deshaped_contents, 8, BITS_IN_LONGNUM, telegram_marking);

    return telegram;
}

t_telegram* generate_random_telegrams(int count)
// generates a linked list of random telegrams (only unshaped user data, random length)
// returns a pointer to the first telegram
{
    int i = 0;
    t_telegram* telegram_list = NULL, * current_telegram = NULL, * temp = NULL;

    if (count > 0)
    {
        for (i = 0; i < count; i++)
        {
            temp = create_random_telegram();
            if (i == 0)
            // save the address of the first telegram:
                telegram_list = temp;
            else
            // point the previous telegram to this telegram:
                current_telegram->next = temp;

            current_telegram = temp;
        }

        // set next pointer of last telegram to NULL
        current_telegram->next = NULL;
    }

    eprintf(VERB_ALL, "Created linked list of %d random telegrams.\n", count);

    return telegram_list;
}

int run_shape_deshape_list_test(int count, int* errcount)
// runs a shape/deshape test using the multithreading-function from balise_codec.c:
{
    int i = 0, err = 0, shaped;
    t_telegram* p_telegrams = NULL, * curr_telegram = NULL;
    t_longnum short_data = { 0 };

    p_telegrams = generate_random_telegrams(count);
    curr_telegram = p_telegrams;

    printf("\nShaping random telegrams:\n");
    shaped = convert_telegrams_multithreaded(p_telegrams, 0);

    printf("Checking shaped telegrams:\n");
    shaped = convert_telegrams_multithreaded(p_telegrams, 0);

    return 0;
}

int run_make_long_test(int count, int* errcount)
// checks the make_long function for count times by generating random telegrams and checking the conversion of the short telegrams by SHR'ing them back again
// returns the amount of errors found
{
    int i=0, err = 0;
    t_telegram *p_telegrams = NULL, *curr_telegram = NULL;
    t_longnum short_data = { 0 };

    p_telegrams = generate_random_telegrams(count);
    curr_telegram = p_telegrams;

    while (curr_telegram)
    {
        i++;

        if (curr_telegram->number_of_userbits == N_USERBITS_S)
        {
            // store the old user data:
            long_copy(short_data, curr_telegram->deshaped_contents);

            // make it a long telegram:
            make_long(curr_telegram);

            // shift the contents right again:
            long_shiftright(curr_telegram->deshaped_contents, N_USERBITS_L - N_USERBITS_S);

            // check the result:
            if (!long_cmp(short_data, curr_telegram->deshaped_contents))
            {
                eprintf(VERB_GLOB, ERROR_COLOR"NOK\n"ANSI_COLOR_RESET);
                eprintf(VERB_GLOB, "\nShort telegram (#%d):\n", i);
                long_print_bin(VERB_GLOB, short_data);
                eprintf(VERB_GLOB, "After calculations:\n");
                long_print_bin(VERB_GLOB, curr_telegram->deshaped_contents);
                err++;
            }
        }
        curr_telegram = curr_telegram->next;
    }

    destroy_telegrams(p_telegrams);
    *errcount += err;
    return err;
}
/*
int run_random_telegram_test(int count, int* errs)
// generates random telegrams of random length (short or long) and calls the shape/deshape test
// todo: make this multithreaded
{
    int i, j, err = 0;
    t_telegram* telegram;

    create_new_telegram(&telegram, "");

    printf("\n");

    for (i = 0; i < count; i++)
    {
        printf("\rChecking telegram %d of %d.", i + 1, count);
        // create a random new telegram:
        init_telegram(telegram, (rand() % 2) ? BITLENGTH_LONG_TELEGRAM : BITLENGTH_SHORT_TELEGRAM);
        long_fill(telegram->deshaped_contents, 0);
        for (j = 0; j < telegram->number_of_userbits / BITS_IN_WORD; j++)
            telegram->deshaped_contents[j] = (rand() << 16) | rand();

        eprintf(VERB_GLOB, "\nCreated random telegram contents of %d bits.\n", telegram->number_of_userbits);
        err += shape_deshape_test(telegram, errs);
    }

    printf("\t\t\t");

    *errs += err;
    return err;
}
*/
/*
char* generate_random_input(int count)
// returns a hex string (as if read from input file) with random contents of length count
// each input string is on its own line, separated with \n
// string length is either 210 bits (26 bytes
{
    int i, j, size;
    char* input_string;
    char line[300];
    char newline[] = "\n";
    t_longnum temp = { 0 };

    input_string = malloc((size_t)count * 300);

    if (!input_string)
        return NULL;

    input_string[0] = 0;

    for (i = 0; i < count; i++)
    {
        size = (rand() % 2) ? N_USERBITS_L : N_USERBITS_S;
        long_fill(temp, 0);
        for (j = 0; j <= size / BITS_IN_WORD; j++)
            temp[j] = (rand() << 16) | rand();

        j = 0;
        long_write_at_location(temp, size, &j, 16);
        //temp[0] |= 0xFF;
        long_shiftleft(temp, 2);
        long_sprint_hex(line, temp, size);
//        eprintf(VERB_GLOB, "Created line of %d chars: %s\n", strlen(line), line);
        strcat(input_string, line);
        strcat(input_string, newline);
    }

    return input_string;
}

*/
/*
void create_off_synch_telegram(t_telegram* p_telegram, int n_cvw, int start)
// creates a telegram p_telegram to check the implementation of the off-synch-parsing condition-check.
// parameters: n_cvw = #cvw's, start=bit 0 of first vw
// commented out because this test does not work (too many false positives)
{
    int i;

    // clear the telegram:
    long_fill(p_telegram->contents, 0);

    start = start % p_telegram->size;

    for (i = 0; i < n_cvw; i++)
    {
        long_write_at_location(p_telegram->contents, start + i * 11, &transformation_words[0], 11);
    }
}

int run_off_synch_test(int telegram_size, int* errs)
// runs the off-synch-parsing condition-check test. Creates many telegrams that should or should not pass the off-synch-parsing condition.
// commented out because this test does not work (too many false positives)
{
    int err = 0, i=1, start=0, index, n_cvw, max_cvw=0;
    t_telegram* p_telegram;

    // initialise the telegram:
    create_new_telegram(&p_telegram, "");
    init_telegram(p_telegram, telegram_size);

    for (i = 0; i < 22; i++)
    // iterate over each bit in the telegram
    {
        // determine the max_ncw:
        if (i % 11 != 0)
        {
            if ((i % 11 == 1) || (i % 11 == 10))
                max_cvw = 2;
            else
                if (p_telegram->size == BITLENGTH_LONG_TELEGRAM)
                    max_cvw = 10;
                else
                    max_cvw = 6;
        }
        else
            max_cvw = 0;

        // iterate over the number of cvw's:
        for (n_cvw = 1; n_cvw <= 11; n_cvw++)
        {
            create_off_synch_telegram(p_telegram, n_cvw, i);

            index = check_off_synch_parsing_condition(p_telegram);

            if ((max_cvw !=0) && (index != MAGIC_WORD) && (n_cvw <= max_cvw))
                // if off-synch check gives false positive:
            {
                eprintf(0, "False positive in off synch telegram: i=%d, n_cvw=%d, location=%d\n", i, n_cvw, index);
                print_telegram_contents_fancy(0, p_telegram);
                err++;
            }
            else
                if ((max_cvw != 0) && (index == MAGIC_WORD) && (n_cvw > max_cvw))
                    // if off-synch check gives false negative:
                {
                    eprintf(0, "False negative in off synch telegram: i=%d, n_cvw=%d, location=%d:\n", i, n_cvw, index);
                    print_telegram_contents_fancy(0, p_telegram);
                    err++;
                }

        }
    }

    *errs += err;
    return err;
}
*/

void create_undersampled_telegram(t_telegram* p_telegram, int factor, int offset)
// creates an undersampled telegram p_telegram with undersampling factor "factor", starting at bitnum offset. 
{
    int i;
    int newbit, wordpointer=0, bitindex=0;

    long_fill(p_telegram->contents, 0);

    for (i = 0; i < BITLENGTH_SHORT_TELEGRAM; i++)
    {
        if (bitindex == 11)
        // skip to next transformation word:
        {
            wordpointer++;
            bitindex = 0;
        }

        // determine the new bit and write it to the telegram:
        newbit = (transformation_words[wordpointer] >> bitindex) & 1;
        long_setbit(p_telegram->contents, (i * factor + offset) % p_telegram->size, newbit);
        bitindex++;
    }
}

int run_undersampling_test(int telegram_size, int* errs)
// runs the undersampling test. Creates many undersampled telegrams with all possible k's and offsets, check the undersampling check.
{
    int err = 0, j, k, temp;
    t_telegram* p_telegram;

    // initialise the telegram:
    create_new_telegram(&p_telegram, "");
    init_telegram(p_telegram, telegram_size);

    for (k = 2; k < 16; k *= 2)
    // iterate over the factors 2,4,8,16
    {
        // create an undersampled telegram with factor k at offset 0
        create_undersampled_telegram(p_telegram, k, 0);

        for (j = 0; j < p_telegram->size; j++)
        // iterate over the starting bits
        {
            // calculate the next telegram by rotating it 1 to the left:
            temp = long_get_bit(p_telegram->contents, p_telegram->size-1);
            long_shiftleft(p_telegram->contents, 1);
            long_setbit(p_telegram->contents, 0, temp);

            if (check_undersampling_condition(p_telegram) != ERR_UNDER_SAMPLING)
                // if undersampling check gives false negative:
            {
                printf("NOK");
                eprintf(0, "\nCreated undersampled telegram (j=%d, k=%d):\n", j, k);
                print_telegram_contents_fancy(0, p_telegram);
                err++;
            }
        }
    }

    *errs += err;
    return err;
}

int run_sanity_tests(t_telegram* telegrams)
// runs various high and low level tests
// returns 0 if no error found, !=0 if error found
{
    int error_count = 0;

    // randomise:
    srand((unsigned)time(NULL));

    // Start with low-level longnum-tests:
 
    printf ("Running GF2-functions test with %d random words of %d bytes at verbosity level %d.\n", WORDS_IN_LONGNUM, (int)sizeof (t_word), verbose);

    printf ("Testing copy & compare:\t\t\t\t");
    print_result(test_copy_cmp(&error_count));

    printf ("Testing shift right:\t\t\t\t");
    print_result(test_shift_right(&error_count));

    printf ("Testing shift left:\t\t\t\t");
    print_result(test_shift_left(&error_count));

    printf ("Testing xor:\t\t\t\t\t");
    print_result(test_xor(&error_count));

    printf ("Testing long_get_word:\t\t\t\t");
    print_result(test_long_get_word(&error_count));

    printf ("Testing long_get_word_wraparound:\t\t");
    print_result(test_long_get_word_wraparound(&error_count));

    printf ("Testing long_write_at_location:\t\t\t");
    print_result(test_long_write_at_location(&error_count));

    printf ("Testing long_get_order:\t\t\t\t");
    print_result(test_long_get_order(&error_count));

    // test longnum division and multiplication:
    printf ("Testing 50*GF2-divison & multiplication:\t");
    print_result(test_divisions (50, &error_count));

    // test encoding schemes:
    printf("Testing HEX and BASE64 encoding/decoding:\t");
    print_result(run_encoding_decoding_test(100, &error_count));

    // test telegram creation functions:
    printf("Testing make_long:\t\t\t\t");
    print_result(run_make_long_test(50, &error_count));

//    printf("Running off-synch-parsing condition check with long telegrams:\t\t");
//    print_result(run_off_synch_test(BITLENGTH_LONG_TELEGRAM, &error_count));

    printf("Testing undersampling check, long telegrams:\t");
    print_result(run_undersampling_test(BITLENGTH_LONG_TELEGRAM, &error_count));

    printf("Testing undersampling check, short telegrams:\t");
    print_result(run_undersampling_test(BITLENGTH_SHORT_TELEGRAM, &error_count));
        
    if (telegrams != NULL)
    {
        printf("Running shape test with %d input telegrams:\t", count_telegrams (telegrams));
        print_result(run_shape_test(telegrams, &error_count));
    }

    printf("Running random multithreaded shape/deshape test:\t");
    print_result(run_shape_deshape_list_test(100, &error_count));

    return error_count;
}    