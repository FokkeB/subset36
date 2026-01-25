# TEST03: test the library

# args: path_of_library_under_test

import sys, os

lib_version_under_test = "2.1"


# show the location and parameters of this test script:
dir_path = os.path.dirname(os.path.realpath(__file__))
print ("Current working path:",dir_path)
for i, arg in enumerate(sys.argv):
    print(f"Argument {i:>6}: {arg}")

# Path to the module, relative to the path of the test script:
LIB_PATH = "/".join(sys.argv[0].split ("/")[0:-3])+"/py_balise_codec"
sys.path.append(LIB_PATH)
print (sys.path)
print ("lib: ",LIB_PATH)
# import and instantiate the balise_codec library:
from balise_codec import balise_codec 
balise_c = balise_codec(sys.argv[1])
print ("lib initialised", sys.argv[1])

# test inputs to use in this test:

testinputs = {
    # OK line with only unshaped data in Base64:
    "OxhnN3kttOo1G/sfwmUneZD25Xpzrx1WvyEdMLZAQ0epMYTrRkbT2VPo0A==":[{'unshaped': 'FABC456091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0', 'shaped': 'OxhnN3kttOo1G/sfwmUneZD25Xpzrx1WvyEdMLZAQ0epMYTrRkbT2VPo0A==', 'errcode': 0}],
    # OK line with only shaped data in hex:
    "1B906971A32D79C0CC2F92727BC739FACD2839BC722C4A451E9CD2FCA240521110DE06C1313EA732EBAF80":[{'unshaped': 'FAB0556091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0', 'shaped': 'G5BpcaMtecDML5Jye8c5+s0oObxyLEpFHpzS/KJAUhEQ3gbBMT6nMuuvgA==', 'errcode': 0}],
    # Mismatch shaped-deshaped:
    "OxhnN3kttOo1G/sfwmUneZD25Xpzrx1WvyEdMLZAQ0epMYTrRkbT2VPo0A==;EABC456091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0": [{'unshaped': 'EABC456091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0', 'shaped': 'OxhnN3kttOo1G/sfwmUneZD25Xpzrx1WvyEdMLZAQ0epMYTrRkbT2VPo0A==', 'errcode': 18}],
    # Missing first char:
    "xhnN3kttOo1G/sfwmUneZD25Xpzrx1WvyEdMLZAQ0epMYTrRkbT2VPo0A==":[{'unshaped': 'xhnN3kttOo1G/sfwmUneZD25Xpzrx1WvyEdMLZAQ0epMYTrRkbT2VPo0A==', 'shaped': '<input err>', 'errcode': 5}],
    # input shaped data in hex:
    "3B186737792DB4EA351BFB1FC265277990F6E57A73AF1D56BF211D30B6404347A93184EB4646D3D953E8D0": [{'unshaped': 'FABC456091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0', 'shaped': 'OxhnN3kttOo1G/sfwmUneZD25Xpzrx1WvyEdMLZAQ0epMYTrRkbT2VPo0A==', 'errcode': 0}],
    # multiple lines with short telegrams:
"""
FAB0156091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0
FAB0256091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0
FAB0356091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0
FAB0456091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0
FAB0556091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0
""":
[{'unshaped': 'FAB0156091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0', 'shaped': 'rPS/nm+o5urFX7Lkr/VJ+Zt4nfnixl3Na7XOsypA9X7QXLr3cnl2KPvICA==', 'errcode': 0}, {'unshaped': 'FAB0256091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0', 'shaped': 'Rj6o6PGZKxSXiQ+xOFbITraciNh4e9hZujciYchAQ3eCRkhCLYweyeS58A==', 'errcode': 0}, {'unshaped': 'FAB0356091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0', 'shaped': 'CvA7kuG+2X7JEIwWBI/ZGypxRANgVw/OJRMpKHxARH5zylNC6Zk0kBS3KA==', 'errcode': 0}, {'unshaped': 'FAB0456091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0', 'shaped': '9Mmr0Xsn+m22NWWBs63+EJErPzB4Xa1yrbs0NNJAVNtqz8XDtofCX6NQwA==', 'errcode': 0}, {'unshaped': 'FAB0556091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0', 'shaped': 'G5BpcaMtecDML5Jye8c5+s0oObxyLEpFHpzS/KJAUhEQ3gbBMT6nMuuvgA==', 'errcode': 0}],
    # multiple lines with long telegrams:
"""
1FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC
2FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC
3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC
4FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC
5FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC
""":
[{'unshaped': '1FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC', 'shaped': 'FKO/NiVY6FCCXr1GGGk4Pz8Ghp9ubJ8Myskibksa1JEQJd7hTfshJRyp1OIZvqDK1kipguO3WpMklSjvqItGspYV8Fum+KPpSqUV4nDSm5XgqZDNAW067esuWKheL3C3EzGrxFr5+pWx/QLwqy6WNb5dkBEDaWAszbO+tJoq/3w=', 'errcode': 0}, {'unshaped': '2FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC', 'shaped': 'DkhU888JmuOvGY6KSDk2ut9YVmrdmNN6V4yw0s0lF8mnnxWkSo0Ay2sbvRc9j7R1t30g5lTbvnVOR3X73w1gYimxDuEEtATyDVDUehhJ0VH8zv44aQzBWbMpkq3mxUntR3hOtkHvEzd8arDgO4Nwx0hmkBEn4qCYYycyJjVf6J4=', 'errcode': 0}, {'unshaped': '3FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC', 'shaped': 'ThlbGigcEWScMZZEcYqD+kvhmml14g70Q8MhMMO6rOLxo0RWBB5YJR0yicEp8W8CeLHCZaPWaSaBnStZrJI/F3MuxLjHTNj9Haa20JBUurAihO2eL2g0T85FxNhI7DZiAqPxV6Q9EtzStpPPMz10ts41kDCPaQYfQkIVTskWc/w=', 'errcode': 0}, {'unshaped': '4FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC', 'shaped': 'Ol5TVZqLYaTyblIZp/k7sB7Tu8PaVlIWydcWys1PQuuGmGEGYWGTp4L9poRqjvkg7vbgLzcik7J1Ss5k8clvSN6s8ph0JklQuJmRK3dyawXGh9LUq01elQ1mutSVQLl/rZN3UlW213nH2c+DC/2d4kxJkBHHq7eGm/dIcCPI3mQ=', 'errcode': 0}, {'unshaped': '5FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC', 'shaped': 'da5QJQzdJtv3iM1/OJlKKS1jlY1Jsoo1FjloHp3JZ4U6F7VNEyrHMM8go+S7nsI72q9OH7gd5h4Vm6LHRSEevSkVCk2Tl7QbAwFcgE6oaBGZOQisk+5uAZ3w+Ek35emrBJcuD7QqeId197wzOhtxXvZ8EBFhRA8gXZ4J2j2fVxY=', 'errcode': 0}]

}   


def check_exc (inputstr, exp):
# tries to trigger an exception with inputstr, compares with the expected result
    try:
        balise_c.parse(inputstr)
    
    except Exception as err:
        if (err.args[0] == exp):
            print ("OK, exception", exp)
            return 0
        else:    
            print ("Unexpected exception", exp)
            return 1
    else:
        print ("Err: no exception occurred")
        return 1

# perform the tests:
print ("Checking balise_codec python library version:", balise_c.__version__ )
print ("Expected library version:", lib_version_under_test)
if '.'.split(lib_version_under_test)[0] == '.'.split(balise_c.__version__)[0]:
    print ("Version OK")
    errcount = 0
else:
    print ("Version NOT OK")
    errcount = 1

# iterate over the input strings and compare the expected result with the outcome of the call to the balise_codec module:
for inputstring, outputstring in testinputs.items():
    print ("\nParsing string: ", inputstring)
    res = balise_c.parse(inputstring)
    print("Result :", res)
    
    if res != outputstring:
        print ("Error. Expected output:", outputstring, "\nFound output:", res)
        errcount += 1
    else:
        print ("Result OK")
    
# trigger some exceptions:
print ("\nChecking exception 1 (empty inputs):")
errcount += check_exc ("", 1)
errcount += check_exc ("  # comments \n", 1)
errcount += check_exc ("\n\n\n", 1)

# end conclusion:
if (not errcount):
    print ("\nAll tests OK")
else:
    print ("\nEnd result:", errcount, "error(s) found.")

sys.exit(errcount)
