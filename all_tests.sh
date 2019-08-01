#!/bin/bash

./main < test_suites/suite1/batch1.1.in > output.txt
diff output.txt test_suites/suite1/batch1.1.py.out
./main < test_suites/suite1/batch1.2.in > output.txt
diff output.txt test_suites/suite1/batch1.2.py.out
./main < test_suites/suite2/batch2.1.in > output.txt
diff output.txt test_suites/suite2/batch2.1.py.out
./main < test_suites/suite2/batch2.2.in > output.txt
diff output.txt test_suites/suite2/batch2.2.py.out
./main < test_suites/suite3/batch3.1.in > output.txt
diff output.txt test_suites/suite3/batch3.1.py.out
./main < test_suites/suite3/batch3.2.in > output.txt
diff output.txt test_suites/suite3/batch3.2.py.out
./main < test_suites/suite4/batch4.1.in > output.txt
diff output.txt test_suites/suite4/batch4.1.py.out
./main < test_suites/suite4/batch4.2.in > output.txt
diff output.txt test_suites/suite4/batch4.2.py.out
./main < test_suites/suite5/batch5.1.in > output.txt
diff output.txt test_suites/suite5/batch5.1.py.out
./main < test_suites/suite5/batch5.2.in > output.txt
diff output.txt test_suites/suite5/batch5.2.py.out
./main < test_suites/suite6/batch6.1.in > output.txt
diff output.txt test_suites/suite6/batch6.1.py.out
./main < test_suites/suite6/batch6.2.in > output.txt
diff output.txt test_suites/suite6/batch6.2.py.out


