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

#include "balise_codec.h"

string read_from_file(string filename)
// reads the balise data from filename and returns the information in one long string
{
    string s;

    // read the file into a string:
    ifstream f(filename);
    if (f.is_open())
    {
        getline(f, s, '\0');
        f.close();
        return s;
    }
    else
    {
        const size_t errmsglen = 100;
        char errmsg[errmsglen];
        strerror_s(errmsg, errmsglen, errno);

        eprintf(VERB_QUIET, ERROR_COLOR "Error" ANSI_COLOR_RESET " reading input file ('%s'): %s. \n", filename.c_str(), errmsg);

        exit(ERR_NO_INPUT);
    }
}

int convert_telegram_optimised(telegram* p_telegram)  
// converts the records from shaped to deshaped and vice versa
// if both shaped and deshaped input data is given in the same record, checks the correctness of the shaped telegram
// returns a meaningless 0
{
    longnum deshaped_data;

    // skip this telegram if there is an error in its input
    if (p_telegram->errcode != ERR_NO_ERR)
        return 0; 

    // determine what we're dealing with (shaped / unshaped / both):
    switch (p_telegram->action)
    {
        case act_check:
        // both shaped and unshaped; verify that the shaped telegram is correct:
        {
            // first print the input:
            eprintf(VERB_GLOB, "INPUT:");
            eprintf(VERB_GLOB, "\nUnshaped contents (%d bits):\n", p_telegram->number_of_userbits);
            p_telegram->deshaped_contents.print_fancy(VERB_GLOB, 8, p_telegram->number_of_userbits, NULL);
            eprintf(VERB_GLOB, "\nShaped contents (%d bits):\n", p_telegram->size);
            p_telegram->print_contents_fancy(VERB_GLOB);

            // check the shaped telegram:
            eprintf(VERB_GLOB, "Perform the condition-checks and content checks of the shaped telegram:\n");
            if (p_telegram->check_shaped_telegram() != ERR_NO_ERR)
                eprintf(VERB_GLOB, ERROR_COLOR "At least one error occured.\n" ANSI_COLOR_RESET);

            // check if the deshaping of the shaped contents leads to the given unshaped contents
            p_telegram->check_shaped_deshaped();
            break;
        }
        case act_deshape:
        // only shaped bytes; check the telegram and de-shape it
        {
            // show the inputs and outputs:
            eprintf(VERB_GLOB, "INPUT: Shaped telegram of %d bits:\n", p_telegram->size);
            p_telegram->print_contents_fancy(VERB_GLOB);

            // check the shaped telegram:
            if (p_telegram->check_shaped_telegram() != ERR_NO_ERR)
                eprintf(VERB_GLOB, ERROR_COLOR "Warning:" ANSI_COLOR_RESET" At least one error occured, continuing to deshape the telegram.\n");

            // deshape the telegram:
            p_telegram->deshape();

            // output the shaped bits:
            p_telegram->align(a_enc);  // shift the bits to the left to prepare for printing
            eprintf(VERB_GLOB, "OUTPUT: %d deshaped user bits:\n", p_telegram->number_of_userbits);
            p_telegram->deshaped_contents.print_hex(VERB_GLOB, p_telegram->number_of_userbits);
            eprintf(VERB_GLOB, "\n");
            break;
        }
        case act_shape:
        // only unshaped bytes; shape them
        {
            eprintf(VERB_GLOB, "INPUT: Unshaped user data of %d bits:\n", p_telegram->number_of_userbits);
            p_telegram->deshaped_contents.print_fancy(VERB_GLOB, 8, p_telegram->number_of_userbits, NULL);

            if (p_telegram->force_long)
                // make this a long telegram before shaping it
                p_telegram->make_userdata_long();

            // shape the telegram:
            p_telegram->shape_opt();

            if (p_telegram->errcode != ERR_SB_ESB_OVERFLOW)
            // there was no overflow of SB+ESB, check the telegram:
            {
                // show check result and the telegram:
                eprintf(VERB_GLOB, "Created shaped telegram, performing checks:\n");

                p_telegram->errcode = ERR_NO_ERR;  // reset the error code (still set from shaping)
                p_telegram->check_shaped_deshaped();  // no errors should occur, this is checked in the next line

                if (p_telegram->errcode != ERR_NO_ERR)
                    eprintf(VERB_QUIET, ERROR_COLOR "ERROR:" ANSI_COLOR_RESET" Created telegram that does not pass the checks. Err=%d\n", p_telegram->errcode);

                // show the output:
                eprintf(VERB_GLOB, "OUTPUT: Shaped telegram: \n");
                p_telegram->print_contents_fancy(VERB_GLOB);
                eprintf(VERB_GLOB, "(hex:) ");
                p_telegram->align(a_enc);  // shift the bits to the left to prepare for printing
                p_telegram->contents.print_hex(VERB_GLOB, p_telegram->size);
                eprintf(VERB_GLOB, "\n");
            }
            else
                eprintf(VERB_ALL, "Skipped checks of overflowed telegram\n");
            
            break;
        }
        default:
        {
            // this should never happen
            eprintf(VERB_QUIET, "A logical error occurred");
            exit(ERR_LOGICAL_ERROR);
        }
    }

    return 0;
}

DWORD WINAPI convert_shortlist(t_shortlist_param *shortlist_param)
// converts the n telegrams in the shortlist
{
    int i;
    telegram *p_telegram = shortlist_param->p_telegram, *p_temp_telegram = NULL, *p_prev_telegram = NULL;
    DWORD dwWaitResult;

    eprintf(VERB_FLOW, "\nStarting shortlist with param address = %p.\n", shortlist_param);

    for (i = 0; i < shortlist_param->n; i++)
    // iterate over the telegrams in the shortlist
    {
        eprintf(VERB_GLOB, "Checking telegram %d in shortlist %p.\n", i, shortlist_param);

        convert_telegram_optimised(p_telegram);
        p_telegram->align(a_calc);
        //eprintf(VERB_GLOB, "i=%d; sb=%d; esb=%d\n", i, p_telegram->get_scrambling_bits(), p_telegram->get_extra_shaping_bits());

        if (p_telegram->errcode == ERR_NO_ERR)
        // calculation went ok, there was no overflow of ESB+SB or other error
        {
            if (shortlist_param->calc_all)
            // calculate all possible shapings and we are not yet at the last
            // -> make a copy of the current telegram and add it after the current telegram
            {
                p_temp_telegram = new telegram(p_telegram);  // new telegram with identical contents as p_telegram
                if (!p_temp_telegram)
                {
                    eprintf(VERB_QUIET, "Error allocating memory for a new telegram, quitting.\n");
                    exit(ERR_MEM_ALLOC);
                }
                
                if (!p_temp_telegram->set_next_esb_opt())   // increase the ESB
                // ESB overflowed, set the next SB and set word9 to -1 to trigger the rescrambling with the new SB
                {
                    p_temp_telegram->set_next_sb_esb_opt(); // small chance on overflow, this will be dealt with when the new telegram is calculated
                    p_temp_telegram->word9 = -1;
                }
                
                // add the new telegram to the end of the list, right after the current telegram:
                p_telegram->next = p_temp_telegram; // p_temp_telegram->next was already memcopied

                // increase the number of telegrams in this shortlist:
                shortlist_param->n++;
                eprintf(VERB_FLOW, "Added new telegram to the shortlist. Addr=%p, new shortlist size=%d.\n", p_temp_telegram, shortlist_param->n);
            }
        }
        else
        if (p_telegram->errcode == ERR_SB_ESB_OVERFLOW)
        // an error occured in the calculation: an overflow of SB+ESB (could also be ERR_INPUT_ERROR, but such a telegram will be skipped)
        {
            if (shortlist_param->calc_all)
            // there was an overflow while calculating all shapings (end reached). 
            // Delete the last added telegram, correctly terminate the list and return.
            {
                if (p_prev_telegram)
                {
                    p_prev_telegram->next = NULL;
                    delete p_telegram;
                    shortlist_param->n--;
                    eprintf(VERB_FLOW, "Finished 'calc_all' for this telegram.\n");
                    return 0;
                }
            }
            else
            {
                // SB+ESB overflowed while not calculating all variations.
                // This should only veeeeery rarely happen: 10^-100 (see subset 36, A1.1.1)
                eprintf(VERB_QUIET, ERROR_COLOR "\n\nERROR:" ANSI_COLOR_RESET " No valid combination of Scrambling Bits and Extra Shaping Bits found for the telegram below. \n");
                eprintf(VERB_QUIET, "Please make a minor change in the telegram contents and try again. See Subset-036.\n");
                eprintf(VERB_QUIET, "Also: please send a copy of the input telegram to the writer of this program (fokke@bronsema.net). Thanks :-)\n");
                eprintf(VERB_QUIET, "Contents of input telegram: \n");// , telegram->input_string);
                p_telegram->align(a_enc);  // shift the bits to the left to prepare for printing
                p_telegram->deshaped_contents.print_hex(VERB_QUIET, p_telegram->number_of_userbits);
                eprintf(VERB_QUIET, "\n\n");

                exit(ERR_SB_ESB_OVERFLOW);
            }
        }

        p_prev_telegram = p_telegram;
        p_telegram = p_telegram->next;

        if (shortlist_param->progressMutex)
        // mutex is set; acquire the mutex and update the progress counter
        {
            dwWaitResult = WaitForSingleObject(shortlist_param->progressMutex, INFINITE);  

            if (dwWaitResult == WAIT_OBJECT_0)
            // mutex obtained, increase the progress_counter
                (*shortlist_param->p_progress_counter)++;
            else
                printf("Unable to acquire mutex while increasing progress counter\n");

            ReleaseMutex(shortlist_param->progressMutex);
        }
    }

    return 0; 
}

int convert_telegrams_multithreaded(telegram *telegrams, unsigned int max_cpu, bool calc_all)
// Converts the telegrams in the linked list pointed to by *telegrams using max_cpu cores
// Splits the long list in max_cpu shortlists of about equal size
// Starts a thread for each shortlist and waits for each thread to finish
// If calc_all, calculate all possible shapes of each input telegram
// Shows progress during the calculation
{
    int telegram_counter = 0, progress_counter = 0, max_threads, telegram_count = 0, thread_count = 0;
    int shortlist_size, shortlist_remainder, shortlist_index;
    HANDLE hThreadArray[MAX_ACTIVE_THREADS] = { NULL };
    HANDLE ghMutex=NULL;      // mutex used for progress information during multithreaded calculations
    SYSTEM_INFO sysinfo;
    telegram *p_telegram = telegrams;
    DWORD dwWaitResult;
    t_shortlist_param shortlists[MAX_ACTIVE_THREADS] = { NULL };

    if (verbose >= VERB_PROG)
    {
        // Progress needs to be shown, create the mutex:
        ghMutex = CreateMutex(
            NULL,              // default security attributes
            FALSE,             // initially not owned
            NULL);             // unnamed mutex

        if (ghMutex == NULL)
        {
            eprintf(VERB_QUIET, "CreateMutex error: %d\n", GetLastError());
            exit(ERR_THREAD_CREATION);
        }
    }

    // determine the max amount of processes to spawn (maximised by either the array size or the #of CPU's in the system)
    GetSystemInfo(&sysinfo);
    if ((max_cpu == 0) || (max_cpu > sysinfo.dwNumberOfProcessors))
        max_threads = sysinfo.dwNumberOfProcessors;
    else
        max_threads = max_cpu;

    // count the number of telegrams, determine the action to be performed:
    while (p_telegram)
    {
        if ((p_telegram->contents.get_order() > 0) && (p_telegram->deshaped_contents.get_order() > 0))
            p_telegram->action = act_check;
        else
            if (p_telegram->contents.get_order() > 0)
                p_telegram->action = act_deshape;
            else
                p_telegram->action = act_shape;

        telegram_count++;
        p_telegram = p_telegram->next;
    }

    // determine the sizes of the shortlists:
    shortlist_size = telegram_count / max_threads;          // the length of the shortlists, rounded to the lower nr
    shortlist_remainder = telegram_count % max_threads;     // the amount of telegrams that remain to be divided over the shortlists

    for (shortlist_index = 0; shortlist_index < max_threads; shortlist_index++)
    {
        if (shortlist_remainder > 0)
        {
            shortlists[shortlist_index].n = shortlist_size + 1;
            shortlist_remainder--;
        }
        else
            shortlists[shortlist_index].n = shortlist_size;

        shortlists[shortlist_index].p_progress_counter = &progress_counter;
        shortlists[shortlist_index].progressMutex = ghMutex;
        shortlists[shortlist_index].calc_all = calc_all;
    }

    // find the addresses of the telegrams at the start of the shortlists
    shortlist_index = 1;
    telegram_counter = 1;
    p_telegram = telegrams;
    shortlists[0].p_telegram = telegrams;    // point the first shortlist to the start of the linked list

    while ( (p_telegram) && (shortlist_index <= max_threads) )
    // iterate over the telegrams and find each start address of the shortlists
    {
        p_telegram = p_telegram->next;

        if (telegram_counter == shortlists[shortlist_index - 1].n)
        {
            shortlists[shortlist_index].p_telegram = p_telegram;
            shortlist_index++;
            telegram_counter = 1;
        }
        else
            telegram_counter++;
    }

    // remember the amount of threads to start:
    thread_count = shortlist_index - 1;

    if (verbose >= VERB_GLOB)
    // print the shortlists
    {
        telegram_counter = 0;
        printf("Shortlists:\n");
        for (shortlist_index = 0; shortlist_index < max_threads; shortlist_index++)
        {
            printf("#%d: n=%d, p_telegram=%p\n", shortlist_index, shortlists[shortlist_index].n, shortlists[shortlist_index].p_telegram);
            telegram_counter += shortlists[shortlist_index].n;
        }
        printf("Total: %d; #of telegrams: %d\n", telegram_counter, telegram_count);
    }

    eprintf(VERB_FLOW, "Spawning %d thread(s).\n", max_threads);

    // spawn a thread for each shortlist with at least one telegram in it:
    for (shortlist_index = 0; shortlist_index < thread_count; shortlist_index++)
    {
//        convert_shortlist(&(shortlists[shortlist_index]));

        hThreadArray[shortlist_index] = CreateThread(
            NULL,
            1024,
            (LPTHREAD_START_ROUTINE)convert_shortlist,   
            &(shortlists[shortlist_index]),
            0,
            NULL);

        // exit if the thread could not be created:
        if (hThreadArray[shortlist_index] == NULL)
        {
            eprintf(VERB_QUIET, "Error creating thread, quitting ... \n");
            exit(ERR_THREAD_CREATION);
        }
        
        eprintf(VERB_FLOW, "Started thread #%d, handle = %p.\n", shortlist_index, hThreadArray[shortlist_index]);
    }

    if (verbose >= VERB_PROG)
    // show progress during calculations, update each PROGRESS_UPDATE_PERIOD msec
    // continue until all telegrams were calculated or all the threads have ended
        do 
        {
            // acquire the mutex:
            dwWaitResult = WaitForSingleObject(ghMutex, INFINITE);

            if (dwWaitResult == WAIT_OBJECT_0)
                telegram_counter = progress_counter;
            else
                printf("\rUnable to acquire mutex, progress not shown\n");

            ReleaseMutex(ghMutex);
        
            printf("\rUsing %d threads, progress: %d / %d telegrams = %d%%     ", thread_count, telegram_counter, telegram_count, 100* telegram_counter/telegram_count);
            dwWaitResult = WaitForMultipleObjects(thread_count, hThreadArray, TRUE, PROGRESS_UPDATE_PERIOD);

            if (dwWaitResult == WAIT_FAILED)
                printf("\nLastError: %d\n", GetLastError());

        } while ((dwWaitResult == WAIT_TIMEOUT) && (progress_counter < telegram_count)); 

    // Wait for our threads to finish, close the thread- and mutex handles:
    WaitForMultipleObjects(thread_count, hThreadArray, TRUE, INFINITE);
    for (shortlist_index = 0; shortlist_index < thread_count; shortlist_index++)
        if (hThreadArray[shortlist_index])
            CloseHandle(hThreadArray[shortlist_index]);
    if (verbose >= VERB_PROG)
        CloseHandle(ghMutex);

    eprintf(VERB_PROG, "\rFinished calculating %d telegrams using %d thread(s).\n", progress_counter, thread_count);

    return progress_counter;
}

// tbd: make a struct out of the parameters?
string output_telegrams_to_string(telegram* telegramlist, const string format, bool error_only, bool include_header, bool calc_all)
// Returns the telegrams in the same string format in which it is read in:
// One telegram is written on one line as a line in a csv-file: <decoded hex>;<encoded hex/base64 (param format);errorcode\n
// Ready to be output to screen, file, returned to python function, ...
// If error_only, only include the telegrams that have an error
// If include_header, print a header on the first line
// if format is "hex", output encoded data as hex. If not, output as base64.
{
    string output_result = "", line, csv_separators = "";
    int count, i;
    telegram* p_telegram = telegramlist;

    eprintf(VERB_FLOW, "Creating the output string.\n");

    if (include_header)
        if (calc_all)
            output_result = string("deshaped") + CSV_SEPARATOR + "shaped" + CSV_SEPARATOR + "errorcode" 
                            + CSV_SEPARATOR + "sb" + CSV_SEPARATOR + "esb"
                            + CSV_SEPARATOR + "word9" + CSV_SEPARATOR + "word10\n";
        else
            output_result = string("deshaped") + CSV_SEPARATOR + "shaped" + CSV_SEPARATOR + "errorcode\n";

    while (p_telegram)
    // iterate over telegrams and create a csv-line for each telegram
    {
        if (!(error_only && (p_telegram->errcode == ERR_NO_ERR)))
        // skip this telegram if (only errors should be outputted and telegram has no error)
        {
            if (p_telegram->errcode == ERR_INPUT_ERROR)
            // the telegram was not parsed because of an error in the input data
            // output the original line, CSV_SEPARATOR(s), error code
            {
                // Try to add some CSV_SEPARATORs to stick to the output format as much as possible
                csv_separators = "";
                count = 0;

                // count the number of CSV_SEPARATORs included in the string:
                for (i = 0; i < p_telegram->input_string.length(); i++)
                    if (p_telegram->input_string[i] == CSV_SEPARATOR)
                        count++;

                // determine the correct number of CVS_SEPARATORs and add them to the end (pointed to by i-1):
                if ( (count <= 1) || (p_telegram->input_string.at((size_t)(i - 1)) != CSV_SEPARATOR) )
                    csv_separators.push_back(CSV_SEPARATOR);

                if (count == 0)
                    csv_separators.push_back(CSV_SEPARATOR);

                // add the line and the separators to the output result:
                output_result += p_telegram->input_string + csv_separators + to_string(ERR_INPUT_ERROR) + "\n";
            }
            else
            // output the line to a csv-format
            {
                p_telegram->align(a_enc);

                // output the deshaped contents followed by a ;
                p_telegram->deshaped_contents.sprint_hex(line, p_telegram->number_of_userbits);
                output_result += line;
                output_result += CSV_SEPARATOR;

                // output the shaped contents, depending on format, followed by a ;
                if (format == "hex")
                    p_telegram->contents.sprint_hex(line, p_telegram->size);
                else
                    p_telegram->contents.sprint_base64(line, p_telegram->size);

                output_result += line + CSV_SEPARATOR;

                // add the error code and the newline:
                output_result += to_string(p_telegram->errcode);
                
                // add the SB and ESB if calculating all shapings:
                if (calc_all)
                {
                    p_telegram->align(a_calc);
                    output_result += CSV_SEPARATOR + to_string(p_telegram->get_scrambling_bits()) + CSV_SEPARATOR +
                        to_string(p_telegram->get_extra_shaping_bits()) +
                        CSV_SEPARATOR + to_string(p_telegram->word9) + CSV_SEPARATOR + to_string(p_telegram->word10);
                }
                output_result += "\n";
            }
        }

        p_telegram = p_telegram->next;
    }

    return output_result;
}

void output_telegrams_to_file(const string& output_string, const string filename)
// writes the output to the indicated file
{
    FILE* fp = NULL;

    // open the indicated file:
    if (fopen_s(&fp, filename.c_str(), "w"))
    {
        const size_t errmsglen = 100;// strerrorlen_s(errno) + 1;
        char errmsg[errmsglen];
        strerror_s(errmsg, errmsglen, errno);

        eprintf(VERB_QUIET, ERROR_COLOR "Error" ANSI_COLOR_RESET " opening output file %s, quitting. Errtext=%s\n", filename.c_str(), errmsg);
        exit(ERR_OUTPUT_FILE);
    }
    else
    {
        eprintf(VERB_FLOW, "Writing output to file: '%s'.\n", filename.c_str());
        fputs(output_string.c_str(), fp);
        fclose(fp);
    }
}

int get_first_error_code(telegram* telegramlist)
// returns the first error code in the telegram list
{
    telegram *p_telegram = telegramlist;

    // iterate over the telegrams and find the first error code != ERR_NO_ERR
    while (p_telegram)
    {
        if (p_telegram->errcode != ERR_NO_ERR)
            return p_telegram->errcode;

        p_telegram = p_telegram->next;
    }

    // no error found, return ERR_NO_ERR
    return ERR_NO_ERR;
}
