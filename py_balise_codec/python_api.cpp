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

#include "python_api.h"

int verbose = VERB_QUIET;


extern "C" EXPORT   // export this function in the .dll / .so file    
const char* get_lib_version(void)
// returns the version of the library
{
    return LIB_VERSION;
}


extern "C" EXPORT   // export this function in the .dll / .so file    
int parse_line(const char* input_line,
               const unsigned int max_cpu,
               const bool calc_all,
               const py_telegram_t** py_telegram,
               const char unshaped_format,
               const char shaped_format)
{
    telegram *p_telegram = nullptr, *temp_telegram = nullptr;
    py_telegram_t *prev_py_telegram = nullptr, *temp_py_telegram = nullptr;
    string line;
//    int i = 0;

//    printf("c++:\n");
//    printf("Input line = '%s'\n", input_line);
     
    // Basic argument validation
    if (!input_line)
        return ERR_NO_INPUT;   // invalid parameters

    try {
        // Parse the input line into a linked list of telegram objects
        p_telegram = parse_content_string((string)input_line);
        
        if (!p_telegram) 
        // empty line or comment-only line
            return ERR_NO_INPUT;
        
        // Process / encode telegram(s)
        convert_telegrams_multithreaded(p_telegram, max_cpu, calc_all);

        int i=1;
        temp_telegram = p_telegram;
        while (temp_telegram)
        {
            string line2 = temp_telegram->get_csv_output_line("base64", false, calc_all, ';');
            printf ("Telegram#%03d:%s\n", i, line2.c_str());
            temp_telegram = temp_telegram->next;
            i++;
        }
        printf("Counted %d telegram(s).\n", i);

//        i = 0;

        // Create the linked list of python telegrams and free the c++ linked list of telegrams
        while (p_telegram) 
        {
            p_telegram->align(a_enc);
            // create a new py_telegram:
            temp_py_telegram = (py_telegram_t*)malloc(sizeof(py_telegram_t));
            if (!temp_py_telegram)
                return ERR_MEM_ALLOC;
 
            // fill the shaped and unshaped data based on the error that occurred:
            if (p_telegram->errcode == ERR_INPUT_ERROR)
            {
                temp_py_telegram->unshaped = (char*)malloc(p_telegram->input_string.size()+1);
                if (!temp_py_telegram->unshaped)
                    return ERR_MEM_ALLOC;

                memcpy(temp_py_telegram->unshaped, p_telegram->input_string.c_str(), p_telegram->input_string.size() + 1);

                line = string("<input err>");
                temp_py_telegram->shaped = (char*)malloc(line.size()+1);
                if (!temp_py_telegram->shaped)
                    return ERR_MEM_ALLOC;
                memcpy(temp_py_telegram->shaped, line.c_str(), line.size() + 1);
            }
            else  // no error in the input line
            {
//                temp_py_telegram->unshaped = (char*)malloc(p_telegram->number_of_userbits / 4 + 1);
//                if (!temp_py_telegram->unshaped)
//                    return ERR_MEM_ALLOC;

                // write the unshaped data in the desired output format:
                if (unshaped_format == 'b')
                    p_telegram->deshaped_contents.sprint_base64(line, p_telegram->number_of_userbits);
                else
                    p_telegram->deshaped_contents.sprint_hex(line, p_telegram->number_of_userbits);
                temp_py_telegram->unshaped = (char*)malloc(line.size()+1);
                if (!temp_py_telegram->unshaped)
                    return ERR_MEM_ALLOC;

                memcpy(temp_py_telegram->unshaped, line.c_str(), line.size() + 1);
                 
                // write the shaped data in the desired output format:
//               temp_py_telegram->shaped = (char*)malloc(p_telegram->size / 4 + 1);
//                if (!temp_py_telegram->shaped)
//                    return ERR_MEM_ALLOC;

                if (shaped_format == 'h')
                    p_telegram->contents.sprint_hex(line, p_telegram->size); 
                else
                    p_telegram->contents.sprint_base64(line, p_telegram->size);
                temp_py_telegram->shaped = (char*)malloc(line.size() + 1);
                if (!temp_py_telegram->shaped)
                    return ERR_MEM_ALLOC;
                memcpy(temp_py_telegram->shaped, line.c_str(), line.size() + 1);

                //exit(0);
            }

            // also copy the error code:
            temp_py_telegram->errcode = p_telegram->errcode; 
            

            if (prev_py_telegram)
            // there is a previous py_telegram, point it to the new telegram
                prev_py_telegram->next = temp_py_telegram;
            else
            // no previous py_telegram (this is the first), store it in py_telegram
                *py_telegram = temp_py_telegram;

            // point previous telegram - pointer to current telegram
            prev_py_telegram = temp_py_telegram;

            // delete the p_telegram and go to the next (if any)
            temp_telegram = p_telegram->next;   
            delete p_telegram;
            p_telegram = temp_telegram;
//            i++;
        } 

        // set the last ->next to point to null to signal end of list:
        temp_py_telegram->next = nullptr;
/*
        temp_py_telegram = *py_telegram;
        i = 1;
        while (temp_py_telegram)
        {
            printf("PYTelegram#%d:Unshaped=%s\nShaped=%s\n", i++, temp_py_telegram->unshaped, temp_py_telegram->shaped);
            temp_py_telegram = temp_py_telegram->next;
        }
*/
        return 0;   // success
    }
    catch (...) {
        return ERR_LOGICAL_ERROR; // unexpected exception
    }
}