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

#include "parse_input.h"

telegram* parse_input_line(const char* line_orig)
// parses the input line, creates and fills the telegram
// returns a pointer to a telegram if all went well, NULL if the line is empty (or only contains comments),
// ERR_INPUT_ERROR if the contents of the line could not be parsed
{
    char* p=NULL, line[MAX_ARRAY_SIZE];
    telegram* p_telegram;

    //strcpy_s(line, line_orig);
    strncpy(line, line_orig, sizeof(line));
    line[sizeof(line) - 1] = '\0';

    // first remove any comments (starting with '#') from the line
    p = strchr(line, '#');
    if (p != NULL)
        *p = '\0'; // end the line at the start of the comments
    else
        p = &(line[strlen(line)]);  // point p to end of string

    // then clear trailing spaces, CR's, LF's, tabs, separators
    p--;
    while ((*p == '\n') || (*p == '\r') || (*p == ' ') || (*p == '\t') || (*p == ';') || (*p == ','))
    {
        *p = '\0';
        p--;
    }

    // check if anything is left; if not, skip to the next line
    if (line[0] == '\0')
        return NULL;

    // create new telegram, fill with dummy values
    p_telegram = new telegram("", s_long);
//    p_telegram = new telegram((string)line_orig, s_long);  // does not work as the creator will parse the string
    p_telegram->errcode = ERR_NO_ERR;
    p_telegram->input_string = line; // _orig;  // do this manually to prevent the creator from parsing the input string

    // see if there is a comma or semicolon. If so: read in both values
    p = strchr(line, ',');
    if (p == NULL)
        p = strchr(line, ';');

    if (p != NULL)
    // comma or semicolon found, p points at it
    {
        *p = '\0';     // terminate the string at the comma
        p_telegram->parse_input((string)line);  // parse the first part of the string
        if (p_telegram->errcode != ERR_NO_ERR)
        // something went wrong with the parsing, skip parsing the rest of the line
            return p_telegram;
        p++; // increase p to point to the location next to where the ',' or ';' used to be
    }
    else
        p = line;

    p_telegram->parse_input((string)p);  // parse the second part of the string
    if (p_telegram->errcode != ERR_NO_ERR)
    // something went wrong with the parsing, skip the rest
        return p_telegram;

    if (p_telegram->number_of_userbits)
    // the unshaped data is set, print it
    {
        eprintf(VERB_ALL, "\nDecoded %d bits:\n", p_telegram->number_of_userbits);
        p_telegram->deshaped_contents.print_fancy(VERB_ALL, 16, p_telegram->number_of_userbits+6, NULL);
    }
    if (p_telegram->number_of_shapeddata_bits)
    // shaped data is set, print it
    {
        eprintf(VERB_ALL, "\nCoded %d bits:\n", p_telegram->size);
        p_telegram->contents.print_fancy(VERB_ALL, 16, p_telegram->size, NULL);
    }

    return p_telegram;
}

telegram* parse_content_string(const string& contents_orig)
// parses the balise information in contents into a linked list of telegrams
{
    size_t start = 0, found;

    string line;
    int linecount = 0;    
    telegram *p_new_telegram = NULL, *p_start_of_list = NULL, *p_previous_telegram = NULL;
    string contents = contents_orig + "\n";

    for (found = contents.find(LINE_DELIM); found != string::npos; found = contents.find(LINE_DELIM, start))
    // iterate over the lines (separated by LINE_DELIM in contents)
    {
        line = contents.substr(start, found - start).c_str();
        linecount++;
        start = found + 1;
        eprintf(VERB_GLOB, "\nRead in line #%d:\t\"%s\"", linecount, line.c_str());
        p_new_telegram = parse_input_line(line.c_str());
        if (!p_new_telegram)
            eprintf(VERB_GLOB, " -> Skipped line\n");
        else
        {
            if (p_start_of_list == NULL)
            // this is the first telegram, point p_start_of_list to it
                p_start_of_list = p_new_telegram;
            else
            // not the first, point next in the previous telegram to the new one
                p_previous_telegram->next = p_new_telegram;

            // remember the previous telegram for the next line
            p_previous_telegram = p_new_telegram;
            
            //telegramlist.push_back(p_new_telegram);

            if (p_new_telegram->errcode == ERR_NO_ERR)
                eprintf(VERB_GLOB, " -> parsed OK\n");
            else
                eprintf(VERB_GLOB, " -> parse error\n");
        }
    }

    return p_start_of_list;
}
