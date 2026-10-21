#ifndef SERVER_H
#define SERVER_H
#include <string>
#include <vector>
#include <queue>
 
struct ResultadoProcesoPy {
    std::string placa;
    int celdaId;
    std::string accion;
    std::string estadoParqueadero;
};
 
class ParqueaderoServidor {
private:
    struct EspacioParqueadero {
        int id;
        bool ocupado;
        std::string placaVehiculo;
    };
    std::vector<EspacioParqueadero> parqueadero;
    std::queue<std::string> colaEsperaPlacas;
 
public:
    ParqueaderoServidor();
    void inicializarParqueadero();
    ResultadoProcesoPy procesarPlacaEntrante(const std::string& placa);
    std::string obtenerEstadoParqueadero();
};
 
#endif