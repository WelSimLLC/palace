IF NOT DEFINED WELSIM_LIBPACK (
call env_var.bat 
)

XCOPY x64\Release\libpalace.lib %WELSIM_LIBPACK%\lib\palace /F /C /S /Y /I
