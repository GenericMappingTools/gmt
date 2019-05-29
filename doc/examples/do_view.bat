echo OFF
REM
REM
REM DOS batch script to view all GMT examples.
REM Assumes gsview32 is in the path and that
REM all examples have been run with do_examples

echo Loop over all examples and view each plot

for /r %%i in (*.ps) do gsview32 %%i

echo "Completed viewing all examples"
echo ON
