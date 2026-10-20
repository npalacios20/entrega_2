@echo off
echo [1/3] Ejecutando SWIG para generar los wrappers...
swig -c++ -python server.i

echo [2/3] Compilando el codigo C++ con g++...
g++ -std=c++17 -c server.cpp server_wrap.cxx -I"C:\Users\Martin\AppData\Local\Programs\Python\Python312\include"

echo [3/3] Creando la libreria dinamica para Python (.pyd)...
g++ -shared server.o server_wrap.o -L"C:\Users\Martin\AppData\Local\Programs\Python\Python312\libs" -lpython312 -lws2_32 -o _server_lib.pyd

pause