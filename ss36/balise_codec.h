#pragma once
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
#define PROG_VERSION "5 (August 2025)"          // version of this program
#define CSV_SEPARATOR ";"						// separator to be used in output

#include "telegram.h"               // subset 36 - related functions
#include "useful_functions.h"
#include "parse_input.h"        // to read in an external file
#include "colors.h"
#include <windows.h>            // multithreading
#include <time.h>               // to time the execution
#include <string>
#include "ansi_escapes.h"
#include <fstream>

string read_from_file(string filename);
int convert_telegrams_multithreaded(t_telegramlist telegrams, unsigned int max_cpu);
string output_telegrams_to_string(t_telegramlist telegramlist, const string format, bool error_only, bool include_header);
void output_telegrams_to_file(const string& output_string, const string filename);
int get_first_error_code(t_telegramlist telegramlist);

#endif