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

// useful_functions.c
// contains some useful functions that have no place elsewhere

#include "useful_functions.h"
#include <string.h>
#include "telegram.h"

void print_hex (int v, unsigned char *bin, unsigned int n)
// prints the n binvalues in bin in hex-format, adds spaces to increase readability
// uses verbosity v
{
    unsigned int i;

    return_if_silent(v);

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

    return_if_silent(v);

    if ((n <= 0) || (n > sizeof(printme)*8))
        n = sizeof(printme);

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

unsigned int hex_to_bin(string hexstr, uint8_t* binstr)
// converts the contents of a string (ending with \0) with hex-data to the binary values
// returns the amount of bytes in binstr
{
    unsigned int w = 0;

    for (int i = 0; i < hexstr.length(); i++)
    {
        if (i % 2 == 0)
        {
            w = i / 2;
            binstr[w] = 0;
        }
        else
            binstr[w] <<= 4;

        if (hexstr[i] <= '9')
            binstr[w] |= (hexstr[i] - '0');
        else if (hexstr[i] >= 'a' && hexstr[i] <= 'f')
            binstr[w] |= hexstr[i] - 'a' + 10;
        else if (hexstr[i] >= 'A' && hexstr[i] <= 'F')
            binstr[w] |= hexstr[i] - 'A' + 10;
    }

    return w + 1;
}


//Base64 char table - used internally for encoding
unsigned char b64_chr[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

unsigned int b64_int(unsigned int ch) {

	// ASCII to base64_int
	// 65-90  Upper Case  >>  0-25
	// 97-122 Lower Case  >>  26-51
	// 48-57  Numbers     >>  52-61
	// 43     Plus (+)    >>  62
	// 47     Slash (/)   >>  63
	// 61     Equal (=)   >>  64~
	if (ch == 43)
		return 62;
	if (ch == 47)
		return 63;
	if (ch == 61)
		return 64;
	if ((ch > 47) && (ch < 58))
		return ch + 4;
	if ((ch > 64) && (ch < 91))
		return ch - 'A';
	if ((ch > 96) && (ch < 123))
		return (ch - 'a') + 26;
	return 0;
}

unsigned int b64_encode(const uint8_t* in, unsigned int in_len, string& outstr) {

	unsigned int i = 0, j = 0, k = 0, s[3] = { 0 };
	char out[MAX_ARRAY_SIZE] = { 0 };

	for (i = 0; i < in_len; i++) {
		s[j++] = *(in + i);
		if (j == 3) {
			out[k + 0] = b64_chr[(s[0] & 255) >> 2];
			out[k + 1] = b64_chr[((s[0] & 0x03) << 4) + ((s[1] & 0xF0) >> 4)];
			out[k + 2] = b64_chr[((s[1] & 0x0F) << 2) + ((s[2] & 0xC0) >> 6)];
			out[k + 3] = b64_chr[s[2] & 0x3F];
			j = 0; k += 4;
		}
	}

	if (j) {
		if (j == 1)
			s[1] = 0;
		out[k + 0] = b64_chr[(s[0] & 255) >> 2];
		out[k + 1] = b64_chr[((s[0] & 0x03) << 4) + ((s[1] & 0xF0) >> 4)];
		if (j == 2)
			out[k + 2] = b64_chr[((s[1] & 0x0F) << 2)];
		else
			out[k + 2] = '=';
		out[k + 3] = '=';
		k += 4;
	}

	outstr = out;

	return k;
}

unsigned int b64_decode(string in, uint8_t* out) {

	unsigned int i = 0, j = 0, k = 0, s[4] = { 0 };

	for (i = 0; i < in.length(); i++) {
		s[j++] = b64_int(in[i]);
		if (j == 4) {
			out[k + 0] = ((s[0] & 255) << 2) + ((s[1] & 0x30) >> 4);
			if (s[2] != 64) {
				out[k + 1] = ((s[1] & 0x0F) << 4) + ((s[2] & 0x3C) >> 2);
				if ((s[3] != 64)) {
					out[k + 2] = ((s[2] & 0x03) << 6) + (s[3]); k += 3;
				}
				else {
					k += 2;
				}
			}
			else {
				k += 1;
			}
			j = 0;
		}
	}

	return k;
}