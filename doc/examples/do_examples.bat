REM
REM DOS batch script to run all GMT examples (DOS versions).
REM

REM Set GMT_END_SHOW to off to disable automatic display of the plots
REM setx GMT_END_SHOW off

echo OFF
echo Loop over all examples and run each job

for /D %%i in (ex*) do (
    echo Running example %%i
    cd %%i
    call %%i.bat
    cd ..
)

echo Completed all examples
echo ON
