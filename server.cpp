#include "server.h"
#include <iostream>
#include <algorithm>

#define TOTAL_ESPACIOS 6

ParqueaderoServidor::ParqueaderoServidor() {
    inicializarParqueadero();
}

void ParqueaderoServidor::inicializarParqueadero() {
    parqueadero.resize(TOTAL_ESPACIOS);
    for (int i = 0; i < TOTAL_ESPACIOS; ++i) {
        parqueadero[i] = {i + 1, false, ""};
    }
    while (!colaEsperaPlacas.empty()) {
        colaEsperaPlacas.pop();
    }
}

std::string ParqueaderoServidor::obtenerEstadoParqueadero() {
    std::string estado = "--- ESTADO ACTUAL DEL PARQUEADERO ---\n";
    int libres = 0;
    for (const auto& espacio : parqueadero) {
        estado += "Espacio [" + std::to_string(espacio.id) + "]: ";
        if (espacio.ocupado) {
            estado += "🔴 OCUPADO (Placa: " + espacio.placaVehiculo + ")\n";
        } else {
            estado += "🟢 LIBRE\n";
            libres++;
        }
    }
    estado += "📊 Espacios libres: " + std::to_string(libres) + "/" + std::to_string(TOTAL_ESPACIOS) + "\n";
    estado += "🚗 Vehículos en fila de espera FIFO: " + std::to_string(colaEsperaPlacas.size()) + "\n";
    return estado;
}

ResultadoProcesoPy ParqueaderoServidor::procesarPlacaEntrante(const std::string& placa) {
    ResultadoProcesoPy res;
    res.placa = placa;

    
    for (int i = 0; i < TOTAL_ESPACIOS; ++i) {
        if (parqueadero[i].ocupado && parqueadero[i].placaVehiculo == placa) {
            res.celdaId = parqueadero[i].id;
            res.accion = "SALIDA";
            
            
            parqueadero[i].ocupado = false;
            parqueadero[i].placaVehiculo = "";

            
            if (!colaEsperaPlacas.empty()) {
                std::string siguientePlaca = colaEsperaPlacas.front();
                colaEsperaPlacas.pop();

                parqueadero[i].ocupado = true;
                parqueadero[i].placaVehiculo = siguientePlaca;
                
            }

            res.estadoParqueadero = obtenerEstadoParqueadero();
            return res;
        }
    }

   
    for (int i = 0; i < TOTAL_ESPACIOS; ++i) {
        if (!parqueadero[i].ocupado) {
            parqueadero[i].ocupado = true;
            parqueadero[i].placaVehiculo = placa;

            res.celdaId = parqueadero[i].id;
            res.accion = "ENTRADA";
            res.estadoParqueadero = obtenerEstadoParqueadero();
            return res;
        }
    }

    
    colaEsperaPlacas.push(placa);
    res.celdaId = -1;
    res.accion = "EN_ESPERA";
    res.estadoParqueadero = obtenerEstadoParqueadero();
    return res;
}