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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"
#include "colors.h"
#include "longnum.h"
#include "parse_input.h"
#include "useful_functions.h"
#include "ss36.h"

unsigned int hex_to_bin (char *hexstr, unsigned char *binstr)
// converts the contents of a string (ending with \0) with hex-data to the binary values
// returns the amount of bytes in binstr
{
    unsigned int w=0;

    for (int i=0; i<strlen(hexstr); i++)
    {
        if (i%2==0) 
            w = i/2;
        else
            binstr[w] <<= 4;

        if (hexstr[i] <= '9')
            binstr[w] |= (hexstr[i] - '0');
        else if (hexstr[i] >= 'a' && hexstr[i] <='f') 
            binstr[w] |= hexstr[i] - 'a' + 10;
        else if (hexstr[i] >= 'A' && hexstr[i] <='F') 
            binstr[w] |= hexstr[i] - 'A' + 10;
    }

    return w+1;
}

void align_telegram(t_telegram* telegram, enum t_align new_alignment)
/** shifts the telegram contents n bits to the left to prepare for hex/base64 - encoding
* n depends on the telegram size:
*
*/
{
    if (telegram->alignment == new_alignment)
    // already aligned correctly
        return;

    if (new_alignment == a_enc)
    // go from a_calc to a_enc by SHL'ing the bits to a byte border:
    {
        long_shiftleft(telegram->contents, 8-telegram->size % 8);             // 1023 or 341 => 1 or 3
        long_shiftleft(telegram->deshaped_contents, 8-telegram->number_of_userbits % 8);  // 830 or 210 => 2 or 6
    }
    else
    // go from a_enc to a_calc by SHR'ing the bits so that bit#0 is in location#0
    {
        long_shiftright(telegram->contents, 8 - telegram->size % 8);                // 1023 or 341 => 1 or 3
        long_shiftright(telegram->deshaped_contents, 8 - telegram->number_of_userbits % 8);     // 830 or 210 => 2 or 6
    }
    
    // store the new alignment:
    telegram->alignment = new_alignment;
}

int parse_input (char* input, t_telegram* telegram)
// parses the input line into the telegram.
// input has to be clean (i.e. no \r\t\n or spaces) and zero-terminated.
// returns the amount of errors that occurred.
// for each variant:
//  - parse the char string into a byte string (so from base64/hex -> binary)
//  - convert the byte string to a longnum 
//  - sets the values of telegram
{
    uint8_t arr[MAX_ARRAY_SIZE] = { 0 };      // temporary array to hold the byte array
    int size=0;                       // bytes in the temp array
    int bitlength=0;                  // #bits in the new telegram (L/S)
    t_longnum* p_ln;                  // pointer to the longnum in telegram that should be modified

    // first find out what kind of input line we have by switching between the length:
    switch (strlen (input))
    {
        case N_CHARS_SHAPED_LONG_HEX :
            bitlength = BITLENGTH_LONG_TELEGRAM;
            p_ln = &telegram->contents;
            telegram->number_of_shapeddata_bits = N_SHAPEDDATA_L;
            size = hex_to_bin(input, arr);
            break;

        case N_CHARS_SHAPED_SHORT_HEX :
            bitlength = BITLENGTH_SHORT_TELEGRAM;
            p_ln = &telegram->contents;
            telegram->number_of_shapeddata_bits = N_SHAPEDDATA_S;
            size = hex_to_bin(input, arr);
            break;

        case N_CHARS_SHAPED_LONG_BASE64 :
            bitlength = BITLENGTH_LONG_TELEGRAM;
            p_ln = &telegram->contents;
            telegram->number_of_shapeddata_bits = N_SHAPEDDATA_L;
            size = b64_decode(input, strlen(input), arr);
            break;

        case N_CHARS_SHAPED_SHORT_BASE64 :
            bitlength = BITLENGTH_SHORT_TELEGRAM;
            p_ln = &telegram->contents;
            telegram->number_of_shapeddata_bits = N_SHAPEDDATA_S;
            size = b64_decode(input, strlen(input), arr);
            break;

        case N_CHARS_UNSHAPED_LONG_HEX :
            bitlength = BITLENGTH_LONG_TELEGRAM;
            p_ln = &telegram->deshaped_contents;
            telegram->number_of_userbits = USERBITS_IN_TELEGRAM_L;
            size = hex_to_bin(input, arr);
            break;

        case N_CHARS_UNSHAPED_SHORT_HEX:
            bitlength = BITLENGTH_SHORT_TELEGRAM;
            p_ln = &telegram->deshaped_contents;
            telegram->number_of_userbits = USERBITS_IN_TELEGRAM_S;
            size = hex_to_bin(input, arr);
            break;

        case N_CHARS_UNSHAPED_LONG_BASE64 :
            bitlength = BITLENGTH_LONG_TELEGRAM;
            p_ln = &telegram->deshaped_contents;
            telegram->number_of_userbits = USERBITS_IN_TELEGRAM_L;
            size = b64_decode(input, strlen(input), arr);
            break;

        case N_CHARS_UNSHAPED_SHORT_BASE64 :
            bitlength = BITLENGTH_SHORT_TELEGRAM;
            p_ln = &telegram->deshaped_contents;
            telegram->number_of_userbits = USERBITS_IN_TELEGRAM_S;
            size = b64_decode(input, strlen(input), arr);
            break;

        default:
            eprintf(VERB_QUIET, ERROR_COLOR"\nError parsing string"ANSI_COLOR_RESET" \"%s\" of %zd chars, skipping to the next.\n", input, strlen(input));
            return 1; // format not recognised, return error
    }

    // perform the parsing based on the settings determined above:
    init_telegram(telegram, bitlength);
    array_to_longnum(arr, *p_ln, size);

    // set the alignment of the telegram to encoding:
    telegram->alignment = a_enc;

    return 0;
}

t_telegram* parse_input_line(char* line)
// parses the input line, creates and fills the telegram
// returns a pointer to a telegram if all went well, or NULL if not
{
    char* p=NULL;
    t_telegram* telegram;

    // first remove any comments (starting with '#') from the line
    p = strchr(line, '#');
    if (p != NULL)
        *p = '\0'; // end the line at the start of the comments
    else
        p = &(line[strlen(line)]);

    // then clear trailing spaces, CR's, LF's, tabs
    p--;
    while ((*p == '\n') || (*p == '\r') || (*p == ' ') || (*p == '\t') )
    {
        *p = '\0';
        p--;
    }

    // check if anything is left; if not, skip to the next line
    if (line[0] == '\0')
        return NULL;

    create_new_telegram(&telegram, line);

    // see if there is a comma. If so: read in both values
    p = strchr(line, ',');
    if (p != NULL)
    {
        *p = '\0';     // terminate the string at the comma

        if (parse_input(line, telegram))  // parse the first part of the string
        // destroy the telegram if a parsing error occured and continue to the next:
        {
            free(telegram);
            return NULL;
        }

        p++; // increase p to point to the location next to where the ',' used to be
    }
    else
        p = line;

    if (parse_input(p, telegram))  // parse the second part of the string
    {
        free(telegram);
        return NULL;
    }

    if (telegram->number_of_userbits)
    // the unshaped data is set, print it
    {
        eprintf(VERB_ALL, "\nDecoded %d bits:\t", telegram->number_of_userbits);
        print_longnum_fancy(VERB_ALL, telegram->deshaped_contents, 16, telegram->number_of_userbits, &no_colors);
    }
    if (telegram->number_of_shapeddata_bits)
    // shaped data is set, print it
    {
        eprintf(VERB_ALL, "\nCoded %d bits:\t\t", telegram->size);
        print_longnum_fancy(VERB_ALL, telegram->contents, 16, telegram->size, &no_colors);
    }

    return telegram;
}

t_telegram* read_from_file_into_list (char *filename, int *telegramcount)
/** Reads the data from the file into the linked list "records".
 * Returns a pointer to the first record in the list.
 * 
 * Each line contains either an encoded or decoded telegram.
 * Telegrams can be either encoded in HEX or in BASE64.
 * Comments are preceded by a '#'.
 * This function will distinguish the lines based on their sizes.
 * 
 */ 
{
    FILE *fp;
    char* p;
    unsigned char line[MAX_ARRAY_SIZE] = { 0 };   // max line length is 382
    int linecount=0;
    t_telegram *previous_telegram = NULL, *new_telegram = NULL, *first_telegram = NULL;

    // open the indicated file:
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        eprintf(VERB_QUIET, ERROR_COLOR"Error"ANSI_COLOR_RESET" reading input file. Errcode=%s.\n", strerror(errno));
        exit(ERR_NO_INPUT);
    }
    else
        eprintf(VERB_GLOB, "Reading input from file: %s\n", filename);

    *telegramcount = 0;

    // read all lines from the file
    while ( ( (p=fgets(line, MAX_ARRAY_SIZE, fp)) != NULL) && ( (*telegramcount < MAX_RECORDS) || ((MAX_RECORDS == 0)) ) )
    // parse the line read from the file:
    {
        linecount++;

        eprintf(VERB_GLOB, "\nRead in line #%d:\t\"%s\"", linecount, line);
        
        if ((new_telegram = parse_input_line(line)) == NULL)
            continue;

        // check if this is the first telegram, initialise some pointers if this is the case
        if (first_telegram == NULL)
        {
            first_telegram = new_telegram;      // remember the start of the list
            previous_telegram = new_telegram;   // to prevent an error a few lines further
        }

        // and link the previous record to the new record:
        previous_telegram->next = new_telegram;
        previous_telegram = new_telegram;

        (*telegramcount)++;
    }

    fclose(fp);

    if (*telegramcount == 1)
        first_telegram->next = NULL;

    return first_telegram;
}
