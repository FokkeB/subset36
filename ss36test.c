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
 * ss36test.c - use the ss36-library to perform calculations with balise contents. See command line options in function main below.
*/

#define OPT_VERSION "3.19"          // needed to satisfy OPT-library. Also set as a preprocessor definition for opt_proc.c. 
#define MAX_ACTIVE_THREADS 100      // max amount of threads to spawn. Array size of list of thread handles.

#include "useful_functions.h"
#include "ss36.h"               // subset 36 - related functions
#include "parse_input.h"        // to read in an external file
#include "colors.h"
#include <opt.h>                // parse command line parameters
#include <windows.h>            // multithreading

int verbose = VERB_PROG;

int load_testdata_in_list(char* filename, t_telegram** telegrams)
// loads data from an external file into recordlist (linked list)
// returns the amount of records in the linked list
{
    int recordcount = 0;

    *telegrams = read_from_file_into_list(filename, &recordcount);
    eprintf(VERB_GLOB, OK_COLOR"\nRead %d input records from %s.\n"ANSI_COLOR_RESET, recordcount, filename);

    return recordcount;
}

DWORD WINAPI convert_telegram(t_telegram* p_telegram)
// converts the records from shaped to deshaped and vice versa
// if both shaped and deshaped input data is given in the same record, checks the correctness of the shaped telegram
// uses as many threads as CPU's available (with a max of MAX_ACTIVE_THREADS)
{
    t_longnum deshaped_data; 

    long_fill(deshaped_data, 0);  // clear variable 
    align_telegram(p_telegram, a_calc);

    // determine what we're dealing with (shaped / unshaped / both):

    if ((get_order(p_telegram->contents) > 0) && (get_order(p_telegram->deshaped_contents) > 0))
        // both shaped and unshaped; verify that the shaped telegram is correct:
    {
        // first print the input:
        eprintf(VERB_GLOB, "INPUT:");
        eprintf(VERB_GLOB, "\nUnshaped %d bytes:\n", p_telegram->number_of_userbits);
        print_longnum_fancy(VERB_GLOB, p_telegram->contents, 16, p_telegram->number_of_shapeddata_bits, &no_colors);
        eprintf(VERB_GLOB, "\nShaped %d bytes:\n", p_telegram->number_of_shapeddata_bits);
        print_telegram_contents_fancy(VERB_GLOB, p_telegram);

        // check the shaped telegram:
        eprintf(VERB_GLOB, "Perform the condition-checks and content checks of the shaped telegram:\n");
        if (check_shaped_telegram(p_telegram) != ERR_NO_ERR)
            eprintf(VERB_GLOB, ERROR_COLOR"At least one error occured.\n"ANSI_COLOR_RESET);

        // check if the deshaping of the shaped contents leads to the given unshaped contents
        eprintf(VERB_GLOB, "Check contents of the shaped telegram:\t\t");
        deshape(p_telegram, deshaped_data);
        if (long_cmp(p_telegram->deshaped_contents, deshaped_data))
            eprintf(VERB_GLOB, OK_COLOR"OK\n"ANSI_COLOR_RESET);
        else
            eprintf(VERB_GLOB, ERROR_COLOR"NOK\n"ANSI_COLOR_RESET);
    }
    else if (get_order(p_telegram->contents) > 0)
        // only shaped bytes; de-shape them
    {
        // show the inputs and outputs:
        eprintf(VERB_GLOB, "INPUT: Shaped telegram of %d bytes:\n", p_telegram->size);
        print_telegram_contents_fancy(VERB_GLOB, p_telegram);

        // check the shaped telegram:
        if (check_shaped_telegram(p_telegram) != ERR_NO_ERR)
            eprintf(VERB_GLOB, ERROR_COLOR"Warning:"ANSI_COLOR_RESET" At least one error occured, continuing to deshape the telegram.\n");

        deshape(p_telegram, p_telegram->deshaped_contents);

        // output the shaped bits, first align them:
        eprintf(VERB_GLOB, "\nOUTPUT: %d deshaped user bits:\n", p_telegram->number_of_userbits);
        print_longnum_fancy(VERB_GLOB, p_telegram->deshaped_contents, 8, p_telegram->number_of_userbits, &no_colors);
        align_telegram(p_telegram, a_enc);
        eprintf(VERB_GLOB, "(left aligned hex:) ");
        print_longnum_hex(VERB_GLOB, p_telegram->deshaped_contents);
        eprintf(VERB_GLOB, "\n");
    }
    else if (get_order(p_telegram->deshaped_contents) > 0)
        // only unshaped bytes; shape them
    {
        eprintf(VERB_GLOB, "INPUT: Unshaped user data of %d bits:\n", p_telegram->number_of_userbits);
        print_longnum_fancy(VERB_GLOB, p_telegram->deshaped_contents, 8, p_telegram->number_of_userbits, &no_colors);

        // shape the telegram:
        shape(p_telegram);

        // show check result and the telegram:
        eprintf(VERB_GLOB, "Created shaped telegram, performing final check:\n");
        if (check_shaped_telegram(p_telegram) != ERR_NO_ERR)
            eprintf(VERB_QUIET, ERROR_COLOR"ERROR:"ANSI_COLOR_RESET" Created telegram that does not pass the checks.\n");
        else
        {
            eprintf(VERB_GLOB, "All checks "OK_COLOR"OK\n"ANSI_COLOR_RESET);
            eprintf(VERB_GLOB, "OUTPUT: Shaped telegram: \n");
            print_telegram_contents_fancy(VERB_GLOB, p_telegram);
            eprintf(VERB_GLOB, "(hex:) ");
            print_longnum_hex(VERB_GLOB, p_telegram->contents);
            eprintf(VERB_GLOB, "\n");
        }
    }
    else
        // this should never happen
        exit(ERR_LOGICAL_ERROR);

    return 0;
}

int convert_telegrams_singlethreaded(t_telegram* telegrams)
// converts the records from shaped to deshaped and vice versa
// if both shaped and deshaped input data is given in the same record, checks the correctness of the shaped telegram
// only uses the main thread
{
    t_telegram* p_telegram = telegrams;    // pointer to walk through the linked list of telegrams
    int counter = 0, n_telegrams=0;

    // first count the nr of telegrams (used to show the progress)
    n_telegrams = count_telegrams(telegrams);

    while (p_telegram != NULL)
    {
        counter++;
        eprintf(VERB_GLOB, ANSI_COLOR_YELLOW"\nParsing input line #%d: \n%s\n"ANSI_COLOR_RESET, counter, p_telegram->input_string);

        convert_telegram(p_telegram);

        eprintf(VERB_GLOB, "\n");
        eprintf(VERB_PROG, "\rRunning single threaded; progress: %d / %d telegrams (%d%%) finished.", counter, n_telegrams, (100*counter/n_telegrams));

        p_telegram = p_telegram->next;
    }

    if (verbose == VERB_PROG)
        // clear the progress line
        eprintf(VERB_PROG, "\r");

    eprintf(VERB_GLOB, "\nReady parsing %d telegram(s).\n", counter);

    return counter;
}

int convert_telegrams_multithreaded(t_telegram* telegrams)
// converts the records from shaped to deshaped and vice versa
// if both shaped and deshaped input data is given in the same record, checks the correctness of the shaped telegram
{
    t_telegram* p_telegram = telegrams;    // pointer to walk through the linked list of telegrams
    int counter=0, n_telegrams=0, active_threads=0, thread_index=0, max_threads;
    HANDLE  hThreadArray[MAX_ACTIVE_THREADS] = { 0 };
    SYSTEM_INFO sysinfo;

    GetSystemInfo(&sysinfo);
    eprintf(VERB_GLOB, "Spawning threads on %d CPUs.\n", sysinfo.dwNumberOfProcessors);

    // count the nr of telegrams (used to show the progress)
    n_telegrams = count_telegrams(telegrams);

    // determine the max amount of processes to spawn (maximised by either the array size or the #of CPU's in the system)
    max_threads = (sysinfo.dwNumberOfProcessors > MAX_ACTIVE_THREADS) ? MAX_ACTIVE_THREADS : sysinfo.dwNumberOfProcessors;

    // iterate over the telegrams and spawn threads to do the required calculations
    while (p_telegram != NULL)
    {
        counter++;
        eprintf(VERB_GLOB, ANSI_COLOR_YELLOW"\nParsing input line #%d: \n%s\n"ANSI_COLOR_RESET, counter, p_telegram->input_string);

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
                        eprintf(VERB_PROG, "\rRunning with %d threads; progress: %d / %d telegrams (%d%%) finished.", max_threads, counter, n_telegrams, (100 * counter / n_telegrams));

                        break;
                    }
                }
                else
                    break;  // unused thread index
        } while (active_threads >= max_threads);

        // found a free spot (@thread_index), create a new thread:
        hThreadArray[thread_index] = CreateThread(
            NULL,
            1024,
            convert_telegram,
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

        // select the next telegram:
        p_telegram = p_telegram->next;
    }

    if (verbose == VERB_PROG)
    // clear the progress line
        eprintf(VERB_PROG, "\r");        

    // all telegrams have been / are being parsed, wait for our threads to finish:
    // WaitForMultipleObjects does not work here as some thread handles may be NULL
    for (thread_index = 0; thread_index < max_threads; thread_index++)
        if (hThreadArray[thread_index] != NULL)
            if (WaitForSingleObject(hThreadArray[thread_index], INFINITE) != WAIT_OBJECT_0)
                eprintf(VERB_QUIET, ERROR_COLOR"Error"ANSI_COLOR_RESET" waiting for thread to finish. Handle = % p.", hThreadArray[thread_index]);

    eprintf(VERB_GLOB, "\nReady parsing %d telegram(s).\n", counter);

    return counter;
}

void output_telegrams(t_telegram* first_telegram, char* format, char* output_to)
// Outputs the parsed telegrams. If output_to == NULL, output to screen.
// Otherwise output to a csv-file with name output_to
{
    t_telegram* p_telegram = first_telegram;    // pointer to walk through the linked list of telegrams
    char line[500] = { 0 }, b64string[200] = {0}, arr[200] = {0}; 
    FILE* fp = NULL;
    int i;

    // open file and/or print header:
    if (output_to != NULL)
    // a filename is given, try to open it for writing
    {
        // open the indicated file:
        fp = fopen(output_to, "w");
        if (fp == NULL)
        {
            eprintf(VERB_QUIET, ERROR_COLOR"Error"ANSI_COLOR_RESET" opening output file, quitting. Errtext=%s\n", strerror(errno));
            exit(ERR_OUTPUT_FILE);
        }
        else
        {
            eprintf(VERB_GLOB, "Writing output to file: '%s'.\n", output_to);
            fputs("deshaped,shaped,errorcode\n", fp);
        }
    }
    else
    // no filename, output header to stdout
        eprintf(VERB_QUIET, "deshaped,shaped,errorcode\n");

    while (p_telegram != NULL)
    // iterate over telegrams and create a csv-line for each telegram
    {
        align_telegram(p_telegram, a_enc);
        i = sprint_longnum_hex(line, p_telegram->deshaped_contents, p_telegram->number_of_userbits);
        line[i] = ',';
        line[i + 1] = 0;
 
        if ( (format != NULL) && (strcmp(format, "hex") == 0) )
            sprint_longnum_hex(&line[i+1], p_telegram->contents, p_telegram->size);
        else
        {
            longnum_to_array(arr, p_telegram->contents, (p_telegram->size + 8) / 8);
            b64_encode(arr, (p_telegram->size+8)/8, b64string);
            strcat(line, b64string);
        }

        sprintf (&line[strlen(line)], ",%d\n", p_telegram->errcode);
        
        // output the csv-line to the indicated medium (file or screen):
        if (fp != NULL)
            fputs(line, fp);
        else
            eprintf(VERB_QUIET, "%s", line);

        p_telegram = p_telegram->next;
    }

    if (fp != NULL)
        fclose(fp);
}
// todo: create a test set to further prove the correct workings

int main(int argc, char** argv)
{
    char* input_file = NULL;            // the input file name
    char* output_file = NULL;           // the output file name
    char* literal = NULL;               // the input literal 
    char* output_format = NULL;         // the output format (if set to hex, output as hex. Otherwise: output in base64)
    t_telegram* telegrams = NULL;       // pointer to the first element in the linked list of telegrams
    int result = ERR_NO_ERR;            // end result of this program (0=success
    int telegramcount = 1;              // #telegrams
    int force_multi = 0;                // set to true, this will force the use of multithreading, even when verbosity level > 0 (will lead to garbled output)
//    int force_long = 0;               // force long format for shaped telegrams
    int help = 0;                       // show help msg

    setupConsole();  // for colorful output

    // get the command line options using opt:
    optrega(&help, OPT_BOOL, 'h', "help", "Show this help message.");
    optrega(&input_file, OPT_STRING, 'i', "input_filename", "Read lines with data from the indicated file (UTF-8, no BOM) and convert its contents from shaped data to unshaped data and vice versa. SS39 automatically determines the used format (base64/hex) and length (short/long). Lines must be separated by '\\n' ('\\r' will be ignored). If both the shaped and unshaped data are given on one line (separated by a comma), SS39 will check the correct shaping. Comments must be preceded by '#'.");
    optrega(&output_file, OPT_STRING, 'o', "output_filename", "Write output to this file.");
    optrega(&literal, OPT_STRING, 's', "string", "Input string (shaped and/or deshaped string in base64/hex), format identical to one line in the input file.");
    optrega(&verbose, OPT_INT, 'v', "verbose", "Level of verbosity: 0 (quiet, only show result), 1 (+show progress, default), 2 (+basic output) or 3 (+lots of output).");
    optrega(&force_multi, OPT_BOOL, 'm', "multithread", "Force multithreading at verbosity > 1 (this will lead to garbled output of different processes interrupting each other). Multithreading is enabled by default for verbosity <= 1.");
//    optrega(&force_long, OPT_BOOL, 'l', "force_long", "Force shaping to the long format, even if the unshaped data is of short format. If not specified, SS39 will use the same format as the input data.");
    optrega(&output_format, OPT_STRING, 'f', "format_output", "Output format for the shaped telegram: 'hex' or 'base64' (default).");
    optVersion("1");
// todo: add force_long
 
    opt(&argc, &argv);

    if (help)
    // show the help message and die
    {
        optPrintUsage();
        return ERR_NO_ERR;
    }

    // execute the commands from the command line:
    
    // first get the input (either input file or literal)
    if (input_file != NULL)
    // filename is set, load the data from the file:
        telegramcount = load_testdata_in_list(input_file, &telegrams);
    else
        if ( (literal == NULL) || ((telegrams = parse_input_line(literal)) == NULL) )
        {
            eprintf(VERB_QUIET, ERROR_COLOR"ERROR: No input specified, quitting.\n"ANSI_COLOR_RESET);
            exit(ERR_NO_INPUT);
        }
     
    // convert the input to the other format or check the correctness of a telegram:
    if ( (force_multi || (verbose <= VERB_PROG)) && (literal == NULL) )
        result = convert_telegrams_multithreaded(telegrams);
    else
        result = convert_telegrams_singlethreaded(telegrams);

    // output the telegrams:
    output_telegrams(telegrams, output_format, output_file);

    // destroy the telegrams:
    destroy_telegrams(telegrams);

    eprintf(VERB_QUIET, "Ready. Press any key to continue ...");
    char dummy = getch();

    restoreConsole();

    return result;
}