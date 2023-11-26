IF NOT DEFINED WELSIM_LIBPACK (
call env_var.bat 
)

XCOPY x64\Debug\palace_d.exe %WELSIM_EXEC% /F /C /S /Y /I
XCOPY x64\Debug\palace_d.exe %WELSIM_LIBPACK%\bin\palace /F /C /S /Y /I



