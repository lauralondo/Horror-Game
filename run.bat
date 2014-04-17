@echo off

rem -- rem is a comment


rem -- compile the .c file
lcc %1


rem -- save the name w/out .c extension
set prog=%~n1


rem -- link the stuff
lcclnk %prog%.obj opengl32.lib glu32.lib glut32.lib


rem -- remove the .obj file. we dont need it anymore
rm %prog%.obj


rem -- loop gets all parameters after param 1
set after1=
:loop
if "%2" == "" goto end
set after1=%after1% %2
shift
goto loop
:end


rem -- run the program with the passed in parameters
%prog% %after1%