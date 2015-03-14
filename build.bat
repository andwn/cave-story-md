@ECHO OFF

REM Running as admin changes the current directory to system32
REM These two lines bring us back to the dir we want
SETLOCAL ENABLEEXTENSIONS
CD /d "%~dp0"

IF "%GDK%" == "" GOTO NOPATH
IF "%GDK_WIN%" == "" GOTO NOPATH

:YESPATH
SET PATH=%GDK_WIN%\bin;%PATH%
make -f makefile.gen
GOTO END

:NOPATH
@ECHO The GDK or GDK_WIN environment variables are not set.
@ECHO You might need to run this as Administrator.
GOTO END

:END
pause
