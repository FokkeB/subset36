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

#ifndef COLORS_H
#define COLORS_H

#include "ansi_escapes.h"

// define some colors for pretty output:
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_GREY    "\x1b[90m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define HEADER_COLOR      ANSI_COLOR_MAGENTA
#define FIELD_COLOR       ANSI_COLOR_BLUE
#define ERROR_COLOR       ANSI_COLOR_RED
#define OK_COLOR          ANSI_COLOR_GREEN
#define BITNUM_COLOR      ANSI_COLOR_GREY

#define MAX_COLOR_LENGTH    10
#define MAX_N_LAYOUT        10

// a structure to define the color markings of longnums (to be used in an array)
// note that the last struct in this array must have .length set to 0 to stop an overflow.
typedef struct 
{
    int start, length;              // startbit and length of color
    char color[MAX_COLOR_LENGTH];   // color to use (ANSI definition)
} t_longnum_layout;

#endif