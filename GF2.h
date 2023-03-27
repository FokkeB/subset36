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

/**
 * GF2 - Functions to implement a Galois Field of order 2
 * note: addition and subtraction are simple xor-operations, see function "long_xor" in longnum.c
 *
 * See: https://www.moria.us/articles/demystifying-the-lfsr/
*/

#ifndef GF2_H
#define GF2_H

#include "longnum.h"

void GF2_multiply (t_longnum p, t_longnum q, t_longnum result);
void GF2_division (t_longnum p, t_longnum q, t_longnum quotient, t_longnum remainder);

#endif