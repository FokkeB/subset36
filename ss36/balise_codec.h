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

#ifndef BALISE_CODEC_H
#define BALISE_CODEC_H

#define MAX_ACTIVE_THREADS 100                  // max amount of threads to spawn. Array size of list of thread handles.
#define PROG_VERSION "5 (March 2025)"           // version of this program
#define CSV_SEPARATOR ";"						// separator to be used in output

// error codes:
#define ERR_NO_INPUT            20       // no input specified
#define ERR_OUTPUT_FILE         21       // error creating output file
#define ERR_PROCESS_CREATE		22		 // error creating a new process

#include "telegram.h"               // subset 36 - related functions
#include "useful_functions.h"		// supporting functions
#include "parse_input.h"			// to read in an external file
#include "colors.h"					// pretty output
#include <windows.h>				// multithreading
#include <time.h>					// to time the execution
#include <string>					// string functions
#include <fstream>					// file i/p

string read_from_file(string filename);
int count(telegram* p_telegrams);
int convert_telegrams_multithreaded(telegram* telegrams, unsigned int max_cpu, bool calc_all);
string output_telegrams_to_string(telegram* telegramlist, const string format, bool error_only, bool include_header);
void output_telegrams_to_file(const string& output_string, const string filename);
int get_first_error_code(telegram* telegramlist);

#endif