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
/*
t_telegramlist parse_file(string contents)
// loads data from an external file into recordlist (linked list)
// returns the amount of records in the linked list
{
    int recordcount = 0;
    t_telegramlist telegramlist;

    telegramlist = read_from_file_into_list(filename);
    eprintf(VERB_GLOB, OK_COLOR "\nRead %d input records from %s.\n" ANSI_COLOR_RESET, recordcount, filename.c_str());

    return telegramlist;
}
*/
DWORD WINAPI convert_telegram(telegram* p_telegram)
// converts the records from shaped to deshaped and vice versa
// if both shaped and deshaped input data is given in the same record, checks the correctness of the shaped telegram
// returns a meaningless 0
{
    longnum deshaped_data; 

    // determine what we're dealing with (shaped / unshaped / both):

    if ((p_telegram->contents.get_order() > 0) && (p_telegram->deshaped_contents.get_order() > 0))
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
    }
    else if (p_telegram->contents.get_order() > 0)
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
    }
    else if (p_telegram->deshaped_contents.get_order() > 0)
    // only unshaped bytes; shape them
    {
        eprintf(VERB_GLOB, "INPUT: Unshaped user data of %d bits:\n", p_telegram->number_of_userbits);
        p_telegram->deshaped_contents.print_fancy(VERB_GLOB, 8, p_telegram->number_of_userbits, NULL);

        if (p_telegram->force_long)
        // make this a long telegram before shaping it
            p_telegram->make_userdata_long();

        // shape the telegram:
        p_telegram->shape();

        // show check result and the telegram:
        eprintf(VERB_GLOB, "Created shaped telegram, performing checks:\n");

        p_telegram->errcode = ERR_NO_ERR;  // reset the error code (still set from shaping)
        p_telegram->check_shaped_deshaped();

        if (p_telegram->check_shaped_telegram() != ERR_NO_ERR)
            eprintf(VERB_QUIET, ERROR_COLOR "ERROR:" ANSI_COLOR_RESET" Created telegram that does not pass the checks.\n");
        
        // show the output:
        eprintf(VERB_GLOB, "OUTPUT: Shaped telegram: \n");
        p_telegram->print_contents_fancy(VERB_GLOB);
        eprintf(VERB_GLOB, "(hex:) ");
        p_telegram->align(a_enc);  // shift the bits to the left to prepare for printing
        p_telegram->contents.print_hex(VERB_GLOB, p_telegram->size);
        eprintf(VERB_GLOB, "\n");
    }
    else
        // this should never happen
        exit(ERR_LOGICAL_ERROR);

    return 0;
}

int convert_telegrams_multithreaded(t_telegramlist telegrams, unsigned int max_cpu)
// converts the records from shaped to deshaped and vice versa
// if both shaped and deshaped input data is given in the same record, checks the correctness of the shaped telegram
// uses as many threads as CPU's available (with a max of max_cpu)
// returns the amount of telegrams that were converted
{
    int counter=0, active_threads=0, thread_index=0, max_threads;
    HANDLE hThreadArray[MAX_ACTIVE_THREADS] = { 0 };
    SYSTEM_INFO sysinfo;

    GetSystemInfo(&sysinfo);
    eprintf(VERB_GLOB, "Spawning thread(s) on %d CPU(s).\n", max_cpu);

    // determine the max amount of processes to spawn (maximised by either the array size or the #of CPU's in the system)
    if ((max_cpu == 0) || (max_cpu > sysinfo.dwNumberOfProcessors))
        max_threads = sysinfo.dwNumberOfProcessors;
    else
        max_threads = max_cpu;

    if (max_threads == 1)
    {
        for (telegram*& p_telegram : telegrams)
        // use a simple iterator if only 1 thread is to be used (to save on overhead of creating and destroying threads)
        {
            convert_telegram(p_telegram);
            eprintf(VERB_PROG, "\rRunning with 1 thread; progress: %d / %d telegrams (%d%%) finished.   ", ++counter, (int)telegrams.size(), (int)(100 * counter / telegrams.size()));
        }

        eprintf(VERB_PROG, "\n");
        return counter;
    }

    // iterate over the telegrams and spawn threads to do the required calculations
    for (telegram*& p_telegram : telegrams)
    {
        do
        // wait until an unused thread is available in the pool:
        {
            active_threads = 0;
            for (thread_index = 0; thread_index < max_threads; thread_index++)
                if (hThreadArray[thread_index] != NULL)
                {
                    if (WaitForSingleObject(hThreadArray[thread_index], 0) == WAIT_TIMEOUT)
                        active_threads++;
                    else
                    {
                        CloseHandle(hThreadArray[thread_index]);  // inactive thread found with index thread_index
                        eprintf(VERB_GLOB, "\n");
                        eprintf(VERB_PROG, "\rRunning with %d thread(s); progress: %d / %d telegrams (%d%%) finished.", 
                                           max_threads, counter, (int)telegrams.size(), (int)(100 * counter / telegrams.size()));

                        break;
                    }
                }
                else
                    break;  // unused thread index
        } while (active_threads >= max_threads);

        counter++;
        eprintf(VERB_GLOB, ANSI_COLOR_YELLOW"\nParsing input line #%d: \n%s\n" ANSI_COLOR_RESET, counter, p_telegram->input_string.c_str());

        // found a free spot (@thread_index), create a new thread:
        hThreadArray[thread_index] = CreateThread(
            NULL,
            1024,
            (LPTHREAD_START_ROUTINE) convert_telegram,
            p_telegram,
            0,
            NULL
        );

        // check if the new thread was created:
        if (hThreadArray[thread_index] == NULL)
        {
            eprintf(VERB_QUIET, "Error creating thread, quitting ... \n");
            ExitProcess(3);
        }
        else
            eprintf(VERB_GLOB, "Started thread #%d, handle = %p.\n", thread_index, hThreadArray[thread_index]);

    }

    // clear the progress line
    eprintf(VERB_PROG, "\r                                                                                            \r");        

    // all telegrams have been / are being parsed, wait for our threads to finish:
    // WaitForMultipleObjects does not work here as some thread handles may be NULL
    for (thread_index = 0; thread_index < max_threads; thread_index++)
        if (hThreadArray[thread_index] != NULL)
            if (WaitForSingleObject(hThreadArray[thread_index], INFINITE) != WAIT_OBJECT_0)
                eprintf(VERB_QUIET, ERROR_COLOR "Error" ANSI_COLOR_RESET " waiting for thread to finish. Handle = % p.\n", hThreadArray[thread_index]);

    eprintf(VERB_PROG, "Finished running with %d thread(s).\n", max_threads);
    return counter;
}

string output_telegrams_to_string(t_telegramlist telegramlist, const string format, bool error_only, bool include_header)
// Returns the telegrams in the same string format in which it is read in:
// One telegram is written on one line as a line in a csv-file: <decoded hex>;<encoded hex/base64 (param format);errorcode\n
// Ready to be output to screen, file, returned to python function, ...
// If error_only, only include the telegrams that have an error
// If include_header, print a header on the first line
// if format is "hex", output encoded data as hex. If not, output as base64.
{
    string output_result, line;

    if (include_header)
        output_result += "deshaped,shaped,errorcode\n";

    for (telegram*& p_telegram : telegramlist)
    // iterate over telegrams and create a csv-line for each telegram
    {
        if (!(error_only && (p_telegram->errcode == ERR_NO_ERR)))
        // skip this telegram if (only errors should be outputted and telegram has no error)
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
                p_telegram->contents.sprint_base64(line, p_telegram->size); //(p_telegram->size + 8) / 8);

            output_result += line + CSV_SEPARATOR;

            // add the error code and the newline:
            output_result += to_string(p_telegram->errcode) + "\n";
        }
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
        eprintf(VERB_GLOB, "Writing output to file: '%s'.\n", filename.c_str());
        fputs(output_string.c_str(), fp);
        fclose(fp);
    }
}

int get_first_error_code(t_telegramlist telegramlist)
// returns the first error code in the list
{
    // iterate over the telegrams and find the first error code != ERR_NO_ERR
    for (telegram*& p_telegram : telegramlist)
        if (p_telegram->errcode != ERR_NO_ERR)
            return p_telegram->errcode;

    // no error found, return ERR_NO_ERR
    return ERR_NO_ERR;
}


/*
* 
int output_telegrams(t_telegramlist telegramlist, const string format, const string output_to, int error_only)
// Outputs the parsed telegrams. If output_to == NULL, output to screen.
// Otherwise output to a csv-file with name output_to
// Returns 0 if all telegrams were parsed correctly, or the error code that occurred last if not.
{
    string line, tmp;
    uint8_t arr[200] = { 0 };
    FILE* fp = NULL;
    int i;
    int end_result = 0;

    // open file and/or print header:
    if (output_to != "")
    // a filename is given, try to open it for writing
    {
        // open the indicated file:
        if (fopen_s (&fp, output_to.c_str(), "w"))
        {
            const size_t errmsglen = 100;// strerrorlen_s(errno) + 1;
            char errmsg[errmsglen];
            strerror_s(errmsg, errmsglen, errno);
            
            eprintf(VERB_QUIET, ERROR_COLOR "Error" ANSI_COLOR_RESET " opening output file %s, quitting. Errtext=%s\n", output_to.c_str(), errmsg);
            exit(ERR_OUTPUT_FILE);
        }
        else
        {
            eprintf(VERB_GLOB, "Writing output to file: '%s'.\n", output_to.c_str());
            fputs("deshaped" CSV_SEPARATOR "shaped" CSV_SEPARATOR "errorcode\n", fp);
        }
    }
    else
    // no filename, output header to stdout
        eprintf(VERB_QUIET, "deshaped,shaped,errorcode\n");

    for (telegram*& p_telegram : telegramlist)
    // iterate over telegrams and create a csv-line for each telegram
    {
        if (!(error_only && (p_telegram->errcode == ERR_NO_ERR)))
        // skip this telegram if (only errors should be outputted and telegram has no error)
        {
            p_telegram->align(a_enc);
            i = p_telegram->deshaped_contents.sprint_hex(line, p_telegram->number_of_userbits);
            line += CSV_SEPARATOR;

            if (format == "hex")
            {
                p_telegram->contents.sprint_hex(tmp, p_telegram->size);
                line += tmp;
            }
            else
            {
                p_telegram->contents.write_to_array(arr, (p_telegram->size + 8) / 8);
                b64_encode(arr, (p_telegram->size + 8) / 8, tmp);
                line += tmp;
            }
            line = line + CSV_SEPARATOR + to_string(p_telegram->errcode);
            line += "\n";

            // output the csv-line to the indicated medium (file or screen):
            if (fp != NULL)
                fputs(line.c_str(), fp);
            else
                eprintf(VERB_QUIET, "%s", line.c_str());
        }

        // determine the end result of all calculations:
        if (p_telegram->errcode != ERR_NO_ERR)
            end_result = p_telegram->errcode;
    }

    if (fp != NULL)
        fclose(fp);

    return end_result;
}
*/