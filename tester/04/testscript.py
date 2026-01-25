# TEST04: test the "calc_all" function of the library 

# args: path_of_library_under_test

import os, sys, test_params

# show the location and parameters of this test script:
dir_path = os.path.dirname(os.path.realpath(__file__))
print ("Current working path:",dir_path)
for i, arg in enumerate(sys.argv):
    print(f"Argument {i:>6}: {arg}")

# Path to the module, relative to the path of the test script:
LIB_PATH = "/".join(sys.argv[0].split ("/")[0:-3])+"/py_balise_codec"
sys.path.append(LIB_PATH)

# import and instantiate the balise_codec library:
from balise_codec import balise_codec 
balise_c = balise_codec(sys.argv[1])
 
print ("Calculating all 474 variations of long input telegram with all 1's.")
res = balise_c.parse(test_params.test_line, calc_all = True, unshaped_format = 'x', shaped_format='x')
print ("#telegrams:", len(res))
if res != test_params.result:
    print ("Error: unexpected result.")

    for index, telegram in enumerate(res):
        print (telegram)
#        print (index,";",telegram["shaped"],";",test_params.result[index]["shaped"],sep="")
#        if res[index]["shaped"] != test_params.result[index]["shaped"]:
#            print("\n", index-1,"\n", res[index-1]["shaped"], "\n", test_params.result[index-1]["shaped"])
#            print(index,"\n", res[index]["shaped"], "\n", test_params.result[index]["shaped"])
#            print("\n", index+1,"\n", res[index+1]["shaped"], "\n", test_params.result[index+1]["shaped"])
#            sys.exit(1)
#    print ("Expected:", test_params.result)
#    print("\nFound:", res)
    result = 1
else:
    print ("Result is OK.")
    result = 0

print ("ready")
sys.exit(result)