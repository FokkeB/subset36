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

#include <stdio.h>        // printf
#include <stdlib.h>       // random numbers
#include <time.h>         // timer for randomisation
#include "useful_functions.h"
#include "longnum.h"
#include "telegram.h"
#include "parse_input.h"
#include "balise_codec.h"     // included to test functions in this file. 

int verbose = VERB_PROG;

void print_result(int result)
// prints out the result (OK=0; NOK!=0)
{
    if (result)
        printf(ERROR_COLOR "NOK (#errs=%d)\n" ANSI_COLOR_RESET, result);
    else
        printf(OK_COLOR "OK\n" ANSI_COLOR_RESET);
}

int test_copy_cmp(int* err_total)
// tests the =, == and != operators, returns error
{
    longnum ln1(FILL_RANDOM), ln2;
    int err = 0;

    ln2 = ln1;

    if (ln1 != ln2)
        err++;

    if (!(ln1 == ln2))
        err++;

    ln1[1] = 1;
    if (ln1 == ln2)
        err++;

    if (!(ln1 != ln2))
        err++;

    *err_total += err;
    return (err);
}

int test_xor(int* errs)
// tests the ^ operator (this also tests ^=), returns #errors
{
    longnum ln1(FILL_RANDOM), ln2(FILL_RANDOM), ln3;
    int i, err = 0;

    ln3 = ln1 ^ ln2;

    // iterate over all the bits, verify the XOR-operation
    for (i = 0; i < BITS_IN_LONGNUM; i++)
        if ((ln1.get_bit(i) ^ ln2.get_bit(i)) != ln3.get_bit(i))
        {
            err++;
            eprintf(VERB_GLOB, "\nERR, XOR fails @bit %d:\n", i);
            eprintf(VERB_GLOB, "ln1 =\t\t"); ln1.print_bin(VERB_GLOB);
            eprintf(VERB_GLOB, "ln2 =\t\t"); ln2.print_bin(VERB_GLOB);
            eprintf(VERB_GLOB, "ln1^ln2 =\t"); ln3.print_bin(VERB_GLOB);
        }

    *errs += err;
    return err;
}

int test_shift_right(int* errs)
// tests the >> operator (this also tests the >>=), returns #errors
{
    longnum ln_orig (FILL_RANDOM), ln2;
    int i, j, new_errs = 0;

    // SHR over the complete longnum in increasing steps of 1
    for (i = 1; i < BITS_IN_LONGNUM; i *= 2)
    {
        ln2 = ln_orig;
        ln2 = ln2 >> i;

        // check the shifted bits:
        for (j = 0; j < BITS_IN_LONGNUM - i; j++)
            if (ln_orig.get_bit(i + j) != ln2.get_bit(j))
            {
                new_errs++;
                eprintf(VERB_GLOB, "\nERR, SHR step 1 fails:\n");
                ln_orig.print_bin(VERB_GLOB);
                ln2.print_bin(VERB_GLOB);
            }

        // check that the higher bits are 0 (0 shifted in)
        // only check bits that are within range of [0..BITS_IN_LONGNUM-1] 
        for (j = BITS_IN_LONGNUM - i; j < BITS_IN_LONGNUM; j++)
            if ((j >= 0) && (ln2.get_bit(j) != 0))
            {
                new_errs++;
                eprintf(VERB_GLOB, "\nERR, SHR step 2 fails:\n");
                ln_orig.print_bin(VERB_GLOB);
                eprintf(VERB_GLOB, ">>%d:\n", i);
                ln2.print_bin(VERB_GLOB);
            }

        errs += new_errs;                   
    }

    *errs += new_errs;
    return new_errs;
}

int test_shift_left(int* errs)
// tests the << operator (this also tests <<=), returns #errors
{
    longnum ln_orig (FILL_RANDOM), ln2;
    int i, j, new_errs = 0;

    // SHL over the complete longnum in increasing steps, including i>N:
    for (i = 1; i <= BITS_IN_LONGNUM * 2; i *= 2)
    {
        ln2 = ln_orig;
        ln2 = ln2 << i;

        // check the shifted bits:
        for (j = i; j < BITS_IN_LONGNUM; j++)
            if (ln_orig.get_bit(j - i) != ln2.get_bit(j))
            {
                new_errs++;
                eprintf(VERB_GLOB, "\nERR, SHL step 1 fails:\n");
                ln_orig.print_bin(VERB_GLOB);
                eprintf(VERB_GLOB, "<<%d:\n", i);
                ln2.print_bin(VERB_GLOB);
            }

        // check that the lower bits are 0 (0 shifted in)
        // only check bits that are within range of [0..BITS_IN_LONGNUM-1] 
        for (j = 0; j < i; j++)
            if ((j < BITS_IN_LONGNUM) && (ln2.get_bit(j) != 0))
            {
                new_errs++;
                eprintf(VERB_GLOB, "\nERR, SHL step 2 fails:\n");
                ln_orig.print_bin(VERB_GLOB);
                eprintf(VERB_GLOB, "<<%d:\n", i);
                ln2.print_bin(VERB_GLOB);
            }

        errs += new_errs;
    }

    *errs += new_errs;
    return new_errs;
}

int test_long_get_word(int* errs)
// tests the get_word-function
{
    longnum ln_orig (FILL_RANDOM), ln2;
    t_word w;
    int i = 0, err = 0;

    // prepare the test data:
    ln2 = ln_orig;

    for (i = 0; i <= BITS_IN_WORD; i++)
    {
        w = ln_orig.get_word(i);

        if (w != ln2[0])
        {
            err++;
            eprintf(VERB_GLOB, "\nERR, long_get_word fails:\n");
            ln_orig.print_bin(VERB_GLOB);
            eprintf(VERB_GLOB, "i=%d:\n", i);
            print_bin(VERB_GLOB, w, BITS_IN_WORD);
        }

        ln2 >>= 1;
    }

    *errs += err;
    return err;
}

int test_long_get_word_wraparound(int* errs)
{
    longnum ln_orig (FILL_RANDOM), ln2;
    t_word w;
    int i = 0, temp, err = 0;

    // prepare the test data:
    ln2 = ln_orig;

    for (i = 0; i <= BITS_IN_LONGNUM; i++)
    {
        w = ln_orig.get_word_wraparound(BITS_IN_LONGNUM, i);

        if (w != ln2[0])
        {
            err++;
            eprintf(VERB_GLOB, "\n\nERR, long_get_word_wrap_around fails:\n");
            ln_orig.print_bin(VERB_GLOB);
            eprintf(VERB_GLOB, "i=%d; read word:", i);
            print_bin(VERB_GLOB, w, BITS_IN_WORD);
        }

        temp = ln2.get_bit(0);
        ln2 >>= 1;
        ln2.set_bit(BITS_IN_LONGNUM - 1, temp);
    }

    *errs += err;
    return err;
}

int test_long_write_at_location(int* errs)
// tests the function long_write_at_location by writing random words of random length
// and reading them back. Returns the amount of errors that occurred.
{
    longnum ln1 (FILL_RANDOM), ln2;
    t_word w;
    int i, j, compval, bitnum, err = 0;

    for (i = 0; i <= BITS_IN_LONGNUM; i++)
    // insert a random w of random length at each bit location 
    // and check that the new longnum is OK
    {
        // prepare the test data:
        ln2 = ln1;       // working long num 
        w = (rand() << 16) + rand();    // random word to write
        bitnum = rand() % BITS_IN_WORD;  // random # bits to write

        ln2.write_at_location(i, &w, bitnum);

        for (j = 0; j < BITS_IN_LONGNUM; j++)
            // now check the values of each bit
        {
            if ((j >= i) && (j < (i + bitnum)))
                // in the area where w was written, get the (j-i)th bit of w
                compval = (w >> (j - i)) & 1;
            else
                // outside of w, get the jth bit in the original longnum 
                compval = ln1.get_bit(j);

            // finally compare with the modified longnum:
            if (compval != ln2.get_bit(j))
            {
                err++;
                eprintf(VERB_GLOB, "\n\nERR, long_write_at_location fails:\n");
                eprintf(VERB_GLOB, "\nOriginal=\t"); ln1.print_bin(VERB_GLOB);
                eprintf(VERB_GLOB, "\ni=%d; w=\t", i); //print_bin(VERB_GLOB, w, bitnum);
                eprintf(VERB_GLOB, "\nnew=\t"); ln2.print_bin(VERB_GLOB);
                eprintf(VERB_GLOB, "\nfailure at bit#%d.", j);
            }
        }
    }

    *errs += err;
    return err;
}

int test_long_get_order(int* errs)
// tests the function long_get_order by trying all possibilities.
// Returns the amount of errors that occurred.
{
    longnum ln1 (FILL_RANDOM);
    int i, err = 0;

    ln1.set_bit(BITS_IN_LONGNUM - 1, 1);  // set the MSB

    for (i = BITS_IN_LONGNUM; i >= 0; i--)
    {
        if (i != ln1.get_order())
        {
            err++;
            eprintf(VERB_GLOB, "\n\nERR, long_get_order fails:");
            eprintf(VERB_GLOB, "\nTestnum=\t"); ln1.print_bin(VERB_GLOB);
            eprintf(VERB_GLOB, "long_get_order=%d; i=%d", ln1.get_order(), i);
        }
        ln1 >>= 1;
    }

    *errs += err;
    return err;
}

int test_division(int* errs)
{
    longnum numerator (FILL_RANDOM), denominator, quotient, remainder;   // teller, noemer, quotient, rest. teller/noemer = quotient; teller%noemer=rest; teller=quotient*noemer+rest
    longnum temp;

    do
    // make sure the denominator != 0
    {
        denominator.fill (FILL_RANDOM);
        denominator = denominator >> (rand() % 100);  // make the denominator smaller than the numerator
    } while (denominator.get_order() == 0);

    // perform the division, return the quotient and remainder:
    numerator.GF2_division(denominator, quotient, remainder);

    // and reproduce the numerator by reversing the division:
    temp = denominator * quotient + remainder;

    if (numerator != temp)
    {
        eprintf(VERB_GLOB, "\nError in GF2-division.");
        eprintf(VERB_GLOB, "Numerator:\t\t"); numerator.print_bin(VERB_GLOB);
        eprintf(VERB_GLOB, "\nDenominator:\t\t"); denominator.print_bin(VERB_GLOB);

        eprintf(VERB_GLOB, "GF2 Quotient (N/D):\t");
        quotient.print_bin(VERB_GLOB);
        eprintf(VERB_GLOB, "GF2 Remainder (D mod R):\t");
        remainder.print_bin(VERB_GLOB);

        eprintf(VERB_GLOB, "GF2 Q*D+R?=D:\t\t"); temp.print_bin(VERB_GLOB);
        return 1;
    }

    return 0;
}

int test_divisions(int n, int* errs)
// tests the divisionfunction n times, returns the amount of errors
{
    int i, err = 0;

    for (i = 0; i < n; i++)
        test_division(&err);

    *errs += err;
    return err;
}

int run_encoding_decoding_test(int count, int* errcount)
// tests the functions to read from and write to hex/base64 strings
// generates "count" random bit sequences of random telegramlength, encodes them and decodes them, checks against original values
{
    longnum original, recoded;
    int i, j, length = 0, err = 0;   // length = #bytes
    //char encoded_string[WORDS_IN_LONGNUM * 32] = { 0 };
    string encoded_string;// , binstring;
    uint8_t binstring[MAX_ARRAY_SIZE] = { 0 };

    for (i = 0; i < count; i++)
    {
        encoded_string = ""; // [0] = 0;
        length = 0;

        original.fill(FILL_RANDOM);
        recoded.fill(0);

        // pick a random telegram length
        length = (rand() % 2) ? N_USERBITS_L : N_USERBITS_S;

        // fill the rest of the longnum with 0's
        for (j = length; j < BITS_IN_LONGNUM; j++)
            original.set_bit(j, 0);

        // hex encode:
        original.sprint_hex(encoded_string, length);

        // hex decode:
        hex_to_bin(encoded_string, binstring);
        recoded.read_from_array(binstring, length / 8 + 1);

        if (original != recoded)
        {
            eprintf(VERB_GLOB, ERROR_COLOR "NOK\n" ANSI_COLOR_RESET);
            eprintf(VERB_GLOB, "\nGenerated string #%d, length=%d bits:\n", i, length);
            original.print_bin(VERB_GLOB);
            eprintf(VERB_GLOB, "encoded hex: %s\n", encoded_string.c_str());
            printf("Intermediate binstring:");
            for (j = 0; j < length / 8 + 1; j++)
                print_bin(VERB_GLOB, binstring[j], 8);
            printf("\n");
            eprintf(VERB_GLOB, "Uncoded output:\n");
            recoded.print_bin(VERB_GLOB);
            err++;
        }

        // base64 encode:
        original.write_to_array(binstring, length / 8 + 1);
        b64_encode(binstring, length / 8 + 1, encoded_string);

        // base64 decode:
        b64_decode(encoded_string, binstring);
        recoded.read_from_array(binstring, length / 8 + 1);

        if (original != recoded)
        {
            eprintf(VERB_GLOB, ERROR_COLOR "NOK\n" ANSI_COLOR_RESET);
            eprintf(VERB_GLOB, "\nGenerated string #%d, length=%d bits:\n", i, length);
            original.print_bin(VERB_GLOB);
            eprintf(VERB_GLOB, "encoded base64: %s\n", encoded_string.c_str());
            printf("Intermediate binstring:");
            for (j = 0; j < length / 8 + 1; j++)
                print_bin(VERB_GLOB, binstring[j], 8);
            printf("\n");
            eprintf(VERB_GLOB, "Uncoded output:\n");
            recoded.print_bin(VERB_GLOB);
            err++;
        }
    }

    *errcount += err;
    return err;
}

telegram* create_random_telegram(void)
// creates a random telegram: random length and random user data
{
    t_word j;
    t_longnum_layout telegram_marking[2] = { {0,0,0}, {0,0,0} };

    // create new telegram with random contents and of random length
    telegram* tel = new telegram ("", (rand() % 2) ? s_short : s_long);
 
    // fill the part of the telegram that is unused with 0's:
    for (j = 0; j <= tel->number_of_userbits / BITS_IN_WORD; j++)
        tel->deshaped_contents[j] = (rand() << 16) | rand();
    j = 0;
    tel->deshaped_contents.write_at_location(tel->number_of_userbits, &j, 16);
    tel->alignment = a_calc;

    eprintf(VERB_ALL, "\nCreated random telegram contents of %d bits:\n", tel->number_of_userbits);
    telegram_marking[0] = { 0, tel->number_of_userbits, ANSI_COLOR_GREEN };
    tel->deshaped_contents.print_fancy(VERB_ALL, 8, BITS_IN_LONGNUM, telegram_marking);

    return tel;
}

telegram* generate_random_telegrams(int count)
// generates a linked list of random telegrams (only unshaped user data, random length)
// returns a pointer to the first telegram
{
    int i = 0;
    telegram* tel_list = NULL;
    telegram* current_tel = NULL;

    for (i = 0; i < count; i++)
    {
        if (i == 0)
        {
            tel_list = create_random_telegram();
            current_tel = tel_list;
        }
        else
        {
            current_tel->next = create_random_telegram();
            current_tel = current_tel->next;
        }
    }

    eprintf(VERB_ALL, "Created list of %d random telegrams.\n", count);

    return tel_list;
}

int run_shape_deshape_list_test(int count, int* errcount)
// runs a shape/deshape test using the multithreading-function from balise_codec.cpp:
{
    int shaped;
    telegram* tel_list;

    tel_list = generate_random_telegrams(count);

    printf("\nShaping random telegrams:\n");
    shaped = convert_telegrams_multithreaded(tel_list, 0, false);

    printf("Checking shaped telegrams:\n");
    shaped = convert_telegrams_multithreaded(tel_list, 0, false);

    return 0;
}

int run_make_long_test(int count, int* errcount)
// checks the make_long function for count times by generating random telegrams and checking the conversion of the short telegrams by SHR'ing them back again
// returns the amount of errors found
{
    int i = 0, err = 0;
    telegram* tel_list;
    longnum short_data;

    tel_list = generate_random_telegrams(count);

    // iterate over the telegrams in the list:
    while (tel_list)
    {
        i++;

        if (tel_list->number_of_userbits == N_USERBITS_S)
        // only look at short telegrams
        {
            // store the old user data:
            short_data = tel_list->deshaped_contents;

            // make it a long telegram:
            tel_list->make_userdata_long();

            // shift the contents right again:
            tel_list->deshaped_contents >>= (N_USERBITS_L - N_USERBITS_S);

            // check the result:
            if (short_data != tel_list->deshaped_contents)
            {
                eprintf(VERB_GLOB, ERROR_COLOR "NOK\n" ANSI_COLOR_RESET);
                eprintf(VERB_GLOB, "\nShort telegram (#%d):\n", i);
                short_data.print_bin(VERB_GLOB);
                eprintf(VERB_GLOB, "After calculations:\n");
                tel_list->deshaped_contents.print_bin(VERB_GLOB);
                err++;
            }
        }

        tel_list = tel_list->next;
    }

    *errcount += err;
    return err;
}

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

void create_undersampled_telegram(t_telegram* p_telegram, int factor, int offset)
// creates an undersampled telegram p_telegram with undersampling factor "factor", starting at bitnum offset. 
{
    int i;
    int newbit, wordpointer = 0, bitindex = 0;

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
            temp = long_get_bit(p_telegram->contents, p_telegram->size - 1);
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
*/


int main(void)
// runs various high and low level tests
// returns 0 if no error found, !=0 if error found
{
    int error_count = 0;

    // randomise:
    srand((unsigned)time(NULL));

    // Start with low-level longnum-tests:

    printf("Running GF2-functions test with %d random words of %d bytes at verbosity level %d.\n", WORDS_IN_LONGNUM, (int)sizeof(t_word), verbose);

    printf("Testing copy & compare:\t\t\t\t");
    print_result(test_copy_cmp(&error_count));

    printf("Testing shift right:\t\t\t\t");
    print_result(test_shift_right(&error_count));

    printf("Testing shift left:\t\t\t\t");
    print_result(test_shift_left(&error_count));

    printf("Testing xor:\t\t\t\t\t");
    print_result(test_xor(&error_count));

    printf("Testing long_get_word:\t\t\t\t");
    print_result(test_long_get_word(&error_count));

    printf("Testing long_get_word_wraparound:\t\t");
    print_result(test_long_get_word_wraparound(&error_count));

    printf("Testing long_write_at_location:\t\t\t");
    print_result(test_long_write_at_location(&error_count));

    printf("Testing long_get_order:\t\t\t\t");
    print_result(test_long_get_order(&error_count));

    // test longnum division and multiplication:
    printf("Testing 50*GF2-divison & multiplication:\t");
    print_result(test_divisions(50, &error_count));

    // test encoding schemes:
    printf("Testing HEX and BASE64 encoding/decoding:\t");
    print_result(run_encoding_decoding_test(100, &error_count));
    
    // test telegram creation functions:
    printf("Testing make_long:\t\t\t\t");
    print_result(run_make_long_test(50, &error_count));
/*
    //    printf("Running off-synch-parsing condition check with long telegrams:\t\t");
    //    print_result(run_off_synch_test(BITLENGTH_LONG_TELEGRAM, &error_count));

    printf("Testing undersampling check, long telegrams:\t");
    print_result(run_undersampling_test(BITLENGTH_LONG_TELEGRAM, &error_count));

    printf("Testing undersampling check, short telegrams:\t");
    print_result(run_undersampling_test(BITLENGTH_SHORT_TELEGRAM, &error_count));

    if (telegrams != NULL)
    {
        printf("Running shape test with %d input telegrams:\t", count_telegrams(telegrams));
        print_result(run_shape_test(telegrams, &error_count));
    }
*/
    printf("Running multithreaded shape/deshape test with 100 random telegrams:\t");
    print_result(run_shape_deshape_list_test(100, &error_count));

    return error_count;
}
