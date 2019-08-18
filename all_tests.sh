#!/bin/bash

echo "Starting first test"
./main < test_suites/suite1/batch1.1.in > output.txt
diff output.txt test_suites/suite1/batch1.1.py.out
echo "Starting second test"
./main < test_suites/suite1/batch1.2.in > output.txt
diff output.txt test_suites/suite1/batch1.2.py.out
echo "Starting third test"
./main < test_suites/suite2/batch2.1.in > output.txt
diff output.txt test_suites/suite2/batch2.1.py.out
echo "Starting fourth test"
./main < test_suites/suite2/batch2.2.in > output.txt
diff output.txt test_suites/suite2/batch2.2.py.out
echo "Starting fifth test"
./main < test_suites/suite3/batch3.1.in > output.txt
diff output.txt test_suites/suite3/batch3.1.py.out
echo "Starting sixth test"
./main < test_suites/suite3/batch3.2.in > output.txt
diff output.txt test_suites/suite3/batch3.2.py.out
echo "Starting seventh test"
./main < test_suites/suite4/batch4.1.in > output.txt
diff output.txt test_suites/suite4/batch4.1.py.out
echo "Starting eigth test"
./main < test_suites/suite4/batch4.2.in > output.txt
diff output.txt test_suites/suite4/batch4.2.py.out
echo "Starting ninth test"
./main < test_suites/suite5/batch5.1.in > output.txt
diff output.txt test_suites/suite5/batch5.1.py.out
echo "Starting tenth test"
./main < test_suites/suite5/batch5.2.in > output.txt
diff output.txt test_suites/suite5/batch5.2.py.out
echo "Starting eleventh test"
./main < test_suites/suite6/batch6.1.in > output.txt
diff output.txt test_suites/suite6/batch6.1.py.out
echo "Starting twelfth test"
./main < test_suites/suite6/batch6.2.in > output.txt
diff output.txt test_suites/suite6/batch6.2.py.out


