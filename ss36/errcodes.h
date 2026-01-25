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

// error codes that are used in many places in this software

#ifndef ERRCODES_H
#define ERRCODES_H

#define ERR_NO_ERR              0       // no error, all OK
#define ERR_NO_INPUT            1       // no input specified
#define ERR_LOGICAL_ERROR       2       // a logical error has occurred
#define ERR_OUTPUT_FILE         3       // error creating output file
#define ERR_MEM_ALLOC           4       // error allocating memory
#define ERR_INPUT_ERROR         5       // error in the input
//#define ERR_THREAD_CREATION     6       // error creating calculation thread or acquiring mutex

// error codes from the subset 36 (including references to relevant paragraphs in the subset):
#define ERR_ALPHABET            10      // 4.3.2.5.2 Alphabet Condition
#define ERR_OFF_SYNCH_PARSING   11      // 4.3.2.5.3 Off-Synch-Parsing Condition
#define ERR_APERIODICITY        12      // 4.3.2.5.4 Aperiodicity Condition for Long Format
#define ERR_UNDER_SAMPLING      13      // 4.3.2.5.5 Under-sampling Condition
#define ERR_CONTROL_BITS        14      // 4.3.1.2 Telegram Format -> control bits 
#define ERR_CHECK_BITS          15      // 4.3.2.4 Computing the Check Bits -> check bits do not match the user data
#define ERR_SB_ESB_OVERFLOW     16      // 4.3.2.4 Computing the Check Bits -> overflow of SB and ESB (no values found)
#define ERR_11_10_BIT           17      // 4.3.2.3 The 10-to-11-Bit Transformation -> error during conversion
#define ERR_CONTENT             18      // General: shaped content does not match unshaped content

#endif