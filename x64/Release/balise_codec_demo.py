# Demo code to show the working of the python module "balise_codec"
# By fokke@bronsema.net, February 2024. Compatible with module version 4, February 2024. Published on: https://github.com/FokkeB/subset36.
# Bugs, requests? Please contact me.
#
# The "balise_codec"-module receives unshaped and/or shaped balise data from Python in a string of lines. 
# Each line is terminated by a newline (\n)-character and contains one of the following fields, or both (separated by a semicolon):
# - unshaped hex data (output of balise design process)
# - shaped data (either hex or base64, either short or long)
# If only one field is given, the module will calculate the other field. 
# If both input fields are given, the module will verify the correctness (is shaped data correct? do the two fields match? etc).
# Comments are preceded with #
# The module returns another string with the following lines:
# <unshaped hex>;<shaped hex/base64 short/long>;<errorcode>\n
# The definition of the error codes is given in "telegram.h"

# import the convert function from balise_codec
import balise_codec

print ("Information about the module:")
print (balise_codec.__doc__)

from balise_codec import convert

print ("Information about the function 'convert':")
help(convert)

# read input from file (which is formatted as described above)
with open(r'Z:\Fokke\Nerdsels\Projecten\ss36\testset\dummy_input.csv', 'r') as file:
    data = file.read()

# convert the data and print it
result = convert (data)
print ("Converted data:")
print (result)

# parse the data:
balise_data = []
for line in result.split ('\n'):
    balise_data.append (line.split(';'))

print ("Parsed data:")
print (balise_data)

# etc...