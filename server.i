%module server_lib

%{
#include "server.h"
%}

%include "std_string.i"
%include "std_vector.i"

struct ResultadoProcesoPy {
    int celdaId;
    std::string placa;
    std::string accion;
    std::string hora;
    std::string estadoParqueadero;
};

%include "server.h"