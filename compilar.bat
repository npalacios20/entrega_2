@echo off
swig -c++ -python server.i
if errorlevel 1 goto error
 
g++ -std=c++17 -c server.cpp server_wrap.cxx -I"C:\Users\Martin\AppData\Local\Programs\Python\Python312\include"
if errorlevel 1 goto error
 
g++ -shared server.o server_wrap.o -L"C:\Users\Martin\AppData\Local\Programs\Python\Python312\libs" -lpython312 -o _server.pyd
if errorlevel 1 goto error
 
echo COMPILADO EXITOSAMENTE
goto fin
 
:error
echo ERROR EN COMPILACION
pause
 
:fin