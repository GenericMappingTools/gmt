echo OFF
REM
REM	$Id: do_examples.bat,v 1.4 2004-09-29 01:29:02 pwessel Exp $
REM
REM DOS batch script to run all GMT examples (DOS versions).

echo Loop over all 25 examples and run each job

for %%d in (01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25) do call ex%%d\job%%d

echo "Completed all examples"
echo ON
