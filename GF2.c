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

#include "GF2.h"
#include "longnum.h"

void GF2_multiply (t_longnum p, t_longnum q, t_longnum result)
// performs a GF2-multiplication of p and q, fills result with the result

/* 
// From https://www.moria.us/articles/demystifying-the-lfsr/:
// Multiply two polynomials, mod 2.
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
    t_longnum ln_tmp; 

    // clear all bits in longnumresult:
    long_fill (result, 0);

    for (i=0; i<BITS_IN_LONGNUM; i++)
    // iterate over all the bits:
    {
        if (long_get_bit(q, i)) 
        {
            long_copy (ln_tmp, p);
            long_shiftleft (ln_tmp, i);
            long_xor(result, ln_tmp, result);
        }    
    }
}

void GF2_division (t_longnum p, t_longnum q, t_longnum quotient, t_longnum remainder)
// performs a GF2-division of p / q, fills quotient and remainder
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
    int q_order = long_get_order (q);
    
    // check that the divisor != 0, do nothing if it is
    if (q_order == 0)
        return;

    // clear all bits in quotient:
    long_fill (quotient, 0);

    // copy contents of p into remainder
    long_copy (remainder, p);
        
    int remainder_order = long_get_order (p);
    int shift = 0;
    t_longnum q_calc; 

    while (remainder_order >= q_order)
    {
        shift = remainder_order - q_order;

        long_copy (q_calc, q);
        long_shiftleft (q_calc, shift);
        long_xor(remainder, q_calc, remainder); 
        long_setbit (quotient, shift, 1);

        remainder_order = long_get_order (remainder);
    }
}