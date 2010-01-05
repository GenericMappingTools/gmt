echo OFF
REM
REM	$Id: do_examples.bat,v 1.9 2010-01-05 03:49:47 guru Exp $
REM
REM DOS batch script to run all GMT examples (DOS versions).

echo Loop over all 30 examples and run each job

for %%d in (01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30) do call ex%%d\job%%d

echo "Completed all examples"
echo ON
