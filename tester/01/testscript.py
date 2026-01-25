# TEST01: parse a file with illegal contents

import sys, subprocess, filecmp

# args: path_of_executable_under_test

import os 
dir_path = os.path.dirname(os.path.realpath(__file__))
print ("Current working path:",dir_path)
for i, arg in enumerate(sys.argv):
    print(f"Argument {i:>6}: {arg}")

command = sys.argv[1] + " -i "+dir_path+"/input.csv -v0 -o "+dir_path+"/testresult.txt"
print ("command = ", command)

output = subprocess.run (command.split(), capture_output=True)
print ("output=", output)

if output.returncode != 5:
    print ("Returncode fails, expected 5 but found", output.returncode)
    sys.exit(1)
elif output.stdout != b'':
    print ("Stdout fails, expected empty but found", output.stdout)
    sys.exit(2)
elif not filecmp.cmp (dir_path+"/testresult.csv", dir_path+"/expected_result.csv"):
    print ("Expected content not found.")
    sys.exit(3)
else:
    print ("Test passed.")
    sys.exit (0)
