echo OFF
REM
REM	$Id: do_examples.bat,v 1.1.1.1 2000-12-28 01:23:45 gmt Exp $
REM
REM DOS batch script to run all GMT examples (DOS versions).

echo Loop over all 20 examples and run each job

for %%d in (01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20) do call ex%%d\job%%d

echo "Completed all examples"
echo ON
