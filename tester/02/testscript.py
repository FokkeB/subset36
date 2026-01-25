# TEST02: test the command line parameters of the executable

import sys, subprocess, filecmp

# args: path_of_executable_under_test

import os 
dir_path = os.path.dirname(os.path.realpath(__file__))
print ("Current working path:",dir_path)
for i, arg in enumerate(sys.argv):
    print(f"Argument {i:>6}: {arg}")

testinputs = {
    # OK line:
    "-s OxhnN3kttOo1G/sfwmUneZD25Xpzrx1WvyEdMLZAQ0epMYTrRkbT2VPo0A==":"FABC456091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0;OxhnN3kttOo1G/sfwmUneZD25Xpzrx1WvyEdMLZAQ0epMYTrRkbT2VPo0A==;0",
    # Missing first char:
    "-s xhnN3kttOo1G/sfwmUneZD25Xpzrx1WvyEdMLZAQ0epMYTrRkbT2VPo0A==":"xhnN3kttOo1G/sfwmUneZD25Xpzrx1WvyEdMLZAQ0epMYTrRkbT2VPo0A==;;5",
    # test make_long:
    "-l -s FABC456091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0": "FABC456091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC;SzC+qd8aKH1SiVrnlmYE5+CNfu1aPioBteDezE+kcLmmmCn42LDX9eTtWeq9rfgiLv5KRQRge8JVu9JhjvPQjIzQ7VnSaJAy3eiqgJTzFQ54pOq1L7wR9Y53kaKprbrh5zPn1EmOzLZspMc8FBvl8dO4kBwRG0qbk0xmtc75hw4=;0",
    # input base64, output hex:
    "-f hex -s OxhnN3kttOo1G/sfwmUneZD25Xpzrx1WvyEdMLZAQ0epMYTrRkbT2VPo0A==":"FABC456091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0;3B186737792DB4EA351BFB1FC265277990F6E57A73AF1D56BF211D30B6404347A93184EB4646D3D953E8D0;0",
    # input shaped data in hex:
    "-s 3B186737792DB4EA351BFB1FC265277990F6E57A73AF1D56BF211D30B6404347A93184EB4646D3D953E8D0": "FABC456091DEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC0;OxhnN3kttOo1G/sfwmUneZD25Xpzrx1WvyEdMLZAQ0epMYTrRkbT2VPo0A==;0"
    }

errcount = 0
count = 1

# test the above inputs:
for inputstring, outputstring in testinputs.items():
    command = sys.argv[1] + " -v0 " + inputstring
    print ("\nTest #", count, "   Command =", command)

    output = subprocess.run (command.split(), capture_output=True, text=True)
    output = output.stdout.split ()[1].strip()  # remove first line with header, remove CR's/LF's
    print ("output =", output)
    
    if output != outputstring:
        print ("Error. Expected output:", outputstring, "Found output:", output)
        errcount += 1
    else:
        print ("Result OK")

    count += 1

command = sys.argv[1] + " -v0 -a -s FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC" 
output = subprocess.run (command.split(), capture_output=True, text=True)
output = output.stdout.split();
print ("output =", output)

sys.exit(errcount)
