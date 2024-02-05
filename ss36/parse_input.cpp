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
// returns a pointer to a telegram if all went well, or NULL if not
// todo: make string out of line?
{
    char* p=NULL, line[MAX_ARRAY_SIZE];
    telegram* p_telegram;

    strcpy_s(line, line_orig);

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

    // see if there is a comma or semicolon. If so: read in both values
    p = strchr(line, ',');
    if (p == NULL)
        p = strchr(line, ';');

    if (p != NULL)
    // comma or semicolon found, p points at it
    {
        *p = '\0';     // terminate the string at the comma

        if (p_telegram->parse_input((string)line) != ERR_NO_ERR)  // parse the first part of the string
        // destroy the telegram if a parsing error occured and continue to the next:
        {
            delete(p_telegram);
            return NULL;
        }

        p++; // increase p to point to the location next to where the ',' or ';' used to be
    }
    else
        p = line;

    if (p_telegram->parse_input((string)p))  // parse the second part of the string
    {
        delete(p_telegram);
        return NULL;
    }

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

    // finally, store the original input string:
    p_telegram->input_string = line_orig;

    return p_telegram;
}

t_telegramlist parse_content_string(const string& contents_orig)
// parses the balise information in contents into a t_telegramlist
{
    size_t start = 0, found;

    string line;
    int linecount = 0;    
    telegram* p_new_telegram;
    t_telegramlist telegramlist;
    string contents = contents_orig + "\n";

    for (found = contents.find(LINE_DELIM); found != string::npos; found = contents.find(LINE_DELIM, start))
    // iterate over the lines (separated by LINE_DELIM in contents
    {
        line = contents.substr(start, found - start).c_str();
        linecount++;
        start = found + 1;
        eprintf(VERB_GLOB, "\nRead in line #%d:\t\"%s\"", linecount, line.c_str());
        if ((p_new_telegram = parse_input_line(line.c_str())) == NULL)
            // illegal contents, skip this line
        {
            eprintf(VERB_GLOB, "-> SKIPPED\n");
            continue;
        }
        else
        {
            // add line to list of telegrams:
            telegramlist.push_back(p_new_telegram);
            eprintf(VERB_GLOB, "-> parsed OK\n");
        }
    }

    return telegramlist;
}

t_telegramlist read_from_file_into_list (const string filename)
/** Reads the data from the file into the linked list "records".
 * Returns a pointer to the first record in the list.
 * 
 * Each line contains either an encoded or decoded telegram.
 * Telegrams can be either encoded in HEX or in BASE64.
 * Comments are preceded by a '#'.
 * This function distinguishes the lines based on the sizes of the data.
 */ 
{
    FILE *fp;
    char* p;
    char line[MAX_ARRAY_SIZE] = { 0 };   // max line length is 382
    int linecount=0;
    telegram* p_new_telegram;
    t_telegramlist telegramlist; // = new t_telegramlist();

    // open the indicated file:
    if (fopen_s(&fp, filename.c_str(), "r"))
    {
        const size_t errmsglen = 100;// strerrorlen_s(errno) + 1;
        char errmsg[errmsglen];
        strerror_s(errmsg, errmsglen, errno);
        eprintf(VERB_QUIET, ERROR_COLOR "Error" ANSI_COLOR_RESET " reading input file ('%s'). Errcode=%s.\n", filename.c_str(), errmsg);
        exit(ERR_NO_INPUT);
    }
    else
        eprintf(VERB_GLOB, "Reading input from file: %s\n", filename.c_str());

    while ( ( (p=fgets(line, MAX_ARRAY_SIZE, fp)) != NULL) ) // && ( (*telegramcount < MAX_RECORDS) || ((MAX_RECORDS == 0)) ) )
    // read all lines from the file and parse them
    {
        linecount++;

        eprintf(VERB_GLOB, "\nRead in line #%d:\t\"%s\"", linecount, line);
        
        if ((p_new_telegram = parse_input_line(line)) == NULL)
        // illegal contents, skip this line
            continue;

        // add line to list of telegrams:
        telegramlist.push_back(p_new_telegram);
    }

    fclose(fp);

    return telegramlist;
}
