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

int verbose = VERB_QUIET;
 
string zp_test_telegram = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC";
struct t_result {
    int sb;
    int esb;
};
// Zho Pengs 474 results in a struct, end with added (0,0) to signal the end of the list
t_result zp_results[] = { {18, 709}, {18, 1015}, {49, 490}, {49, 650}, {53, 981}, {94, 480}, {94, 922}, {96, 719}, {101, 15}, {101, 217}, {103, 91}, {103, 386}, {108, 67}, {108, 164}, {108, 171}, {108, 855}, {108, 987}, {116, 656}, {116, 798}, {132, 162}, {132, 295}, {133, 365}, {133, 750}, {136, 186}, {136, 904}, {148, 815}, {154, 130}, {154, 277}, {169, 219}, {177, 113}, {177, 498}, {177, 894}, {180, 525}, {180, 956}, {182, 858}, {183, 113}, {183, 805}, {187, 501}, {187, 633}, {235, 725}, {239, 99}, {250, 666}, {250, 876}, {250, 878}, {255, 134}, {255, 403}, {267, 76}, {267, 229}, {267, 712}, {270, 714}, {270, 897}, {358, 165}, {358, 481}, {361, 543}, {363, 743}, {363, 989}, {375, 341}, {375, 877}, {377, 178}, {418, 375}, {433, 219}, {433, 638}, {445, 555}, {571, 370}, {571, 406}, {599, 44}, {599, 297}, {599, 977}, {606, 547}, {606, 905}, {738, 286}, {738, 918}, {743, 158}, {754, 495}, {754, 927}, {762, 824}, {765, 213}, {765, 220}, {784, 663}, {784, 854}, {836, 14}, {836, 235}, {836, 307}, {836, 716}, {905, 919}, {908, 77}, {908, 124}, {908, 555}, {911, 43}, {911, 499}, {920, 702}, {920, 826}, {934, 369}, {934, 866}, {942, 551}, {972, 120}, {975, 137}, {996, 1014}, {1002, 176}, {1002, 470}, {1004, 179}, {1004, 212}, {1004, 394}, {1004, 590}, {1004, 868}, {1051, 215}, {1051, 622}, {1051, 668}, {1051, 831}, {1053, 32}, {1053, 126}, {1053, 185}, {1053, 318}, {1053, 654}, {1053, 745}, {1053, 855}, {1053, 894}, {1063, 885}, {1079, 973}, {1083, 805}, {1091, 107}, {1099, 215}, {1099, 785}, {1101, 286}, {1101, 969}, {1112, 134}, {1112, 533}, {1112, 849}, {1116, 88}, {1116, 92}, {1116, 183}, {1142, 955}, {1277, 52}, {1277, 985}, {1283, 910}, {1287, 104}, {1287, 206}, {1287, 349}, {1292, 984}, {1295, 116}, {1295, 163}, {1295, 430}, {1302, 390}, {1313, 120}, {1313, 344}, {1329, 850}, {1338, 654}, {1341, 54}, {1341, 930}, {1357, 344}, {1357, 966}, {1362, 457}, {1415, 995}, {1426, 130}, {1431, 93}, {1442, 125}, {1444, 386}, {1444, 398}, {1448, 703}, {1448, 739}, {1465, 164}, {1480, 49}, {1480, 898}, {1484, 577}, {1484, 588}, {1484, 868}, {1500, 147}, {1500, 313}, {1501, 396}, {1608, 93}, {1629, 114}, {1629, 310}, {1629, 464}, {1629, 538}, {1634, 68}, {1634, 178}, {1636, 156}, {1636, 234}, {1636, 294}, {1636, 1011}, {1646, 172}, {1654, 556}, {1654, 627}, {1654, 643}, {1670, 864}, {1672, 159}, {1673, 720}, {1673, 803}, {1677, 384}, {1677, 473}, {1677, 897}, {1691, 392}, {1696, 618}, {1696, 961}, {1704, 721}, {1705, 290}, {1705, 348}, {1741, 210}, {1746, 904}, {1754, 59}, {1754, 868}, {1760, 735}, {1760, 820}, {1760, 978}, {1762, 456}, {1787, 293}, {1787, 867}, {1826, 131}, {1826, 667}, {1826, 841}, {1826, 878}, {1826, 898}, {1832, 24}, {1832, 33}, {1832, 104}, {1832, 203}, {1855, 303}, {1974, 907}, {1976, 802}, {1976, 916}, {1976, 925}, {2019, 858}, {2019, 987}, {2068, 577}, {2068, 845}, {2087, 105}, {2087, 111}, {2087, 464}, {2088, 691}, {2093, 200}, {2093, 964}, {2116, 175}, {2116, 291}, {2116, 579}, {2123, 320}, {2123, 723}, {2123, 1014}, {2124, 383}, {2140, 306}, {2140, 393}, {2140, 542}, {2140, 579}, {2191, 390}, {2191, 494}, {2198, 230}, {2198, 405}, {2204, 121}, {2204, 897}, {2204, 911}, {2209, 481}, {2211, 865}, {2238, 239}, {2238, 731}, {2238, 970}, {2258, 715}, {2296, 934}, {2310, 129}, {2310, 677}, {2337, 200}, {2337, 788}, {2337, 823}, {2345, 542}, {2356, 497}, {2356, 705}, {2367, 374}, {2370, 984}, {2379, 897}, {2399, 400}, {2418, 678}, {2445, 159}, {2445, 842}, {2508, 154}, {2508, 942}, {2508, 980}, {2518, 206}, {2518, 676}, {2637, 152}, {2637, 458}, {2647, 68}, {2647, 218}, {2647, 373}, {2647, 1001}, {2656, 684}, {2656, 709}, {2656, 715}, {2675, 197}, {2675, 982}, {2699, 709}, {2702, 15}, {2702, 166}, {2703, 18}, {2708, 580}, {2708, 681}, {2708, 831}, {2760, 921}, {2774, 173}, {2774, 625}, {2779, 358}, {2779, 359}, {2792, 65}, {2795, 383}, {2795, 985}, {2806, 865}, {2807, 84}, {2807, 139}, {2807, 216}, {2807, 351}, {2811, 231}, {2811, 318}, {2815, 47}, {2815, 73}, {2815, 89}, {2815, 320}, {2823, 343}, {2823, 853}, {2846, 129}, {2846, 219}, {2846, 406}, {2854, 96}, {2859, 159}, {2870, 126}, {2871, 353}, {2871, 905}, {2878, 712}, {2917, 212}, {2917, 490}, {2930, 470}, {2930, 497}, {2931, 902}, {2976, 549}, {2976, 810}, {2977, 468}, {2977, 521}, {2978, 45}, {2978, 522}, {3018, 179}, {3018, 871}, {3028, 873}, {3101, 171}, {3101, 294}, {3101, 812}, {3235, 350}, {3240, 164}, {3240, 722}, {3256, 116}, {3256, 154}, {3257, 851}, {3296, 604}, {3296, 643}, {3296, 777}, {3319, 155}, {3319, 386}, {3319, 494}, {3319, 829}, {3319, 951}, {3330, 494}, {3339, 70}, {3339, 989}, {3353, 854}, {3362, 287}, {3362, 426}, {3362, 838}, {3369, 617}, {3369, 651}, {3384, 180}, {3384, 886}, {3384, 947}, {3384, 970}, {3386, 673}, {3467, 618}, {3467, 726}, {3493, 487}, {3493, 809}, {3495, 239}, {3495, 839}, {3495, 879}, {3495, 898}, {3501, 133}, {3503, 93}, {3503, 229}, {3510, 201}, {3515, 498}, {3521, 122}, {3523, 294}, {3523, 471}, {3523, 804}, {3523, 909}, {3525, 125}, {3525, 972}, {3620, 885}, {3625, 819}, {3632, 703}, {3637, 178}, {3651, 352}, {3651, 357}, {3651, 636}, {3660, 388}, {3660, 792}, {3663, 153}, {3674, 147}, {3674, 741}, {3674, 846}, {3676, 237}, {3704, 146}, {3705, 848}, {3713, 345}, {3732, 115}, {3732, 232}, {3732, 288}, {3736, 25}, {3736, 55}, {3736, 678}, {3741, 96}, {3741, 984}, {3747, 602}, {3747, 853}, {3751, 323}, {3756, 86}, {3756, 238}, {3756, 720}, {3756, 886}, {3779, 958}, {3784, 109}, {3784, 654}, {3784, 819}, {3789, 170}, {3797, 469}, {3797, 559}, {3797, 862}, {3797, 995}, {3809, 162}, {3809, 852}, {3810, 1013}, {3815, 91}, {3824, 819}, {3824, 861}, {3826, 533}, {3827, 870}, {3828, 289}, {3829, 842}, {3837, 308}, {3839, 358}, {3845, 388}, {3864, 815}, {3865, 906}, {3907, 164}, {3907, 402}, {3907, 931}, {3914, 168}, {3914, 493}, {3914, 833}, {3920, 616}, {3920, 822}, {3920, 977}, {3922, 68}, {3922, 745}, {3928, 860}, {3944, 54}, {3944, 201}, {3944, 648}, {3944, 831}, {3948, 540}, {3948, 623}, {3961, 526}, {3965, 566}, {3965, 1014}, {4001, 10}, {4001, 133}, {4009, 344}, {4009, 477}, {4009, 926}, {4011, 302}, {4011, 446}, {4069, 535}
, {0,0}
};


t_longnum_layout no_markings = { 0, 0, ""};   // use if no colors are to be shown


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

int test_rotate_and_inversion(int* errs)
// tests the rotate function by rotating over all allowed values and back again
{
    longnum ln_rnd(FILL_RANDOM), ln_55, ln1, ln2;
    int i, err=0, size = BITS_IN_LONGNUM;

    // prepare the test data:
    for (i = 0; i < WORDS_IN_LONGNUM; i++)
        ln_55[i] = 0x55555555;  // bit pattern 0101010101 etc, if rotated +/- 1 yields the inverted pattern 1010101010 etc

    ln2 = ln_rnd;

    for (i = -BITS_IN_WORD; i <= BITS_IN_WORD; i++)
    {
        ln1 = ln_55;
        ln1.rotate(size, i);
        if ( ((i % 2 == 0) && (ln1 == ~ln_55)) ||   // even rotation -> same pattern
             ((i % 2 == 1) && (ln1 != ~ln_55))      // odd rotation -> inverted pattern
           )
        {
            err++;
            eprintf(VERB_GLOB, "Rotate or ~ error at i=%d.\n", i);
            eprintf(VERB_GLOB, "Original longnum:\n");
            ln_55.print_fancy(VERB_GLOB, 11, size, &no_markings);
            eprintf(VERB_GLOB, "rotated longnum:\n");
            ln1.print_fancy(VERB_GLOB, 11, size, &no_markings);
        }

        ln2.rotate(size, i);
        ln2.rotate(size, -i);
        if (ln2 != ln_rnd)
        {
            err++;
            eprintf(VERB_GLOB, "Rotate error, i=%d\n", i);
            eprintf(VERB_GLOB, "Original longnum:\n");
            ln_rnd.print_fancy(VERB_GLOB, 11, size, &no_markings);
            eprintf(VERB_GLOB, "rotated longnum:\n");
            ln2.print_fancy(VERB_GLOB, 11, size, &no_markings);
        }
    }

    *errs += err;
    return err;
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
    telegram_marking[0] = { 0, (int)tel->number_of_userbits, ANSI_COLOR_GREEN };
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
    telegram* telegramlist;

    telegramlist = generate_random_telegrams(count);

    eprintf(VERB_GLOB, "\nShaping random telegrams:\n");
    convert_telegrams_multithreaded(telegramlist, 1, false);

    eprintf(VERB_GLOB, "Checking shaped telegrams:\n");
    convert_telegrams_multithreaded(telegramlist, 1, false);

    return 0;
}

int run_make_long_test(int count, int* errcount)
// checks the make_long function for count times by generating random telegrams and checking the conversion of the short telegrams by SHR'ing them back again
// returns the amount of errors found
{
    int i = 0, err = 0;
    longnum short_data;
    telegram *telegramlist, *p_telegram;

    telegramlist = generate_random_telegrams(count);
    p_telegram = telegramlist;

    // iterate over the telegrams in the list:
    while (p_telegram)
    {
        i++;

        if (p_telegram->number_of_userbits == N_USERBITS_S)
        // only look at short telegrams
        {
            // store the old user data:
            short_data = p_telegram->deshaped_contents;

            // make it a long telegram:
            p_telegram->make_userdata_long();

            // shift the contents right again:
            p_telegram->deshaped_contents >>= (N_USERBITS_L - N_USERBITS_S);

            // check the result:
            if (short_data != p_telegram->deshaped_contents)
            {
                eprintf(VERB_GLOB, ERROR_COLOR "NOK\n" ANSI_COLOR_RESET);
                eprintf(VERB_GLOB, "\nShort telegram (#%d):\n", i);
                short_data.print_bin(VERB_GLOB);
                eprintf(VERB_GLOB, "After calculations:\n");
                p_telegram->deshaped_contents.print_bin(VERB_GLOB);
                err++;
            }
        }

        p_telegram = p_telegram->next;
    }

    *errcount += err;
    return err;
}

void create_off_synch_telegram(telegram* p_telegram, int n_cvw, int start)
// creates a telegram p_telegram to check the implementation of the off-synch-parsing condition-check.
// parameters: n_cvw = #cvw's, start=bit 0 of first vw
{
    int i;

    eprintf(VERB_GLOB, "Creating telegram with n_cvw=%d @ bit#%d:\n", n_cvw, start);

    // clear the telegram:
    p_telegram->contents.fill(0);

    start = start % p_telegram->size;

    // first fill the telegram starting at bit 0 with random transformation words:
    for (i = 0; i < n_cvw; i++)
    {
        p_telegram->contents.write_at_location(i * 11, (t_word*) & transformation_words[rand()% (N_TRANS_WORDS-1)], 11);  
    }

    // then rotate the telegram:
    p_telegram->contents.rotate(p_telegram->size, start);

    p_telegram->contents.print_fancy(VERB_GLOB, 11, p_telegram->size, &no_markings);
}

bool check_off_sync_test(telegram* p_telegram, int index)
// returns true if a sequence of n_cvw+1 valid words is present at bit #index
{
    int j, max_cvw;

    // calculate the max_cvw for the found error location:
    if ((index % 11 == 1) || (index % 11 == 10))
        max_cvw = 2;
    else
        if (p_telegram->size == BITLENGTH_LONG_TELEGRAM)
            max_cvw = 10;
        else
            max_cvw = 6;

    eprintf(VERB_GLOB, "word @ bit#%d = %o\n", index, 0b11111111111 & p_telegram->contents.get_word_wraparound(p_telegram->size, index));
    eprintf(VERB_GLOB, "word @ bit#%d = %o\n", index + 11 * max_cvw, 0b11111111111 & p_telegram->contents.get_word_wraparound(p_telegram->size, index + 11 * max_cvw));

    // check whether all words are valid (should be):
    for (j = 0; j <= max_cvw; j++)
        if (transform_word11_to_word10(0b11111111111 & p_telegram->contents.get_word_wraparound(p_telegram->size, index + 11 * j)) == NO_TW)
        // this word is not a valid transformation word
        {
            eprintf(VERB_PROG, "Error in off sync check\n");
            return false;
        }

    return true;
}


int run_alphabet_test(int* error_count)
// Runs the alphabet condition - test
{
    telegram* p_telegram = new telegram("", s_long);
    int i, j, result, err=0;

    for (i = 0; i < BITLENGTH_LONG_TELEGRAM - 1; i += 11)
        p_telegram->contents.write_at_location(i, transformation_words[i], 11);

    eprintf(VERB_ALL, "Performing alphabet check on telegram with only transformation words:\n");
    p_telegram->contents.print_fancy(VERB_ALL, 11, p_telegram->size, &no_markings);

    // no error should be found:
    result = p_telegram->check_alphabet_condition();
    if (result != MAGIC_WORD)
    {
        err += 1;
        eprintf(VERB_GLOB, "Alphabet check fails.\n");
    }
    else
        eprintf(VERB_ALL, "No error in telegram found.\n");

    // iterate over all words, replace them with a non-transformation word and check that the alphabet-check finds it.
    // note that the alphabet check works from high to low, so there is no need to put the correct word back at position i after checking it.
    for (i = 0; i < BITLENGTH_LONG_TELEGRAM - 1; i += 11)
    {
        // find a non-transformation word:
        do
        {
            j = rand() % 0x3FF;
        } while (transform_word11_to_word10(j) != NO_TW);

        p_telegram->contents.write_at_location(i, j, 11);
        eprintf(VERB_ALL, "Telegram with illegal word %o (octal) @ bit %d:\n", j, i);
        p_telegram->contents.print_fancy(VERB_ALL, 11, p_telegram->size, &no_markings);

        result = p_telegram->check_alphabet_condition();
        if (result != i) 
        {
            err += 1;
            eprintf(VERB_GLOB, "Alphabet check fails.\n");
        }
    }

    *error_count += err;
    return err;
}


int run_off_synch_test(int telegram_size, int* errs)
// Runs the off-synch-parsing condition-check test. Creates many telegrams that should or should not pass the off-synch-parsing condition.
// Often finds unintended positives (a different sequence than the created sequence of non-consecutive valid words was found). In this case, check that the found cvw-sequence is correct.
// Adds the number of found errors to errs, returns the number of found errors (err).
{
    int err = 0, offset, index, n_cvw, max_cvw=0, max_cvw_local=0, local_err=0;
    telegram* p_telegram = new telegram("", s_short);

    for (offset = -10; offset <= 10; offset++)
    // iterate over each bit offset in the telegram
    {
        // determine the max_ncw for this bit offset:
        if (offset % 11 != 0)
        {
            if ((offset % 11 == 1) || (offset % 11 == 10))
                max_cvw = 2;
            else
                if (p_telegram->size == BITLENGTH_LONG_TELEGRAM)
                    max_cvw = 10;
                else
                    max_cvw = 6;
        }
        else
        // offset = 0, no off sync test needed
            continue;

        // iterate over the number of cvw's:
        for (n_cvw = 1; n_cvw <= max_cvw+1; n_cvw++)
        {
            // create a telegram with a sequence of n_cvw @ offset and check it:
            create_off_synch_telegram(p_telegram, n_cvw, offset);
            index = p_telegram->check_off_synch_parsing_condition();
            local_err = 0;

            if (n_cvw > max_cvw)
            // the telegram contains an illegal sequence starting at bit offset
            // add a failure if the algorithm did not find this sequence and no other sequence was found at bit #index
            {
                if (index == MAGIC_WORD)
                // the algorithm did not find any illegal sequence
                {
                    eprintf(VERB_GLOB, "Check off-sync parsing condition failed: no illegal sequence of cvw found.\n");
                    local_err++;
                }
                else                    
                // the algorithm found an illegal sequence, this should be either the intended sequence or another.
                    if ( (index != offset) && !(check_off_sync_test(p_telegram, index)) )
                    {
                        eprintf(VERB_GLOB, "Check off-sync parsing condition failed at bit #%d. Check returned value: %d.\n", offset, index);
                        local_err++;
                    }
            }
            else  
            {
            // n_cvw <= max_cvw; there is no intended illegal sequence in this telegram
                if (index != MAGIC_WORD) 
                // but: the off-synch check gives a positive
                {
                    eprintf(VERB_GLOB, "\nUnexpected positive in off synch telegram: offset=%d, n_cvw=%d, max_cvw=%d, location=%d\n", offset, n_cvw, max_cvw, index);

                    if (!check_off_sync_test(p_telegram, index))
                    // a sequence was found but the check fails
                    {
                        eprintf(VERB_GLOB, "Check off-sync parsing condition failed at bit #%d. Check returned value: %d.\n", offset, index);
                        local_err++;
                    }
                }
            }

            if (local_err)
            // an error occurred, print the contents of the telegram
            {
                eprintf(VERB_GLOB, "Check was performed on this telegram: \n");
                p_telegram->contents.print_fancy(VERB_GLOB, 11, p_telegram->size, &no_markings);
                err += local_err;
            }
        }
    }

    *errs += err;
    return err;
}

int run_aperiodicity_test(int* error_count)
// Runs the "aperiodicity condition for long format"-check test
{
    telegram* p_err_telegram = new telegram("", s_long);
    telegram* p_ok_telegram = new telegram("", s_long);
    int k, i, result, err=0;

    t_longnum_layout err_marking[3] = { 0 };
    err_marking[2].length = 0;   // initialise the last marking to 0

    for (k = -3; k <= 3; k++)
    {
        // fill each byte in the telegram with increasing values, copy the first two words (+k) to bit position 341 (+k) and toggle the first bits -> check fails as the Hamming distance is too small
        for (i = 0; i < WORDS_IN_LONGNUM*4; i++)
            p_err_telegram->contents.write_at_location(i * 8, i, 8);

        p_err_telegram->contents.write_at_location(341+11, p_err_telegram->contents.get_word(11+k), 22);
        p_ok_telegram->contents = p_err_telegram->contents;
        p_err_telegram->contents.write_at_location(11 + k, ~p_err_telegram->contents.get_word(11 + k), (k == 0) ? 2 : 1);  // toggle exactly the amount of bits needed to FAIL the check
        p_ok_telegram->contents.write_at_location(11 + k, ~p_ok_telegram->contents.get_word(11 + k), (k == 0) ? 3 : 2);  // toggle exactly the amount of bits needed to PASS the check

        err_marking[0] = { (k + 11), 22, ANSI_COLOR_RED };
        err_marking[1] = { 341 + 11, 22, ANSI_COLOR_RED };

        eprintf(VERB_ALL, "Created telegram that should fail the aperiodicity test, k=%d:\n", k);
        p_err_telegram->contents.print_fancy(VERB_ALL, 11, p_err_telegram->size, err_marking);

        eprintf(VERB_ALL, "Created telegram that should pass the aperiodicity test, k=%d:\n", k);
        p_ok_telegram->contents.print_fancy(VERB_ALL, 11, p_err_telegram->size, err_marking);

        for (i = 0; i < BITLENGTH_LONG_TELEGRAM / 11; i++)
        {
            // check the erroneous telegram:
            result = p_err_telegram->check_aperiodicity_condition();
            if (result == MAGIC_WORD)
            {
                eprintf(VERB_GLOB, "NOK fail check; k=%d, result=%d.\n", k, result);
                err++;
            }
            else
                eprintf(VERB_ALL, "Aperiodicity check FAIL test was OK at pos=%d, k=%d\n", result, k);

            p_err_telegram->contents.rotate(BITLENGTH_LONG_TELEGRAM, 11);

            // check the OK telegram (note that this could fail due to other words in the random telegram):
            result = p_ok_telegram->check_aperiodicity_condition();
            if (result != MAGIC_WORD)
            {
                eprintf(VERB_QUIET, "NOK ok check; k=%d, result=%d.\n", k, result);
                p_ok_telegram->contents.print_fancy(VERB_QUIET, 11, p_err_telegram->size, err_marking);
                err++;
            }
            else
                eprintf(VERB_ALL, "Aperiodicity check OK test was OK at pos=%d, k=%d\n", result, k);

            p_ok_telegram->contents.rotate(BITLENGTH_LONG_TELEGRAM, 11);
        }
    }

    *error_count += err;
    return err;
}

void create_undersampled_telegram(telegram* p_telegram, int factor, int offset)
// Creates an undersampled telegram p_telegram with undersampling factor "factor", starting at bitnum offset. 
// This telegram will be undersampled by the check and yield a telegram consisting of all valid transformation words, therefore the test will fail.
{
    int i;
    int newbit, wordpointer = 0, bitindex = 0;

    p_telegram->contents.fill(0);

    for (i = 0; i < p_telegram->size; i++)
    {
        if (bitindex == 11)
        // skip to next transformation word:
        {
            wordpointer++;
            bitindex = 0;
        }

        // determine the new bit and write it to the telegram:
        newbit = (transformation_words[wordpointer] >> bitindex) & 1;
        p_telegram->contents.set_bit((i * factor + offset) % p_telegram->size, newbit);
        bitindex++;
    }
}

int run_undersampling_test(t_size telegram_size, int* errs)
// runs the undersampling test. Creates many undersampled telegrams with all possible k's and offsets, check the undersampling check.
{
    int err = 0, j, k;
    telegram* p_telegram = new telegram ("", telegram_size);

    for (k = 2; k <= 16; k *= 2)
    // iterate over the factors 2,4,8,16
    {
        // create an undersampled telegram with factor k at offset 0
        create_undersampled_telegram(p_telegram, k, 0);

        for (j = 0; j < p_telegram->size; j++)
        // iterate over the starting bits
        {
            // calculate the next telegram by rotating it 1 to the left:
            p_telegram->contents.rotate(p_telegram->size, 1);

            if (p_telegram->check_undersampling_condition() != ERR_UNDER_SAMPLING)
            // undersampling check gives false negative:
            {
                printf("NOK");
                eprintf(VERB_GLOB, "\nCreated undersampled telegram (j=%d, k=%d):\n", j, k);
                p_telegram->print_contents_fancy(VERB_GLOB);
                err++;
            }
        }
    }

    *errs += err;
    return err;
}


int test_zp_results(int* error_count)
// Insert the results of Zhuo Peng (SB, ESB) into this program and see if these yield correct telegrams.
// If not, this means that the program contains an error.
{
    // results from Zhuo Peng:
    // encoding the following long telegram (all 1's):
    // FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC
    // this yields 474 valid datasets of (SB, ESB):
    // (18, 709); (18, 1015); (49, 490); (49, 650); (53, 981); (94, 480); (94, 922); (96, 719); (101, 15); (101, 217); (103, 91); (103, 386); (108, 67); (108, 164); (108, 171); (108, 855); (108, 987); (116, 656); (116, 798); (132, 162); (132, 295); (133, 365); (133, 750); (136, 186); (136, 904); (148, 815); (154, 130); (154, 277); (169, 219); (177, 113); (177, 498); (177, 894); (180, 525); (180, 956); (182, 858); (183, 113); (183, 805); (187, 501); (187, 633); (235, 725); (239, 99); (250, 666); (250, 876); (250, 878); (255, 134); (255, 403); (267, 76); (267, 229); (267, 712); (270, 714); (270, 897); (358, 165); (358, 481); (361, 543); (363, 743); (363, 989); (375, 341); (375, 877); (377, 178); (418, 375); (433, 219); (433, 638); (445, 555); (571, 370); (571, 406); (599, 44); (599, 297); (599, 977); (606, 547); (606, 905); (738, 286); (738, 918); (743, 158); (754, 495); (754, 927); (762, 824); (765, 213); (765, 220); (784, 663); (784, 854); (836, 14); (836, 235); (836, 307); (836, 716); (905, 919); (908, 77); (908, 124); (908, 555); (911, 43); (911, 499); (920, 702); (920, 826); (934, 369); (934, 866); (942, 551); (972, 120); (975, 137); (996, 1014); (1002, 176); (1002, 470); (1004, 179); (1004, 212); (1004, 394); (1004, 590); (1004, 868); (1051, 215); (1051, 622); (1051, 668); (1051, 831); (1053, 32); (1053, 126); (1053, 185); (1053, 318); (1053, 654); (1053, 745); (1053, 855); (1053, 894); (1063, 885); (1079, 973); (1083, 805); (1091, 107); (1099, 215); (1099, 785); (1101, 286); (1101, 969); (1112, 134); (1112, 533); (1112, 849); (1116, 88); (1116, 92); (1116, 183); (1142, 955); (1277, 52); (1277, 985); (1283, 910); (1287, 104); (1287, 206); (1287, 349); (1292, 984); (1295, 116); (1295, 163); (1295, 430); (1302, 390); (1313, 120); (1313, 344); (1329, 850); (1338, 654); (1341, 54); (1341, 930); (1357, 344); (1357, 966); (1362, 457); (1415, 995); (1426, 130); (1431, 93); (1442, 125); (1444, 386); (1444, 398); (1448, 703); (1448, 739); (1465, 164); (1480, 49); (1480, 898); (1484, 577); (1484, 588); (1484, 868); (1500, 147); (1500, 313); (1501, 396); (1608, 93); (1629, 114); (1629, 310); (1629, 464); (1629, 538); (1634, 68); (1634, 178); (1636, 156); (1636, 234); (1636, 294); (1636, 1011); (1646, 172); (1654, 556); (1654, 627); (1654, 643); (1670, 864); (1672, 159); (1673, 720); (1673, 803); (1677, 384); (1677, 473); (1677, 897); (1691, 392); (1696, 618); (1696, 961); (1704, 721); (1705, 290); (1705, 348); (1741, 210); (1746, 904); (1754, 59); (1754, 868); (1760, 735); (1760, 820); (1760, 978); (1762, 456); (1787, 293); (1787, 867); (1826, 131); (1826, 667); (1826, 841); (1826, 878); (1826, 898); (1832, 24); (1832, 33); (1832, 104); (1832, 203); (1855, 303); (1974, 907); (1976, 802); (1976, 916); (1976, 925); (2019, 858); (2019, 987); (2068, 577); (2068, 845); (2087, 105); (2087, 111); (2087, 464); (2088, 691); (2093, 200); (2093, 964); (2116, 175); (2116, 291); (2116, 579); (2123, 320); (2123, 723); (2123, 1014); (2124, 383); (2140, 306); (2140, 393); (2140, 542); (2140, 579); (2191, 390); (2191, 494); (2198, 230); (2198, 405); (2204, 121); (2204, 897); (2204, 911); (2209, 481); (2211, 865); (2238, 239); (2238, 731); (2238, 970); (2258, 715); (2296, 934); (2310, 129); (2310, 677); (2337, 200); (2337, 788); (2337, 823); (2345, 542); (2356, 497); (2356, 705); (2367, 374); (2370, 984); (2379, 897); (2399, 400); (2418, 678); (2445, 159); (2445, 842); (2508, 154); (2508, 942); (2508, 980); (2518, 206); (2518, 676); (2637, 152); (2637, 458); (2647, 68); (2647, 218); (2647, 373); (2647, 1001); (2656, 684); (2656, 709); (2656, 715); (2675, 197); (2675, 982); (2699, 709); (2702, 15); (2702, 166); (2703, 18); (2708, 580); (2708, 681); (2708, 831); (2760, 921); (2774, 173); (2774, 625); (2779, 358); (2779, 359); (2792, 65); (2795, 383); (2795, 985); (2806, 865); (2807, 84); (2807, 139); (2807, 216); (2807, 351); (2811, 231); (2811, 318); (2815, 47); (2815, 73); (2815, 89); (2815, 320); (2823, 343); (2823, 853); (2846, 129); (2846, 219); (2846, 406); (2854, 96); (2859, 159); (2870, 126); (2871, 353); (2871, 905); (2878, 712); (2917, 212); (2917, 490); (2930, 470); (2930, 497); (2931, 902); (2976, 549); (2976, 810); (2977, 468); (2977, 521); (2978, 45); (2978, 522); (3018, 179); (3018, 871); (3028, 873); (3101, 171); (3101, 294); (3101, 812); (3235, 350); (3240, 164); (3240, 722); (3256, 116); (3256, 154); (3257, 851); (3296, 604); (3296, 643); (3296, 777); (3319, 155); (3319, 386); (3319, 494); (3319, 829); (3319, 951); (3330, 494); (3339, 70); (3339, 989); (3353, 854); (3362, 287); (3362, 426); (3362, 838); (3369, 617); (3369, 651); (3384, 180); (3384, 886); (3384, 947); (3384, 970); (3386, 673); (3467, 618); (3467, 726); (3493, 487); (3493, 809); (3495, 239); (3495, 839); (3495, 879); (3495, 898); (3501, 133); (3503, 93); (3503, 229); (3510, 201); (3515, 498); (3521, 122); (3523, 294); (3523, 471); (3523, 804); (3523, 909); (3525, 125); (3525, 972); (3620, 885); (3625, 819); (3632, 703); (3637, 178); (3651, 352); (3651, 357); (3651, 636); (3660, 388); (3660, 792); (3663, 153); (3674, 147); (3674, 741); (3674, 846); (3676, 237); (3704, 146); (3705, 848); (3713, 345); (3732, 115); (3732, 232); (3732, 288); (3736, 25); (3736, 55); (3736, 678); (3741, 96); (3741, 984); (3747, 602); (3747, 853); (3751, 323); (3756, 86); (3756, 238); (3756, 720); (3756, 886); (3779, 958); (3784, 109); (3784, 654); (3784, 819); (3789, 170); (3797, 469); (3797, 559); (3797, 862); (3797, 995); (3809, 162); (3809, 852); (3810, 1013); (3815, 91); (3824, 819); (3824, 861); (3826, 533); (3827, 870); (3828, 289); (3829, 842); (3837, 308); (3839, 358); (3845, 388); (3864, 815); (3865, 906); (3907, 164); (3907, 402); (3907, 931); (3914, 168); (3914, 493); (3914, 833); (3920, 616); (3920, 822); (3920, 977); (3922, 68); (3922, 745); (3928, 860); (3944, 54); (3944, 201); (3944, 648); (3944, 831); (3948, 540); (3948, 623); (3961, 526); (3965, 566); (3965, 1014); (4001, 10); (4001, 133); (4009, 344); (4009, 477); (4009, 926); (4011, 302); (4011, 446); (4069, 535);

    int i = 0, result = 0, err_location, local_error = 0;
    telegram* p_telegram = new telegram(zp_test_telegram, s_long);
    longnum Utick;
    t_word cb = 1;

    p_telegram->align(a_calc);
    Utick = p_telegram->deshaped_contents;
    p_telegram->determine_U_tick(Utick);
    p_telegram->set_control_bits(cb);

    while ((zp_results[i].esb != 0) && (zp_results[i].sb != 0))
    // iterate over ZP's results
    {
        eprintf(VERB_ALL, "i=%d; sb=%d; esb=%d; ", i, zp_results[i].sb, zp_results[i].esb);
        
        int word9 = (zp_results[i].esb >> 3) + (zp_results[i].sb << 7);
        int word10 = 0b00100000000 + (zp_results[i].sb >> 4);
        eprintf(VERB_ALL, "%d;%d;%d;%d;%d\n", i, zp_results[i].sb, zp_results[i].esb, word9, word10);

        p_telegram->set_scrambling_bits(zp_results[i].sb);
        p_telegram->set_extra_shaping_bits(zp_results[i].esb);
        result = p_telegram->scramble_transform_check_user_data(p_telegram->determine_S(), H, Utick);
        if (result != ERR_NO_ERR)
        // there was an error while scrambling with these sb/esb
        {
            eprintf(VERB_GLOB, "Scrambling error in telegram #%d. Result = %d\n", i, result);
            i++;
            local_error++;
            continue;
        }

        p_telegram->compute_check_bits();
        eprintf(VERB_ALL, "\nChecking new telegram:\n");
        p_telegram->print_contents_fancy(VERB_ALL);
        result = p_telegram->perform_candidate_checks(VERB_ALL, &err_location);
        if (result != ERR_NO_ERR)
        {
            eprintf(VERB_GLOB, "Shaping error in telegram #%d. Result = %d\n", i, result);
            local_error++;
        }
        else
            eprintf(VERB_ALL, "Result: OK\n");

        i++;
    }

    *error_count += local_error;
    return local_error;
}


int check_against_zp(int* error_count)
// Calculates all possible variations of a long telegram with all 1's and verifies this against Zhuo Peng's results
// Note that the order of the results is not checked, this could lead to false positives
{
    int i=0, local_error=0;
    telegram* p_telegram;

    // create the telegram and the shortlist parameters:
    telegram *p_test_telegram = new telegram(zp_test_telegram, s_long);
    p_test_telegram->action = act_shape;

    // calculate all possible telegrams:
    telegram_calc_all(p_test_telegram);

    // check the outcome against the results of ZP:
    p_telegram = p_test_telegram;
    while (p_telegram)
    {
        if ((zp_results[i].sb != p_telegram->get_scrambling_bits()) || (zp_results[i].esb != p_telegram->get_extra_shaping_bits()))
        // an error was found
        {
            eprintf(VERB_GLOB, "Error: i=%d; SB=%d; ESB=%d", i, p_telegram->get_scrambling_bits(), p_telegram->get_extra_shaping_bits());
            local_error++;
        }

        p_telegram = p_telegram->next;
        i++;
    }

    *error_count += local_error;
    return local_error;
}

/*
void wraparoundcoloringtest(void)
// test wraparound coloring of longnum
{
    telegram* p_telegram = create_random_telegram();

    t_longnum_layout telegram_marking[] = { {100, 20, ANSI_COLOR_RED}, 
                                          //  {p_telegram->number_of_userbits - 10, 20, ANSI_COLOR_GREEN}, 
                                            {p_telegram->number_of_userbits - 60, 120, ANSI_COLOR_YELLOW},
                                          //  {33, 20, ANSI_COLOR_BLUE},
                                            { 0,0,0 } 
                                          };

    p_telegram->deshaped_contents.print_fancy(0, 11, p_telegram->number_of_userbits, telegram_marking);
    exit(0);
}
*/

int main(void)
// runs various high and low level tests
// returns 0 if no error found, !=0 if error found
{
    int error_count = 0;

    // randomise:
    srand((unsigned)time(NULL));

//    wraparoundcoloringtest();

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

    printf("Testing rotation and inversion:\t\t\t");
    print_result(test_rotate_and_inversion(&error_count));

    // test longnum division and multiplication:
    printf("Testing 50*GF2-divison & multiplication:\t");
    print_result(test_divisions(50, &error_count));

    // test encoding schemes:
    printf("Testing HEX and BASE64 encoding/decoding:\t");
    print_result(run_encoding_decoding_test(100, &error_count));
    
    // test telegram creation functions:
    printf("Testing make_long:\t\t\t\t");
    print_result(run_make_long_test(50, &error_count));

    // test telegram candidate checks (see subset 36, 4.3.2.5)
    printf("Testing alphabet condition check:\t\t");
    print_result(run_alphabet_test(&error_count));

    printf("Testing off-synch-parsing condition check:\t");
    print_result(run_off_synch_test(BITLENGTH_LONG_TELEGRAM, &error_count));

    printf("Testing aperiodicity condition check:\t\t");
    print_result(run_aperiodicity_test(&error_count));

    printf("Testing undersampling check, long telegrams:\t");
    print_result(run_undersampling_test(s_long, &error_count));

    printf("Testing undersampling check, short telegrams:\t");
    print_result(run_undersampling_test(s_short, &error_count));

    // test shaping and deshaping:
    printf("Running multithreaded shape/deshape test with 100 random telegrams:\t");
    print_result(run_shape_deshape_list_test(100, &error_count));

    // test agains Zho Peng's results:
    printf("Checking results of Zhuo Peng:\t\t\t");
    print_result(test_zp_results(&error_count));

    printf("Re-calculating results of Zhuo Peng:\t\t");
    print_result(check_against_zp(&error_count));

    printf("\nFinished running tests. Result:\t\t\t");
    print_result(error_count);

    return error_count;
}
