@ECHO off
SET PATH=%PATH%;%~dp0\3rd\xxd
cd glsl
for /f %%f in ('dir /b .\*.glsl') do	(
	xxd.exe -i %%f %%~nf.h
	echo compile %%f
)
del "..\include\glsl\*.h"
type *.h > ..\include\glsl\glsl.h
del "*.h"