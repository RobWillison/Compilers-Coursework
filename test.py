import subprocess
import glob, os
import re

expected = {'Examples/test_if_else.c': 8, 'Examples/test_math.c': 14, 'Examples/test_simple.c': 8}
FNULL = open(os.devnull, 'w')

#Test all cases
for file in glob.glob("Examples/test*"):
    result = subprocess.call("./mycc " + file, shell=True, stdout=FNULL, stderr=subprocess.STDOUT)

    if(expected[file] != result):
        print("FAIL: " + file +" RESULT: " + str(result) + " EXPECTED: " + str(expected))
        exit(1)



exit(0);
