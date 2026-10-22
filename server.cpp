#include "server.h"
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <chrono>
<<<<<<< HEAD
 
=======
>>>>>>> 40fa55daef5a32fc0f2db735b0c45eb061e65e88
#define TOTAL_ESPACIOS 18
 
ParqueaderoServidor::ParqueaderoServidor() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
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
            estado += "OCUPADO (Placa: " + espacio.placaVehiculo + ")\n";
        } else {
            estado += "LIBRE\n";
            libres++;
        }
    }
    estado += "ESPACIOS_LIBRES:" + std::to_string(libres) + "/" + std::to_string(TOTAL_ESPACIOS) + "\n";
    estado += "EN_FILA:" + std::to_string(colaEsperaPlacas.size()) + "\n";
    return estado;
}
 
ResultadoProcesoPy ParqueaderoServidor::procesarPlacaEntrante(const std::string& placa) {
    ResultadoProcesoPy res;
    res.placa = placa;
    std::string eventosExtra = "";
 
    static auto ultimaSalidaAutomatica = std::chrono::steady_clock::now();
    auto ahora = std::chrono::steady_clock::now();
    auto segundosTranscurridos = std::chrono::duration_cast<std::chrono::seconds>(ahora - ultimaSalidaAutomatica).count();
 
    while (segundosTranscurridos >= 12) {
        std::vector<int> indicesOcupados;
        for (int i = 0; i < TOTAL_ESPACIOS; ++i) {
            if (parqueadero[i].ocupado) {
                indicesOcupados.push_back(i);
            }
        }
        if (!indicesOcupados.empty()) {
            int idxAuto = indicesOcupados[std::rand() % indicesOcupados.size()];
            std::string placaSalida = parqueadero[idxAuto].placaVehiculo;
            int idCelda = parqueadero[idxAuto].id;
            parqueadero[idxAuto].ocupado = false;
            parqueadero[idxAuto].placaVehiculo = "";
            eventosExtra += "Placa: " + placaSalida + " | Accion: SALIDA_AUTO | Celda ID: " + std::to_string(idCelda) + "\n";
            if (!colaEsperaPlacas.empty()) {
                std::string siguientePlaca = colaEsperaPlacas.front();
                colaEsperaPlacas.pop();
                parqueadero[idxAuto].ocupado = true;
                parqueadero[idxAuto].placaVehiculo = siguientePlaca;
                eventosExtra += "Placa: " + siguientePlaca + " | Accion: ENTRADA_FIFO | Celda ID: " + std::to_string(idCelda) + "\n";
            }
        }
        ultimaSalidaAutomatica += std::chrono::seconds(12);
        segundosTranscurridos -= 12;
    }
 
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
                eventosExtra += "Placa: " + siguientePlaca + " | Accion: ENTRADA_FIFO | Celda ID: " + std::to_string(res.celdaId) + "\n";
            }
            res.estadoParqueadero = eventosExtra + obtenerEstadoParqueadero();
            return res;
        }
    }
 
    std::vector<int> indicesLibres;
    for (int i = 0; i < TOTAL_ESPACIOS; ++i) {
        if (!parqueadero[i].ocupado) {
            indicesLibres.push_back(i);
        }
    }
    if (!indicesLibres.empty()) {
        int indiceAleatorio = indicesLibres[std::rand() % indicesLibres.size()];
        parqueadero[indiceAleatorio].ocupado = true;
        parqueadero[indiceAleatorio].placaVehiculo = placa;
        res.celdaId = parqueadero[indiceAleatorio].id;
        res.accion = "ENTRADA";
        res.estadoParqueadero = eventosExtra + obtenerEstadoParqueadero();
        return res;
    }
 
    colaEsperaPlacas.push(placa);
    res.celdaId = static_cast<int>(colaEsperaPlacas.size());
    res.accion = "EN_ESPERA";
    res.estadoParqueadero = eventosExtra + obtenerEstadoParqueadero();
    return res;
}
