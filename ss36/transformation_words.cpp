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

t_word transform_word10_to_word11(unsigned int val10)
// converts the 10-bit input to the 11-bit output
// performs a range check, stops the program if val10 is out of range
{
    if (val10 > N_TRANS_WORDS-1)
    // overflow, stop the program
    {
        printf("Overflow error in transform_word10_to_word11 (val10=%d), quitting.\n", val10);
        exit(ERR_LOGICAL_ERROR);
    }

    return (t_word)(transformation_words[val10]);
}

t_word transform_word11_to_word10(unsigned int val11)
// returns the index of val11 in the transformation words (returning its index, which is a val10).
// returns NO_TW (-1) if it does not exist.
// uses a lookup table that was generated from the transformation words, to save some clock ticks.
// performs a range check, stops the program if val10 is out of range
{
    if (val11 > N_TW_INVERTED - 1)
    // overflow, stop the program
    {
        printf("Overflow error in transform11to10 (val11=%d), quitting.\n", val11);
        exit(ERR_LOGICAL_ERROR);
    }

    return (t_word)transformation_words_inverted[val11];
}
