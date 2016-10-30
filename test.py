import subprocess
import glob, os
import re

expected = {'Examples/test_while.c': 5, 'Examples/test_twice.c': 22,'Examples/test_cplus.c': 7,'Examples/test_fact.c': 24,'Examples/test_innerfunc.c': 6, 'Examples/test_if_else.c': 3, 'Examples/test_math.c': 14, 'Examples/test_simple.c': 8, 'Examples/test_function.c': 4, 'Examples/test_function_args.c': 6}
compiler_ignore = {'Examples/test_twice.c','Examples/test_cplus.c','Examples/test_fact.c'}
FNULL = open(os.devnull, 'w')

#Test all cases for intepret
for file in glob.glob("Examples/test*"):
    result = subprocess.call("./mycc -i " + file, shell=True, stdout=FNULL, stderr=subprocess.STDOUT)

    if(expected[file] != result):
        print("INTERPRET FAIL: " + file +" RESULT: " + str(result) + " EXPECTED: " + str(expected[file]))
        exit(1);

#Test all cases for compiler
for file in glob.glob("Examples/test*"):
    if file in compiler_ignore:
        continue;
    output = open('temp.txt', 'w');
    result = subprocess.call("./mycc -c " + file, shell=True, stdout=FNULL, stderr=subprocess.STDOUT)
    result = subprocess.call("spim -exception_file testExceptionHandler.s -file Output/test.asm", shell=True, stdout=output, stderr=subprocess.STDOUT)
    output.close();
    output = open('temp.txt', 'r');
    value = output.read().splitlines()[1];
    value = int(value);
    os.remove('temp.txt');

    if(expected[file] != value):
        print("COMPILER FAIL: " + file +" RESULT: " + str(value) + " EXPECTED: " + str(expected[file]))
        exit(1);


print("SUCCESS!!")
exit(0);
