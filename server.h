#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <vector>
#include <queue>

struct ResultadoProcesoPy {
    std::string placa;
    int celdaId;         // 1 al 6, o -1 si queda en espera
    std::string accion;  // "ENTRADA", "SALIDA", "EN_ESPERA"
    std::string estadoParqueadero;
};

class ParqueaderoServidor {
private:
    // Estructuras internas de tu lógica original protegidas en la clase
    struct EspacioParqueadero {
        int id;
        bool ocupado;
        std::string placaVehiculo;
    };

    std::vector<EspacioParqueadero> parqueadero;
    std::queue<std::string> colaEsperaPlacas; // Fila FIFO pura de placas

public:
    ParqueaderoServidor();
    void inicializarParqueadero();
    ResultadoProcesoPy procesarPlacaEntrante(const std::string& placa);
    std::string obtenerEstadoParqueadero();
};

#endif