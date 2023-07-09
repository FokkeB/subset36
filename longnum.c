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

#include <string.h>  // for memxxx-functions
#include "longnum.h"

// define the parameters of the longnum:
const int BITS_IN_WORD = sizeof(t_word) * 8;                        // nr of bits in one t_word
const int BITS_IN_LONGNUM = sizeof(t_word) * 8 * LONGNUM_LENGTH;    // nr of bits in one t_longnum

void long_shiftleft (t_longnum longnum, int count)
// shifts the bits in longnum left with count bits
// if count >= #bits, longnum = 0 
// if count <= 0, do nothing
{
    int i, wordshift, bitshift;
 
    if (count <= 0)
        return;
        
    if (count >= BITS_IN_LONGNUM)
    {
        long_fill (longnum, 0);
        return;
    }

    wordshift = count / BITS_IN_WORD;
    bitshift = count % BITS_IN_WORD;

    if (wordshift > 0)
    // first shift whole words:
        for (i=LONGNUM_LENGTH-1; i>=0; i--)
            if (i>=wordshift) 
                longnum[i] = longnum[i-wordshift]; 
            else
                longnum[i] = 0;    

    if (bitshift > 0)
    // then shift bits within the word
        for (i=LONGNUM_LENGTH-1; i>=wordshift; i--) 
        {
            longnum[i] = (longnum[i] << bitshift); 
            if (i > wordshift)
            // and copy the bits from the word to the right if needed
                longnum[i] |= (longnum[i-1] >> (BITS_IN_WORD-bitshift)); 
        }        
}

void long_shiftright (t_longnum longnum, int count)
// shifts the bits in longnum right with count bits
// if count >= #bits, longnum = 0 
// if count <= 0, do nothing
{
    int i, wordshift, bitshift;
 
    if (count <= 0)
        return;

    if (count >= BITS_IN_LONGNUM)
    {
        long_fill (longnum, 0);
        return;
    }

    wordshift = count / BITS_IN_WORD;
    bitshift = count % BITS_IN_WORD;

    if (wordshift > 0)
    // first shift whole words:
        for (i=0; i<LONGNUM_LENGTH; i++)
            if (i<LONGNUM_LENGTH-wordshift) 
                longnum[i] = longnum[i+wordshift]; 
            else
                longnum[i] = 0;    

    if (bitshift > 0)
    // then shift bits within the word    
        for (i=0; i<LONGNUM_LENGTH-wordshift; i++) 
        {
            longnum[i] = (longnum[i] >> bitshift);
            if (i<(LONGNUM_LENGTH-1)) 
            // and copy the bits from the word to the left (except when we're looking at the last word)
                longnum[i] |= (longnum[i+1] << (BITS_IN_WORD-bitshift));                 
        }        
}

void long_xor (t_longnum longnum1, t_longnum longnum2, t_longnum longnumresult)
// performs a xor between longnum1 and longnum2, fills longnumresult with the result
// note: no check on size of the longnums, as they are all of equal length LONGNUM_LENGTH
// note: this function is equal to addition and subtraction in GF2
// note: longnumresult can be a new variable or longnum1/2
{
    int i;

    for (i=0; i<LONGNUM_LENGTH; i++)
        longnumresult[i] = longnum1[i]^longnum2[i];
}

void long_fill(t_longnum longnum, int value)
// sets all bits in longnum to the indicated value (0 or 1)
{
    int i=0;
    t_word new_word = 0;

    if (value)
    // set new_word to all 1's if value is true, otherwise keep it at 0
        new_word = ~new_word;

    for (i=0; i<LONGNUM_LENGTH; i++)
    // iterate and fill
        longnum[i] = new_word;
}

t_word long_get_word (t_longnum longnum, int bitnum)
// returns the t_word at bit position bitnum [0..#BITS-1]
// fills the highest part of the return values with 0's if the last word is requested
{
    int word_index, bit_index;
    t_word retval = 0;

    if ( (bitnum >= BITS_IN_LONGNUM) || (bitnum < 0) )
    // return 0 if bitnum out of range
        return 0;

    word_index = bitnum / BITS_IN_WORD;
    bit_index = bitnum % BITS_IN_WORD;

    retval = (longnum[word_index] >> bit_index);

    if ( (bit_index > 0) && (word_index < LONGNUM_LENGTH) )
    // check needed to prevent <<BITS_IN_WORD, which != 0
    // also, shift in from higher word is not needed for highest word
        retval |= (longnum[word_index+1] << (BITS_IN_WORD-bit_index));

    return retval; 
}

t_word long_get_word_wraparound (t_longnum longnum, int size, int bitnum)
/** returns the t_word @ position bitnum, wraps around @ size or bit 0 if needed.
 * 
 * description:
 * first: (bitnum mod size) to deal with bit positions outside of the telegram
 * bitnum < 0 and >= size now do not occur anymore
 * if 0 <= bitnum < size-BITS_IN_WORD+1 -> get value of two consecutive words [bitnum .. bitnum+BITS_IN_WORD-1]
 * if size-BITS_IN_WORD+1 <= bitnum < size: 
 *      lower bits are [bitnum .. size]
 *      higher bits are [0..size-bitnum]
*/
{
    int i, n;
    t_word retval = 0;

    n = (bitnum % size + size) % size;  // n is new bitnum, guaranteed to lie within the boundaries of size. Use of weird modulo to deal with negative bitnums. 

    for (i=n+BITS_IN_WORD-1; i>=n; i--)
    // iterate over the bits and determine the right bit to fetch
    {
        retval <<= 1;  // SHL 1    

        if (i < size)
            retval |= long_get_bit (longnum, i);
        else
            retval |= long_get_bit (longnum, i-size);
    }

    return retval;
}

int long_get_bit (t_longnum longnum, int bitnum)
// returns the requested value (0 or 1) of the bit at bitnum [0..N-1]
// returns 0 if bitnum is out of range
{
    if ((bitnum < 0) || (bitnum >= BITS_IN_LONGNUM))
    // requested bit is out of range, return 0
        return 0;

    int word_index = bitnum / BITS_IN_WORD;
    int bit_index = bitnum % BITS_IN_WORD;

    return ( (longnum[word_index] & (1 << bit_index)) > 0 );
}

void long_setbit (t_longnum longnum, int bitnum, int value)
// sets bit #bitnum in longnum to the indicated value 0 or 1. Bitnum = [0..N-1].
{
    if ((bitnum < 0) || (bitnum >= BITS_IN_LONGNUM))
        // requested bit is out of range, do nothing and return
        return;

    int word_index = bitnum / BITS_IN_WORD;
    int bit_index = bitnum % BITS_IN_WORD;

    t_word t = (1<<bit_index);

    if (value)
        longnum[word_index] |= t;
    else
        longnum[word_index] &= ~t;    
}

void long_copy (t_longnum to, t_longnum from)
// copies the contents of from to to
{
    memcpy (to, from, sizeof(t_longnum));  
}

int long_cmp (t_longnum ln1, t_longnum ln2)
// returns true if contents of ln1 and ln2 are the same, false otherwise
{
    return (memcmp (ln1, ln2, sizeof (t_longnum)) == 0);
}

void long_write_at_location (t_longnum longnum, int location, t_word *newvalue, int n_bits)
// writes the first n bits of newvalue to longnum @ bitposition location [0..N-1]
// stops when the last bit of longnum is reached
{
    if ( (location >= BITS_IN_LONGNUM) || (location < 0) )
    // location outside of longnum; don't do anything
        return;

    if (location + n_bits >= BITS_IN_LONGNUM)
    // shorten n_bits to stay within longword
        n_bits = BITS_IN_LONGNUM - location;

    for (int i=0; i<n_bits; i++)
    // iterate over newvalues, put them in the right place
    {
        int n_word = i / BITS_IN_WORD;
        int n_bit = i % BITS_IN_WORD;

        long_setbit(longnum, location + i, (newvalue[n_word]>>n_bit) & 1);
    }
}

int get_order (t_longnum longnum)
// returns the order of longnum (position of highest bit that is 1, 0 if longnum==0)
{
    int wordnum=LONGNUM_LENGTH-1;
    int i;

    // coming from the MSB, find the first word that is > 0:
    while ( (longnum[wordnum] == 0) && (wordnum >= 0) )
        wordnum--;

    if (wordnum >= 0)
    // found one, now find the highest bit in that word that is 1
    {
        i = BITS_IN_WORD-1;

        while ( (i >= 0) && !(longnum[wordnum] & (1 << i)) )
            i--;

        return (wordnum*BITS_IN_WORD + i + 1);
    }
    else
    // no set bit found (longnum==0), return order 0
        return 0;

    // this return should never occur:
    return -1;
}

void array_to_longnum(uint8_t* arr, t_longnum ln, int n)
// converts the first n bytes [0..n] in the char array arr to a longnum ln (setting bits [n*8-1 .. 0] )
{
    int j;
    t_word newbyte;

    for (j = n - 1; j >= 0; j--)
    {
        newbyte = (t_word)arr[j] & 0xFF;
        long_write_at_location(ln, (n - j - 1) * 8, &newbyte, 8);
    }
}

void longnum_to_array(uint8_t* arr, t_longnum ln, int n)
// converts the first n bytes [0..n] in longnum ln to the char array arr 
{
    int j;

    for (j = n - 1; j >= 0; j--)
        arr[n-j-1] = long_get_word(ln, j * 8) & 0xFF;
}

/*
void longnum_reverse (t_longnum longnum)
// reverses the bit order of longnum
// todo: "order" is not the right size, because if MSB=0 it will be skipped
{
    int i, order = get_order (longnum);
    t_longnum tmp;

    long_copy (tmp, longnum);
    long_fill (longnum, 0);

    for (i=order-1; i>=0; i--)  //BITS_IN_WORD*LONGNUM_LENGTH-1
        if (long_get_bit(tmp, i))
            long_setbit (longnum, order-i-1, 1);
}
*/
/*
int long_find_bit_pattern (t_longnum longnum, unsigned int findme, int n)
// finds bit pattern in findme (n bits) in longnum, prints out the bit numbers of the positions, returns -1 if not found, 1 if found
{
    int i, result=-1;
    t_word tmp;

    for (i=0; i<LONGNUM_LENGTH*BITS_IN_WORD-n; i++)
    {
        tmp = long_get_word (longnum, i);
        tmp &= ~(0xFFFFFFFF << n);  // mask out all the bits higher than n
        if (findme == tmp)
        {
            printf ("%d,",i);
            result=1;
        }
    }

    return result;
}
*/
void print_longnum_bin (int v, t_longnum longnum)
// prints longnum in a structured way, nibble by nibble
// ends with the order of longnum and a CR
// uses v as verbosity level
{
    for (int j=(get_order(longnum)-1)/BITS_IN_WORD; j>=0; j--)
//    for (int j=LONGNUM_LENGTH-1; j>=0; j--)
    {
        eprintf (v, "#%02d=", j);
        for (int i=BITS_IN_WORD-1; i>=0; i--) 
        {
            if (longnum[j]>>i & 1)
                eprintf(v, "1");
            else
                eprintf(v, "0");

            if (i%4==0)
                eprintf(v, " ");
        }
    }

    eprintf(v, " order: %d\n", get_order(longnum));
}

void print_longnum_hex (int v, t_longnum longnum)
// prints the hex values in longnum using verbosity level v
{
    int j;

    for (j=((get_order(longnum)-1)/BITS_IN_WORD); j>=0; j--) 
        eprintf (v, "%08X", longnum[j]);
}

int sprint_longnum_hex(char* line, t_longnum longnum, int n)
// writes the hex values in longnum to *line (no range check)
// returns the amount of chars written
// write per byte (instead of per whole word) to prevent unwanted leading 0's
{
    int j;
    char t[10];
    *line = 0;
    t_word w;

    for (j = (n-1) / 8; j >= 0; j--)
    {
        w = long_get_word(longnum, j*8);
        sprintf(t, "%02X", w&0xFF);
        strcat(line, t);
    }

     return strlen(line);
}
/*
int sprint_longnum_hex(char* line, t_longnum longnum)
// writes the hex values in longnum to *line (no range check)
// returns the amount of chars written
// todo: do not use get_order (high bits could be 000)
{
    int j;
    char t[10];
    *line = 0;

    for (j = ((get_order(longnum) - 1) / BITS_IN_WORD); j >= 0; j--)
    {
        sprintf(t, "%08X", longnum[j]);
        strcat(line, t);
    }

    return strlen (line);
}
*/
void print_longnum_fancy(int v, t_longnum longnum, int wordlength, int size, t_longnum_layout *longnum_layout)
/**
 * prints the longnum binary, in a structured way, with colors!
 * 
 * - starts at the MSB (printing from bit [size .. 0])
 * - prints with a wordlength of wordlength
 * - uses the indicated color 
 * - prints the bitnum of the first bit of the word in dark grey
 * - prints the bits in the indicated color (start, length, color).  Last struct must have length set to 0.
 * - uses v as verbosity level
*/
{
    int i,layout_count, layout_index;
    char clear_color[] = ANSI_COLOR_RESET;
    char *current_color = clear_color; 

    for (layout_count=0; layout_count<MAX_N_LAYOUT; layout_count++)
        if (longnum_layout[layout_count].length==0)
            break;

    if (size % wordlength != 0)
        for (i = 0; i < wordlength-(size % wordlength); i++)
            eprintf(v, " ");

    for (i=size-1; i>=0; i--)
    // iterate over the bits
    {
        for (layout_index=0; layout_index<layout_count; layout_index++)
        // see if this bits needs some coloring
        {
            if (longnum_layout[layout_index].start+longnum_layout[layout_index].length-1 == i) 
            {
                current_color = longnum_layout[layout_index].color;
                eprintf(v, current_color);
                break;
            }
        }

        // print the actual bit
        if (long_get_bit (longnum, i))
            eprintf(v, "1");
        else
            eprintf(v, "0");

        // stop coloring after this bit if needed
        for (layout_index=0; layout_index<layout_count; layout_index++)
        {
            if (longnum_layout[layout_index].start == i) 
            {
                current_color = clear_color;    
                eprintf(v, current_color);
                break;
            }
        }

        if (i%wordlength==0)
        // print the bit number
        {
            eprintf(v, BITNUM_COLOR"(%04d)  ", i);
            eprintf(v, current_color);
        }    

        if ( (i%(wordlength*4) == 0) && (i!=0) )
        // add a line break after 4 numbers
            eprintf(v, "\n");
    }

    eprintf(v, "\nOrder: %d\n", get_order(longnum));
}