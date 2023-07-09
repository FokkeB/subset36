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

#ifndef PARSE_INPUT_H
#define PARSE_INPUT_H

#include <stdint.h>
#include "ss36.h"

// expected input sizes of shaped telegrams: 
#define N_CHARS_SHAPED_LONG_HEX          256    // 2 char/byte, 1024 bits/8=128 bytes=256 chars. So: SHR 1 to loose 1 bit to get to 1023 bits in complete telegram.
#define N_CHARS_SHAPED_SHORT_HEX         86     // 2 char/byte, 344 bits/8=43 bytes=86 chars. So: SHR 3 to loose 3 bits to get 341 bits in complete telegram.
#define N_CHARS_SHAPED_LONG_BASE64       172    // 4 chars/3 bytes, 1032 bits/8=129 bytes=172 chars. Leaves 9 bits, of which 8 are captured in trailing "=" So SHR 1 -> 1023 bits in complete telegram.
#define N_CHARS_SHAPED_SHORT_BASE64      60     // 4 chars/3 bytes, 360 bits/8=45 bytes=60 chars. Leaves 19 bits, of which 16 are captured in trailing "==". So SHR 3 -> 341 bits in complete telegram.

// expected input sizes of unshaped telegrams:
#define N_CHARS_UNSHAPED_LONG_HEX        208    // 2 char/byte, 832 bits/8=104 bytes=208 chars. So: SHR 2 -> 830 user bits.
#define N_CHARS_UNSHAPED_SHORT_HEX       54     // 2 char/byte, 216 bits/8=27 bytes=54 chars. So: SHR 6 -> 210 user bits.
#define N_CHARS_UNSHAPED_LONG_BASE64     140    // 4 chars/3 bytes, 840 bits/8=105 bytes=140 chars. Leaves 10 bits, of which 8 are captured in trailing "=". So SHR 2 -> 830 user bits.
#define N_CHARS_UNSHAPED_SHORT_BASE64    36     // 4 chars/3 bytes, 216 bits/8=27 bytes=36 chars. SHR 6 -> 210 user bits.

#define MAX_RECORDS             0				// max# records to be read in from external file (if set to 0, until memory runs out)
#define MAX_ARRAY_SIZE          500				// max length of byte string read from a file (=one line)

// prototypes:
void align_telegram(t_telegram* telegram, enum t_align new_alignment);
t_telegram* read_from_file_into_list(char* filename, int* telegramcount);
t_telegram* parse_input_line(char* line);

#endif