echo OFF
REM
REM	$Id$
REM
REM DOS batch script to run all GMT examples (DOS versions).

echo Loop over all examples and run each job

for %%d in (01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45) do (
	cd ex%%d
	call example_%%d
	cd ..
)

echo "Completed all examples"
echo ON
