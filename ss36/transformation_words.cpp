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

int find11(int val11)
// returns the index of val11 in the transformation words (returning its index, which is a val10).
// returns NO_TW (-1) if it does not exist.
// uses a lookup table that was generated from the transformation words, to save some clock ticks.
{
    return transformation_words_inverted[val11];
}
