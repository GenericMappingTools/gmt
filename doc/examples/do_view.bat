echo OFF
REM
REM	$Id$
REM
REM DOS batch script to view all GMT examples.
REM Assumes gsview32 is in the path and that
REM all examples have been run with do_examples

echo Loop over all 30 examples and view each plot

for %%d in (01 02 03 03a 03b 03c 03d 03e 03f 04 04c 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30) do gsview32 example_%%d.ps

echo "Completed viewing all examples"
echo ON
