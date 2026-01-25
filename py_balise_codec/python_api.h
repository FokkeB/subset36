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

const char LIB_VERSION[] = "2.0";     // version of the library. Major version nr will increase if the new version is not backward compatible, minor version will increase if it remains backwards compatible

// define the EXPORT literal (add __declspec to the header if Windows)
#if defined(_WIN32) || defined(_WIN64)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

// one chain in a linked list of python telegrams:
struct py_telegram_t
{
    char* unshaped;             // unshaped data in the specified format
    char* shaped;               // shaped data in the specified format
    int errcode;                // code of the error that occurred during calculation (0 means no error)
    py_telegram_t* next;        // pointer to the next telegram in the linked list
};

extern "C"
{
    EXPORT const char* get_lib_version(void);

    EXPORT int parse_line(const char* input_line,                   // the input line that shall be parsed
                          const unsigned int max_cpu,               // max amount of cpu's to use (0=auto)
                          const bool calc_all,                      // calculate all possible versions of the telegram(s)?
                          const py_telegram_t** py_telegram,        // pointer to a pointer to the first telegram in the output linked list of py_telegrams
                          const char unshaped_format,               // format of the unshaped output ('h'=hex, 'b'=base64), defaults to 'h'
                          const char shaped_format                  // format of the shaped output ('h'=hex, 'b'=base64), defaults to 'b'
                         );
}