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

/**
 * ss36test.c - use the ss36-library to perform calculations with balise contents. See command line options in function main below.
*/

#define OPT_VERSION "3.19"          // needed to satisfy OPT-library. Also set as a preprocessor definition for opt_proc.c. 
#define MAX_ACTIVE_THREADS 100      // max amount of threads to spawn. Array size of list of thread handles.
#define PROG_VERSION "3 (October 4th 2023)"            // version of this program

#include "useful_functions.h"
#include "ss36.h"               // subset 36 - related functions
#include "parse_input.h"        // to read in an external file
#include "colors.h"
#include "opt\opt.h"            // parse command line parameters, see https://public.lanl.gov/jt/Software/
#include <windows.h>            // multithreading
#include <time.h>               // to time the execution
#include "test_functions.h"           // test functions
#include "base64.h"             // base64 encoding of result

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
{
    t_longnum deshaped_data; 

    long_fill(deshaped_data, 0);  // clear variable 
    align_telegram(p_telegram, a_calc);  // shift the bits to the right to align with bit 0
    //p_telegram->errcode = ERR_NO_ERR;

    // determine what we're dealing with (shaped / unshaped / both):

    if ((long_get_order(p_telegram->contents) > 0) && (long_get_order(p_telegram->deshaped_contents) > 0))
    // both shaped and unshaped; verify that the shaped telegram is correct:
    {
        // first print the input:
        eprintf(VERB_GLOB, "INPUT:");
        eprintf(VERB_GLOB, "\nUnshaped contents (%d bits):\n", p_telegram->number_of_userbits);
        long_print_fancy(VERB_GLOB, p_telegram->deshaped_contents, 8, p_telegram->number_of_userbits, NULL);
        eprintf(VERB_GLOB, "\nShaped contents (%d bits):\n", p_telegram->size);
        print_telegram_contents_fancy(VERB_GLOB, p_telegram);

        // check the shaped telegram:
        eprintf(VERB_GLOB, "Perform the condition-checks and content checks of the shaped telegram:\n");
        if (check_shaped_telegram(p_telegram) != ERR_NO_ERR)
            eprintf(VERB_GLOB, ERROR_COLOR"At least one error occured.\n"ANSI_COLOR_RESET);

        // check if the deshaping of the shaped contents leads to the given unshaped contents
        check_shaped_deshaped(p_telegram); 
    }
    else if (long_get_order(p_telegram->contents) > 0)
    // only shaped bytes; check the telegram and de-shape it
    {
        // show the inputs and outputs:
        eprintf(VERB_GLOB, "INPUT: Shaped telegram of %d bits:\n", p_telegram->size);
        print_telegram_contents_fancy(VERB_GLOB, p_telegram);

        // check the shaped telegram:
        if (check_shaped_telegram(p_telegram) != ERR_NO_ERR)
            eprintf(VERB_GLOB, ERROR_COLOR"Warning:"ANSI_COLOR_RESET" At least one error occured, continuing to deshape the telegram.\n");

        // deshape the telegram:
        deshape(p_telegram, p_telegram->deshaped_contents);

        // output the shaped bits:
        align_telegram(p_telegram, a_enc);  // shift the bits to the left to prepare for printing
        eprintf(VERB_GLOB, "OUTPUT: %d deshaped user bits:\n", p_telegram->number_of_userbits);
        long_print_hex(VERB_GLOB, p_telegram->deshaped_contents, p_telegram->number_of_userbits);
        eprintf(VERB_GLOB, "\n");
    }
    else if (long_get_order(p_telegram->deshaped_contents) > 0)
    // only unshaped bytes; shape them
    {
        eprintf(VERB_GLOB, "INPUT: Unshaped user data of %d bits:\n", p_telegram->number_of_userbits);
        long_print_fancy(VERB_GLOB, p_telegram->deshaped_contents, 8, p_telegram->number_of_userbits, NULL); 

        // shape the telegram:
        shape(p_telegram);

        // show check result and the telegram:
        eprintf(VERB_GLOB, "Created shaped telegram, performing checks:\n");

        p_telegram->errcode = ERR_NO_ERR;  // reset the error code (still set from shaping)
        check_shaped_deshaped(p_telegram); 

        if (check_shaped_telegram(p_telegram) != ERR_NO_ERR)
            eprintf(VERB_QUIET, ERROR_COLOR"ERROR:"ANSI_COLOR_RESET" Created telegram that does not pass the checks.\n");
        
        // show the output:
        eprintf(VERB_GLOB, "OUTPUT: Shaped telegram: \n");
        print_telegram_contents_fancy(VERB_GLOB, p_telegram);
        eprintf(VERB_GLOB, "(hex:) ");
        align_telegram(p_telegram, a_enc);  // shift the bits to the left to prepare for printing
        long_print_hex(VERB_GLOB, p_telegram->contents, p_telegram->size);
        eprintf(VERB_GLOB, "\n");
    }
    else
        // this should never happen
        exit(ERR_LOGICAL_ERROR);

    return 0;
}

int convert_telegrams_multithreaded(t_telegram* telegrams, int max_cpu)
// converts the records from shaped to deshaped and vice versa
// if both shaped and deshaped input data is given in the same record, checks the correctness of the shaped telegram
// uses as many threads as CPU's available (with a max of max_cpu)
{
    t_telegram* p_telegram = telegrams;    // pointer to walk through the linked list of telegrams
    int counter=0, n_telegrams=0, active_threads=0, thread_index=0, max_threads;
    HANDLE hThreadArray[MAX_ACTIVE_THREADS] = { 0 };
    SYSTEM_INFO sysinfo;

    GetSystemInfo(&sysinfo);
    eprintf(VERB_GLOB, "Spawning thread(s) on %d CPU(s).\n", max_cpu);

    // count the nr of telegrams (used to show the progress)
    n_telegrams = count_telegrams(telegrams);

    // determine the max amount of processes to spawn (maximised by either the array size or the #of CPU's in the system)
    if ((max_cpu == 0) || (max_cpu > sysinfo.dwNumberOfProcessors))
        max_threads = sysinfo.dwNumberOfProcessors;
    else
        max_threads = max_cpu;

    // iterate over the telegrams and spawn threads to do the required calculations
    while (p_telegram != NULL)
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
                        eprintf(VERB_PROG, "\rRunning with %d thread(s); progress: %d / %d telegrams (%d%%) finished.", max_threads, counter, n_telegrams, (100 * counter / n_telegrams));

                        break;
                    }
                }
                else
                    break;  // unused thread index
        } while (active_threads >= max_threads);

        counter++;
        eprintf(VERB_GLOB, ANSI_COLOR_YELLOW"\nParsing input line #%d: \n%s\n"ANSI_COLOR_RESET, counter, p_telegram->input_string);

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

    // clear the progress line
    eprintf(VERB_PROG, "\n");        

    // all telegrams have been / are being parsed, wait for our threads to finish:
    // WaitForMultipleObjects does not work here as some thread handles may be NULL
    for (thread_index = 0; thread_index < max_threads; thread_index++)
        if (hThreadArray[thread_index] != NULL)
            if (WaitForSingleObject(hThreadArray[thread_index], INFINITE) != WAIT_OBJECT_0)
                eprintf(VERB_QUIET, ERROR_COLOR"Error"ANSI_COLOR_RESET" waiting for thread to finish. Handle = % p.", hThreadArray[thread_index]);

//    eprintf(VERB_GLOB, "\nReady parsing %d telegram(s).\n", counter);

    return counter;
}

void output_telegrams(t_telegram* first_telegram, char* format, char* output_to, int error_only)
// Outputs the parsed telegrams. If output_to == NULL, output to screen.
// Otherwise output to a csv-file with name output_to
{
    t_telegram* p_telegram = first_telegram;    // pointer to walk through the linked list of telegrams
    char line[500], b64string[200] = {0}, arr[200] = {0}; 
    FILE* fp = NULL;
    int i;
    #define CSV_SEPARATOR ";"

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
            fputs("deshaped"CSV_SEPARATOR"shaped"CSV_SEPARATOR"errorcode\n", fp);
        }
    }
    else
    // no filename, output header to stdout
        eprintf(VERB_QUIET, "deshaped,shaped,errorcode\n");

    while (p_telegram != NULL)
    // iterate over telegrams and create a csv-line for each telegram
    {
        if (!(error_only && (p_telegram->errcode == ERR_NO_ERR)))
        // skip this telegram if (only errors should be outputted and telegram has no error)
        {
            //long_shiftleft(p_telegram->deshaped_contents, 2);
            align_telegram(p_telegram, a_enc);
            i = long_sprint_hex(line, p_telegram->deshaped_contents, p_telegram->number_of_userbits);
            strcat(line, CSV_SEPARATOR);

            if ((format != NULL) && (strcmp(format, "hex") == 0))
                long_sprint_hex(&line[i + 1], p_telegram->contents, p_telegram->size);
            else
            {
                longnum_to_array(arr, p_telegram->contents, (p_telegram->size + 8) / 8);
                b64_encode(arr, (p_telegram->size + 8) / 8, b64string);
                strcat(line, b64string);
            }
            sprintf(&line[strlen(line)], CSV_SEPARATOR"%d\n", p_telegram->errcode);

            // output the csv-line to the indicated medium (file or screen):
            if (fp != NULL)
                fputs(line, fp);
            else
                eprintf(VERB_QUIET, "%s", line);
        }

        p_telegram = p_telegram->next;
    }

    if (fp != NULL)
        fclose(fp);
}

int main(int argc, char** argv)
{
    char* input_file = NULL;            // the input file name
    char* output_file = NULL;           // the output file name
    char* literal = NULL;               // the input literal 
    char* output_format = NULL;         // the output format (if set to hex, output as hex. Otherwise: output in base64)
    t_telegram* telegrams = NULL;       // pointer to the first element in the linked list of telegrams
    int result = ERR_NO_ERR;            // end result of this program (0=success)
    int telegramcount = 1;              // #telegrams
    int max_cpu = 0;                   // maximum nr of simultaneous processes (0=unlimited)
    int force_long = 0;               // force long format for shaped telegrams
    int help = 0;                       // show help msg
    int show_err = 0;                   // show the meaning of the error codes
    int error_only = 0;                 // only show output lines with an error code != 0
    int run_tests = 0;                  // run various tests

    clock_t start, end;
    double execution_time;

    setupConsole();  // for colorful output

    // get the command line options using opt:
    optrega(&help, OPT_BOOL, 'h', "help", "Show this help message.");
    optrega(&input_file, OPT_STRING, 'i', "input_filename", "Read lines with data from the indicated csv-file (UTF-8, no BOM) and convert its contents from shaped data to unshaped data and vice versa. This tool automatically determines the used format (base64/hex) and length (short/long). Lines must be separated by '\\n' ('\\r' will be ignored). If both the shaped and unshaped data are given on one line (separated by a comma or semicolon), this tool will check the correct shaping. Comments must be preceded by '#'.");
    optrega(&output_file, OPT_STRING, 'o', "output_filename", "Write output to this file.");
    optrega(&literal, OPT_STRING, 's', "string", "Input string (shaped and/or deshaped string in base64/hex), format identical to one line in the input file.");
    optrega(&verbose, OPT_INT, 'v', "verbose", "Level of verbosity: 0 (quiet, only show result), 1 (+show progress, default), 2 (+basic output) or 3 (+lots of output).");
    optrega(&max_cpu, OPT_INT, 'm', "max_cpu", "Max nr of cpu's to use. Multithreading is enabled by default for verbosity <= 1 or if set to 0.");
    optrega(&force_long, OPT_BOOL, 'l', "force_long", "Force shaping to the long format (1023 bits), even if the unshaped data is of short format (341 bits). If not specified, this tool will use the same format as the input data.");
    optrega(&output_format, OPT_STRING, 'f', "format_output", "Output format for the shaped telegram: 'hex' or 'base64' (default).");
    optrega(&show_err, OPT_BOOL, 'e', "show_error_codes", "Shows the meaning of the error codes that can be generated when checking / shaping telegrams.");
    optrega(&error_only, OPT_BOOL, 'E', "error_only", "Output only the telegrams in which an error was found (-e gives the error codes).");
    optrega(&run_tests, OPT_BOOL, 'T', "run_tests", "Run various tests to check the workings of this program, using input from the input file. This tool uses lines from the input files containing both decoded and encoded contents.");

    optVersion(PROG_VERSION);
    optTitle("This is BALISE_CODEC by fokke@bronsema.net. See https://github.com/FokkeB/subset36 for more details. Version: \n"PROG_VERSION);
    opt(&argc, &argv);

    if (help)
    // show the help message and die
    {
        optPrintUsage();
        return ERR_NO_ERR;
    }

    if (show_err)
    // show the meaning of the error messages and die
    {
        printf("The error codes below can be generated when checking / shaping a telegram. \n");
        printf("See SUBSET - 036 paragraph 4.3 for more details concerning error codes >= 10.\n");
        printf("Increase verbosity to 2 to see detailed information about the errors found during conversion.\n");
        printf("Error code\tExplanation\n");
        printf("\t0\tNo error\n");
        printf("\t1\tNo input specified\n");
        printf("\t2\tA logical error (not further specified)\n");
        printf("\t3\tError creating output file\n");
        printf("\t4\tError during memory allocation\n");
        printf("\t10\tAlphabet condition fails\n");
        printf("\t11\tOff-sync parsing condition fails\n");
        printf("\t12\tAperiodicity condition fails\n");
        printf("\t13\tUndersampling check fails\n");
        printf("\t14\tControl bits check fails\n");
        printf("\t15\tCheck bits check fails\n");
        printf("\t16\tOverflow of SB and ESB (should never occur, please contact author if it did)\n");
        printf("\t17\tError during conversion from 10 bits to 11 bits (11-bit value not found in list of transformation words)\n");
        printf("\t18\tShaped contents do not match the unshaped contents (encoding error)\n");

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

    if (force_long)
        make_long(telegrams);

    if (run_tests)
    {
        int errs = run_sanity_tests(telegrams);
        printf("Testing complete. Result:\t\t\t");
        print_result(errs);

        return errs;
    }

    // set the #processes to 1 if the verbosity level is higher than VERB_PROG
    if (verbose > VERB_PROG)
        max_cpu = 1;

    start = clock();

    // convert the input to the other format or check the correctness of a telegram:
    result = convert_telegrams_multithreaded(telegrams, max_cpu);

    end = clock();
    execution_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    eprintf(VERB_PROG, "Calculation time: %.2f secs\n", execution_time);

    // output the telegrams:
    output_telegrams(telegrams, output_format, output_file, error_only);

    // destroy the telegrams:
    destroy_telegrams(telegrams);

//    eprintf(VERB_QUIET, "Ready. Press any key to continue ...");
//    char dummy = getch();

    restoreConsole();

    return result;
}