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

#include "longnum.h"

longnum::longnum(int with)
// constructor, initialise value
{
    fill(with);
}

longnum::longnum(t_word* init_val, int count)   
// constructor, fills with indicated values
{
    int i;

    for (i = 0; i < count; i++)
        value[i] = init_val[i];
}

longnum longnum::operator << (int count) const
// shift left << operator overload
{
    longnum result = *this;
    result <<= count;
    return result;
}

longnum& longnum::operator <<= (int count) 
// shifts the bits in longnum left with count bits, returns the result by reference
// if count >= #bits, longnum = 0 
// if count <= 0, do nothing
{
    int i, wordshift, bitshift;

    if (count <= 0)
        return *this;

    if (count >= BITS_IN_LONGNUM)
    //  fill with 0's if all bits are shifted out
    {
        fill(0);
        return *this;
    }

    wordshift = count / BITS_IN_WORD;
    bitshift = count % BITS_IN_WORD;

    if (wordshift > 0)
        // first shift whole words:
        for (i = WORDS_IN_LONGNUM - 1; i >= 0; i--)
            if (i >= wordshift)
                value[i] = value[i - wordshift];
            else
                value[i] = 0;

    if (bitshift > 0)
        // then shift bits within the word
        for (i = WORDS_IN_LONGNUM - 1; i >= wordshift; i--)
        {
            value[i] = (value[i] << bitshift);
            if (i > wordshift)
                // and copy the bits from the word to the right if needed
                value[i] |= (value[i - 1] >> (BITS_IN_WORD - bitshift));
        }

    return *this;
}

longnum longnum::operator >> (int count) const
// shift right >> operator overload
{
    longnum result = *this;
    result >>= count;
    return result;
}

longnum& longnum::operator >>= (int count)
// shifts the bits in longnum right with count bits
// if count >= #bits, longnum = 0 
// if count <= 0, do nothing
{
    int i, wordshift, bitshift;

    if (count <= 0)
        return *this;

    if (count >= BITS_IN_LONGNUM)
    {
        fill(0);
        return *this;
    }

    wordshift = count / BITS_IN_WORD;
    bitshift = count % BITS_IN_WORD;

    if (wordshift > 0)
        // first shift whole words:
        for (i = 0; i < WORDS_IN_LONGNUM; i++)
            if (i < WORDS_IN_LONGNUM - wordshift)
                value[i] = value[i + wordshift];
            else
                value[i] = 0;

    if (bitshift > 0)
        // then shift bits within the word    
        for (i = 0; i < WORDS_IN_LONGNUM - wordshift; i++)
        {
            value[i] = value[i] >> bitshift;
            if (i < (WORDS_IN_LONGNUM - 1))
                // and copy the bits from the word to the left (except when we're looking at the last word)
                value[i] |= (value[i + 1] << (BITS_IN_WORD - bitshift));
        }

    return *this;
}

longnum longnum::operator ^ (const longnum& xor_with) const
// performs a xor between value and xor_with, returns the result by reference
// note: no check on size of the longnums, as they are all of equal length WORDS_IN_LONGNUM
// note: this function is equal to addition and subtraction in GF2 (see overloads of + and -)
{
    longnum ln_result = *this;
    ln_result ^= xor_with;
    return ln_result;
}

longnum& longnum::operator ^= (const longnum& xor_with)
// ^= operator overload
{
    int i;

    for (i = 0; i < WORDS_IN_LONGNUM; i++)
        value[i] = value[i] ^ xor_with.value[i];

    return *this;
}

t_word& longnum::operator [] (int i)
// returns the indicated t_word, operator [] overload
{
    return value[i];
}

void longnum::fill(int new_value)
// sets all bits in longnum to the indicated value (0 or 1) or random (-1)
{
    int i = 0;
    t_word new_word = 0;

    if (new_value == FILL_RANDOM)
    // fill with random values
    {
        for (i = 0; i < WORDS_IN_LONGNUM; i++)
            value[i] = ((rand() << 16) | rand());
        return;
    }

    if (new_value)
        // set new_word to all 1's if value is true, otherwise keep it at 0
        new_word = ~new_word;

    for (i = 0; i < WORDS_IN_LONGNUM; i++)
        // iterate and fill
        value[i] = new_word;
}

t_word longnum::get_word(const int bitnum) const
// returns the t_word at bit position bitnum [0..#BITS-1]
// fills the highest part of the return values with 0's if the last word is requested
{
    int word_index, bit_index;
    t_word retval = 0;

    if ((bitnum >= BITS_IN_LONGNUM) || (bitnum < 0))
    // return 0 if bitnum out of range
        return 0;

    word_index = bitnum / BITS_IN_WORD;
    bit_index = bitnum % BITS_IN_WORD;

    retval = (value[word_index] >> bit_index);

    if ((bit_index > 0) && (word_index < WORDS_IN_LONGNUM-1))
        // check needed to prevent <<BITS_IN_WORD, which != 0
        // also, shift in from higher word is not needed for highest word
        retval |= (value[word_index + 1] << (BITS_IN_WORD - bit_index));

    return retval;
}

t_word longnum::get_word_wraparound(const int size, const int bitnum) const
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
    int i, n=0;
    t_word retval = 0;

    if (size)
        n = (bitnum % size + size) % size;  // n is new bitnum, guaranteed to lie within the boundaries of size. Use of weird modulo to deal with negative bitnums. 

    for (i = n + BITS_IN_WORD - 1; i >= n; i--)
        // iterate over the bits and determine the right bit to fetch
    {
        retval <<= 1;    

        if (i < size)
            retval |= get_bit(i);
        else
            retval |= get_bit(i - size);
    }

    return retval;
}

int longnum::get_bit(const int bitnum) const
// returns the requested value (0 or 1) of the bit at bitnum [0..N-1]
// returns 0 if bitnum is out of range
{
    if ((bitnum < 0) || (bitnum >= BITS_IN_LONGNUM))
        // requested bit is out of range, return 0
        return 0;

    int word_index = bitnum / BITS_IN_WORD;
    int bit_index = bitnum % BITS_IN_WORD;

    return ((value[word_index] & (1 << bit_index)) > 0);
}

void longnum::set_bit(const int bitnum, const int newvalue)
// sets bit #bitnum in longnum to the indicated value 0 or 1. Bitnum = [0..N-1].
{
    if ((bitnum < 0) || (bitnum >= BITS_IN_LONGNUM))
        // requested bit is out of range, do nothing and return
        return;

    int word_index = bitnum / BITS_IN_WORD;
    int bit_index = bitnum % BITS_IN_WORD;

    t_word t = (1 << bit_index);

    if (newvalue)
        value[word_index] |= t;
    else
        value[word_index] &= ~t;
}

bool longnum::operator == (const longnum ln2)
// returns true if contents of ln1 and ln2 are the same, false otherwise, == operator overload
{
    return (memcmp(value, ln2.value, sizeof(t_longnum)) == 0);
}

bool longnum::operator != (const longnum ln2)
// returns true if contents of ln1 and ln2 differ, false if they are equal, != operator overload
{
    return !(*this == ln2); 
}

void longnum::write_at_location(unsigned int location, const t_word* newvalue, int n_bits)
// writes the first n bits of newvalue (array of t_word) to longnum @ bitposition location [0..N-1]
// stops when the last bit of longnum is reached
{
    if (location >= BITS_IN_LONGNUM) 
    // location outside of longnum; don't do anything
        return;

    if (location + n_bits >= BITS_IN_LONGNUM)
    // shorten n_bits to stay within longword
        n_bits = BITS_IN_LONGNUM - location;

    for (int i = 0; i < n_bits; i++)
    // iterate over newvalues, put them in the right place
    {
        int n_word = i / BITS_IN_WORD;
        int n_bit = i % BITS_IN_WORD;

        set_bit(location + i, (newvalue[n_word] >> n_bit) & 1);
    }
}

void longnum::write_at_location(unsigned int location, const t_word newvalue, int n_bits)
// writes the first n_bits of newvalue to the longnum
{
    write_at_location(location, &newvalue, n_bits);
}

int longnum::get_order(void) const
// returns the order of longnum (position of highest bit that is 1, 0 if longnum==0)
{
    int wordnum = WORDS_IN_LONGNUM - 1;
    int i;

    // coming from the MSB, find the first word that is > 0:
    while ((value[wordnum] == 0) && (wordnum >= 0))
        wordnum--;

    if (wordnum >= 0)
    // found one, now find the highest bit in that word that is 1
    {
        i = BITS_IN_WORD - 1;

        while ((i >= 0) && !(value[wordnum] & (1 << i)))
            i--;

        return (wordnum * BITS_IN_WORD + i + 1);
    }
    else
        // no set bit found (longnum==0), return order 0
        return 0;

    // this return should never occur:
    return -1;
}

void longnum::read_from_array(uint8_t* arr, int n)
// converts the first n bytes [0..n] in the char array arr to a longnum ln (setting bits [n*8-1 .. 0] )
{
    int j;
    t_word newbyte;

    for (j = n - 1; j >= 0; j--)
    {
        newbyte = (t_word)arr[j] & 0xFF;
        write_at_location((n - j - 1) * 8, &newbyte, 8);
    }
}

void longnum::write_to_array(uint8_t* arr, int n) const
// converts the first n bytes [0..n] in longnum ln to the char array arr 
{
    int j;

    for (j = n - 1; j >= 0; j--)
        arr[n - j - 1] = get_word(j * 8) & 0xFF;
}

void longnum::print_bin(int v) const
// prints longnum in a structured way, nibble by nibble
// ends with the order of longnum and a CR
// uses v as verbosity level
{
    int j, order = get_order();

    return_if_silent(v);

    for (j = (order / BITS_IN_WORD); j >= 0; j--)
    {
        printf("#%02d=", j);
        for (int i = BITS_IN_WORD - 1; i >= 0; i--)
        {
            if (value[j] >> i & 1)
                printf("1");
            else
                printf("0");

            if (i % 4 == 0)
                printf(" ");
        }
    }

    printf(" order: %d\n", order);
}

void longnum::print_hex(int v, int n) const
// prints the hex values in longnum (n bits) using verbosity level v
// write per byte (instead of per whole word) to prevent unwanted leading 0's
{
    int j;

    return_if_silent(v);

    for (j = (n - 1) / 8; j >= 0; j--)
        printf("%02X", get_word(j * 8) & 0xFF);
}

int longnum::sprint_hex(string& line, int n) const
// writes the first n bits in longnum to *line in hex format    
// returns the amount of chars written
// write per byte (instead of per whole word) to prevent unwanted leading 0's 
{
    int j;
    t_word w;
    char temp[3];

    line = "";

    for (j = (n - 1) / 8; j >= 0; j--)
    {
        w = get_word(j * 8);
        //sprintf_s(temp, 3, "%02X", w & 0xFF);
        snprintf(temp, sizeof(temp), "%02X", w & 0xFF);
        line += temp;
    }

    return (int)line.length();
}

int longnum::sprint_base64(string& line, int n) const
// writes the first n bits in longnum to *line in base64 format
// returns the amount of chars written
{
    uint8_t arr[WORDS_IN_LONGNUM*8] = { 0 };

    write_to_array(arr, n/8+1);
    b64_encode(arr, n/8+1, line);

    return (int)line.length();
}

void longnum::print_fancy(int v, int wordlength, int size, t_longnum_layout* longnum_layout) const
/**
 * prints the longnum binary, in a structured way, with colors! 8-o
 *
 * - starts at the MSB (printing from bit [size .. 0])
 * - prints with a wordlength of wordlength
 * - uses the indicated color
 * - prints the bitnum of the first bit of the word in dark grey
 * - prints the bits in the indicated color (start, length, color).  Last struct must have length set to 0. Set *longnum_layout to NULL if no coloring is needed.
 * - uses v as verbosity level
 * TBD: wraparound coloring
*/
{
    int i, layout_count, layout_index;
    char clear_color[] = ANSI_COLOR_RESET;
    char* current_color = clear_color;

    return_if_silent(v);

    if (longnum_layout == NULL)
        // no layout colors specified
        layout_count = 0;
    else
        // count the amount of colors
        for (layout_count = 0; layout_count < MAX_N_LAYOUT; layout_count++)
            if (longnum_layout[layout_count].length == 0)
                break;

    // print some spaces to align the first part
    if (size % wordlength != 0)
        for (i = 0; i < wordlength - (size % wordlength); i++)
            printf(" ");

    for (i = size - 1; i >= 0; i--)
        // iterate over the bits
    {
        for (layout_index = 0; layout_index < layout_count; layout_index++)
            // see if this bits needs some coloring
        {
            if (longnum_layout[layout_index].start + longnum_layout[layout_index].length - 1 == i)
            {
                current_color = longnum_layout[layout_index].color;
                printf("%s", current_color);
                break;
            }
        }

        // print the actual bit
        if (get_bit(i))
            printf("1");
        else
            printf("0");

        // stop coloring after this bit if needed
        for (layout_index = 0; layout_index < layout_count; layout_index++)
        {
            if (longnum_layout[layout_index].start == i)
            {
                current_color = clear_color;
                printf("%s", current_color);
                break;
            }
        }

        if (i % wordlength == 0)
            // print the bit number
        {
            printf(BITNUM_COLOR"(%04d)  ", i);
            printf("%s", current_color);
        }

        if ((i % (wordlength * 4) == 0) && (i != 0))
            // add a line break after 4 numbers
            printf("\n");
    }

    printf("\nOrder: %d\n", get_order());
}

longnum longnum::operator * (const longnum& with) const
// * operator overload, performs a GF2-multiplication
{
    longnum result = *this;
    result *= with;
    return result;
}

longnum& longnum::operator *= (const longnum& q)
/* performs a GF2 - multiplication of values with q, returns the result

From https://www.moria.us/articles/demystifying-the-lfsr/:
Multiply two polynomials, mod 2.
uint32_t mod2_multiply(uint32_t p, uint32_t q) {
  uint32_t result = 0;
  for (int i = 0; i < 32; i++) {
    if (((q >> i) & 1) != 0) {
      result ^= p << i;
    }
  }
  return result;
}
*/
{
    int i;
    longnum result;

    // clear all bits in result:
    result.fill(0);

    for (i = 0; i < BITS_IN_LONGNUM; i++)
    // iterate over all the bits:
        if (q.get_bit(i))
            result ^= *this << i;

    *this = result;
    return *this;
}

longnum longnum::operator + (const longnum& q) const
// GF2 addition is a simple ^
{
    return *this ^ q;
}

longnum& longnum::operator += (const longnum& q)
// += operator overload, perform a GF2 addition
{
    *this ^= q;
    return *this;
}

longnum longnum::operator - (const longnum& q) const
// GF2 subtraction is a simple ^
{
    return *this ^ q;
}

longnum& longnum::operator -= (const longnum& q)
// -= operator overload, perform a GF2 subtraction
{
    *this ^= q;
    return *this;
}

longnum& longnum::GF2_division(const longnum& denominator, longnum& quotient, longnum& remainder)
// performs a GF2-division of this / denominator, fills quotient and remainder, returns the quotient (=self)
// returns without doing anything if divider q==0

/*
// From https://www.moria.us/articles/demystifying-the-lfsr/:
// Divide the polynomial p by q, modulo 2.
struct division_result mod2_divide(uint32_t p, uint32_t q) {
  assert(q != 0);
  uint32_t quotient = 0;
  uint32_t remainder = p;
  int remainder_width = highest_bit(p);
  int q_width = highest_bit(q);
  while (remainder_width >= q_width) {
    int shift = remainder_width - q_width;
    remainder ^= q << shift;
    quotient |= 1 << shift;
    remainder_width = highest_bit(remainder);
  }
  return (struct division_result){
      .quotient = quotient,
      .remainder = remainder,
  };
}*/
{
    int q_order = denominator.get_order();

    // check that the divisor != 0, do nothing if it is
    if (q_order == 0)
        return *this;

    // clear all bits in quotient:
    quotient.fill(0);

    // copy contents of this into remainder
    remainder = *this;

    int remainder_order = this->get_order();
    int shift = 0;
    //longnum q_calc;

    while (remainder_order >= q_order)
    {
        shift = remainder_order - q_order;
        remainder ^= denominator << shift;
        quotient.set_bit(shift, 1);
        remainder_order = remainder.get_order();
    }

    return *this;
}

/*
void longnum::reverse (t_longnum longnum)
// reverses the bit order of longnum. Currently unused and therefore commented out.
// todo: "order" is not the right size, because if MSB=0 it will be skipped
{
    int i, order = long_get_order (longnum);
    t_longnum tmp;

    long_copy (tmp, longnum);
    long_fill (longnum, 0);

    for (i=order-1; i>=0; i--)  //BITS_IN_WORD*WORDS_IN_LONGNUM-1
        if (long_get_bit(tmp, i))
            long_setbit (longnum, order-i-1, 1);
}
*/
/*
int longnum::find_bit_pattern (t_longnum longnum, unsigned int findme, int n)
// finds bit pattern in findme (n bits) in longnum, prints out the bit numbers of the positions, returns -1 if not found, 1 if found
// Currently unused and therefore commented out.
{
    int i, result=-1;
    t_word tmp;

    for (i=0; i<WORDS_IN_LONGNUM*BITS_IN_WORD-n; i++)
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