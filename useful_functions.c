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

// useful_functions.c
// contains some useful functions that have no place elsewhere

#include "useful_functions.h"

void print_hex (int v, unsigned char *bin, unsigned int n)
// prints the n binvalues in bin in hex-format, adds spaces to increase readability
// uses verbosity v
{
    int i;

    for (i=0; i<n; i++)
    {
        eprintf (v, "%02X", bin[i]);

        if ((i+1)%2==0)    
            eprintf(v, " ");
    }        
}

void print_bin (int v, uint64_t printme, int n)
// prints out the first n bits of the value in printme, adds spaces to increase readability
// uses verbosity v
{
    int i;

    if ((n <= 0) || (n > sizeof(printme)*8))
        n= sizeof(printme);

    for (i=n-1; i>=0; i--)  
    {
        if (printme & (1ULL<<i))
            eprintf(v, "1");
        else
            eprintf(v, "0");

        if (i%4==0)    
            eprintf(v, " ");
    }
}
