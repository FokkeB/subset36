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
constexpr char CSV_SEPARATOR = ';';				// separator to be used in output (comma separated values)
#define PROGRESS_UPDATE_PERIOD 250              // update the progress indicator each PROGRESS_UPDATE_PERIOD msec
#define VER_FILEVERSION_STR         "9 (December 23rd, 2025)"
 
//#include "..\version.h"
#include "parse_input.h"        // parse the input into a structured linked list
#include "telegram.h"           // subset 36 - related functions
#include "useful_functions.h"   // supporting functions
#include "colors.h"             // pretty colors in the output
#include <time.h>               // to time the execution
#include <string>               // string functions
#include <fstream>              // to read in an external file
#include <errno.h>              // for errno
#include <string.h>             // for strerror
#include <future>
#include "BS_thread_pool.hpp"

string read_from_file(string filename);
void convert_telegram(telegram* p_telegram);
void telegram_calc_all(telegram* p_start_telegram);
void convert_telegrams_multithreaded(telegram* telegrams, unsigned int max_cpu, bool calc_all);
string output_telegrams_to_string(telegram *telegramlist, const string format, bool error_only, bool include_header, bool calc_all);
void output_telegrams_to_file(const string& output_string, const string filename);
int get_first_error_code(telegram *telegramlist);

#endif