@echo off&setlocal
::
:: From http://www.commandline.co.uk/cmdfuncs/dandt/index.html
::
call :GetDate year month day
call :MonthName %month% monthname
echo/%year% %month% %day% %monthname%
goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:GetDate yy mm dd
::
:: By:   Ritchie Lawrence, 2002-06-15. Version 1.0
::
:: Func: Loads local system date components into args 1 to 3. For NT4/2K/XP
::
:: Args: %1 var to receive year, 4 digits (by ref)
::       %2 var to receive month, 2 digits, 01 to 12 (by ref)
::       %3 Var to receive day of month, 2 digits, 01 to 31 (by ref)
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
setlocal ENABLEEXTENSIONS
set t=2&if "%date%z" LSS "A" set t=1
for /f "skip=1 tokens=2-4 delims=(-)" %%a in ('echo/^|date') do (
  for /f "tokens=%t%-4 delims=.-/ " %%d in ('date/t') do (
    set %%a=%%d&set %%b=%%e&set %%c=%%f))
endlocal&set %1=%yy%&set %2=%mm%&set %3=%dd%&goto :EOF
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:MonthName %mm% month
::
:: By:   Ritchie Lawrence, 2002-10-04. Version 1.0
::
:: Func: Returns the name of month from the number of the month.
::       For NT4/2K/XP
::
:: Args: %1 month number convert to name of month, 1 or 01 to 12 (by val)
::       %2 var to receive name of month (by ref)
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
setlocal ENABLEEXTENSIONS&set /a m=100%1%%100
for /f "tokens=%m%" %%a in ('echo/January February March April May June^
  July August September October November December'
) do endlocal&set %2=%%a&goto :EOF
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

