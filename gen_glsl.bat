@ECHO off
SET PATH=%PATH%;%~dp0\3rd\xxd
cd glsl
set res_name=glsl.h
for /f %%f in ('dir /b .\*.frag') do	(
	xxd.exe -i %%f >> %res_name%
	echo compile %%f
)
for /f %%f in ('dir /b .\*.vs') do	(
	xxd.exe -i %%f  >> %res_name%
	echo compile %%f
)
del ..\include\glsl\%res_name%
xcopy %res_name% ..\include\glsl\
del %res_name%