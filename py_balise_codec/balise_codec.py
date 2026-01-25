# balise_codec.py: a python module to interface with the balise_codec library, part of https://github.com/FokkeB/subset36
#
# balise_codec is free software: you can distribute it and/or modify it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# See the GNU Lesser General Public License for more details.
# You should have received a copy of the GNU General Public License along with this program.
# If not, see < https://www.gnu.org/licenses/>.
#
# Usage example (see also test 3):
#
# from balise_codec import balise_codec 
# balise_c = balise_codec(path_to_library)
# result = balise_c.parse(test_params.test_line, max_cpu = 0, calc_all = True, unshaped_format = 'h', shaped_format='b')
# see the code below for more details of the parameters
#
# - "result" contains a list of dicts with fields {"unshaped", "shaped", "errcode"}
# - raises exceptions if something goes wrong
# - __version__ is set to the version of the c++ library
#
# thanks to Pierre (https://github.com/Pierretsc) for his input


import ctypes
from ctypes import c_char_p, c_uint, c_bool, c_int, Structure, POINTER, byref, c_char

class t_telegram (Structure):
    pass

# type definition of a telegram to be passed to the library:
p_t_telegram = POINTER(t_telegram)
t_telegram._fields_ = [#("line", c_char_p), 
                       ("unshaped", c_char_p),
                       ("shaped", c_char_p),
                       ("errcode", c_int),
                       ("next", p_t_telegram)
                      ]

class balise_codec:
    def __init__(self, path=""):
        """ constructor: load the module and initialise the interface with balise_codec
        : path: The path of the balise_codec-library (defaults to current working path)
        """
        print ("Loading lib", path)
         
        self.lib = ctypes.CDLL(path)   # will raise an error if lib could not be loaded
         
        # set the i/o parameters of get_lib_version and read the version:
        self.lib.get_lib_version.argtypes = []
        self.lib.get_lib_version.restype = c_char_p
        self.__version__ = self.lib.get_lib_version().decode("ascii")

        # set the i/o parameters of parse_line:
        self.lib.parse_line.argtypes = [
            c_char_p,    # input_line
            c_uint,      # max_cpu
            c_bool,      # calc_all
            POINTER(p_t_telegram), # pointer to pointer to first telegram
            c_char,      # format shaped
            c_char       # format unshaped
            ]
        self.lib.parse_line.restype = c_int

    def parse(self,
              input_line: str,
              max_cpu: int = 0,
              calc_all: bool = False,
              unshaped_format: str = 'h',
              shaped_format: str = 'b'):
        """
        Calls the C++ library to parse the telegram(s) in the input line (same actions as with the executable: shape/deshape/check).

        :param input_line: The raw telegram line; same format as in the original input files, may contain many telegrams, each on its own line
        :param max_cpu: 0 = auto, otherwise CPU/thread limit 
        :param calc_all: True = generate all variations, False = generate a single result
        :param unshaped_format: the format in which the unshaped string is returned ('h'=hex, 'b'=base64), defaults to hex
        :param shaped_format: the format in which the shaped string is returned ('h'=hex, 'b'=base64), defaults to base64
        :returns: list of parsed telegrams with fields {"unshaped", "shaped", "errcode"}
        """

        if not isinstance(input_line, str):
            raise TypeError("input_line must be a string")

        # p_telegram will point to the first telegram in the linked list
        p_telegram = p_t_telegram()
        rc = self.lib.parse_line(
            input_line.encode("ascii"),
            max_cpu,
            calc_all,
            byref(p_telegram),
            unshaped_format.encode('utf-8')[0],
            shaped_format.encode('utf-8')[0]
        )
        
        if rc == 0:
        # no error occurred, iterate over the telegrams, fill a list of dicts with the result and return it
#            count = 1
            telegrams = []
 
            while p_telegram:
#                print ("Telegram #", count, " @ ", hex(id(p_telegram)))
#                print ("Line =",p_telegram.contents.line) 
#                print ("Next =", hex(id(p_telegram.contents.next)))

                telegrams.append (
                    {"unshaped": p_telegram.contents.unshaped.decode("ascii"),
                     "shaped": p_telegram.contents.shaped.decode("ascii"),
                     "errcode": p_telegram.contents.errcode}
                    )

                p_telegram = p_telegram.contents.next
#                count += 1

            return telegrams

        # there was an error; raise an exception
        raise ValueError (rc)
