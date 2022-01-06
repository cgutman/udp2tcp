@echo off
setlocal enableDelayedExpansion

rem Run from VS command prompt

rem Build release binaries
msbuild u2t.sln /verbosity:minimal /t:Clean;Rebuild /property:Configuration=Release /property:Platform=x86
if !ERRORLEVEL! NEQ 0 goto Error
msbuild u2t.sln /verbosity:minimal /t:Clean;Rebuild /property:Configuration=Release /property:Platform=x64
if !ERRORLEVEL! NEQ 0 goto Error
msbuild u2t.sln /verbosity:minimal /t:Clean;Rebuild /property:Configuration=Release /property:Platform=ARM64
if !ERRORLEVEL! NEQ 0 goto Error

rem Sign the binaries
set FILES_TO_SIGN=
for /r "%CD%" %%f in (Release\*.exe) do (
    set FILES_TO_SIGN=!FILES_TO_SIGN! %%f
)
signtool sign /tr http://timestamp.digicert.com /td sha256 /fd sha256 /sha1 b28642b756ebec4884d1063dfa4de803a6dcecdc /v !FILES_TO_SIGN!
if !ERRORLEVEL! NEQ 0 goto Error

rem Zip the binaries
del u2t-*.zip
if !ERRORLEVEL! NEQ 0 goto Error
7z a u2t-x86.zip .\Release\*.exe .\Release\*.pdb
if !ERRORLEVEL! NEQ 0 goto Error
7z a u2t-x64.zip .\x64\Release\*.exe .\x64\Release\*.pdb
if !ERRORLEVEL! NEQ 0 goto Error
7z a u2t-arm64.zip .\ARM64\Release\*.exe .\ARM64\Release\*.pdb
if !ERRORLEVEL! NEQ 0 goto Error

echo Build successful!
exit /b 0

:Error
echo Build failed!
exit /b !ERRORLEVEL!