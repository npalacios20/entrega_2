#define _WIN32_WINNT 0x0600
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <random>
#include <thread>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define TOTAL_ESPACIOS 6 

struct EspacioParqueadero {
    int id;
    bool ocupado;
    std::string placaVehiculo;
};

struct PeticionEspera {
    SOCKET sock;
    std::string placa;
    bool atendido = false;
    std::mutex mtxLocal;
    std::condition_variable cvLocal;
};

std::vector<EspacioParqueadero> parqueadero(TOTAL_ESPACIOS);
std::queue<std::shared_ptr<PeticionEspera>> colaEspera; 
std::mutex mtxParqueadero; 

void inicializarParqueadero() {
    for (int i = 0; i < TOTAL_ESPACIOS; ++i) {
        parqueadero[i] = {i + 1, false, ""};
    }
}

std::string obtenerEstadoParqueadero() {
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
    estado += "🚗 Vehículos en fila de espera: " + std::to_string(colaEspera.size()) + "\n";
    return estado;
}

ssize_t enviarTodo(SOCKET sock, const char* datos, size_t longitud) {
    size_t enviado = 0;
    while (enviado < longitud) {
        int res = send(sock, datos + enviado, static_cast<int>(longitud - enviado), 0);
        if (res <= 0) return res;
        enviado += res;
    }
    return static_cast<ssize_t>(longitud);
}

ssize_t recibirTodo(SOCKET sock, char* buffer, size_t longitud) {
    size_t recibido = 0;
    while (recibido < longitud) {
        int res = recv(sock, buffer + recibido, static_cast<int>(longitud - recibido), 0);
        if (res <= 0) return res;
        recibido += res;
    }
    return static_cast<ssize_t>(longitud);
}

bool enviarMensaje(SOCKET sock, const std::string& texto) {
    uint32_t tam = htonl(static_cast<uint32_t>(texto.size()));
    if (enviarTodo(sock, reinterpret_cast<char*>(&tam), sizeof(tam)) <= 0) return false;
    if (enviarTodo(sock, texto.c_str(), texto.size()) <= 0) return false;
    return true;
}

std::string recibirMensaje(SOCKET sock) {
    uint32_t tam = 0;
    int res = recibirTodo(sock, reinterpret_cast<char*>(&tam), sizeof(tam));
    if (res <= 0) return "";
    tam = ntohl(tam);
    
    char* buffer = new char[tam + 1];
    res = recibirTodo(sock, buffer, tam);
    if (res <= 0) {
        delete[] buffer;
        return "";
    }
    buffer[tam] = '\0';
    std::string mensaje(buffer);
    delete[] buffer;
    return mensaje;
}


void hiloLiberadorAutomatico() {
    std::random_device rd;
    std::mt19937 gen(rd());

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(8)); 

        std::unique_lock<std::mutex> lock(mtxParqueadero);

        std::vector<int> espaciosOcupadosIndices;
        for (int i = 0; i < TOTAL_ESPACIOS; ++i) {
            if (parqueadero[i].ocupado) {
                espaciosOcupadosIndices.push_back(i);
            }
        }

        if (!espaciosOcupadosIndices.empty()) {
            std::uniform_int_distribution<> dis(0, espaciosOcupadosIndices.size() - 1);
            int indiceLiberar = espaciosOcupadosIndices[dis(gen)];

            std::cout << "\n⏱️ [SALIDA] El vehículo con placa " 
                      << parqueadero[indiceLiberar].placaVehiculo 
                      << " salió del espacio " << parqueadero[indiceLiberar].id << "\n";

            if (!colaEspera.empty()) {
                
                auto siguientePeticion = colaEspera.front();
                colaEspera.pop();

                parqueadero[indiceLiberar].placaVehiculo = siguientePeticion->placa;
                
                std::cout << "🚗 [FILA - TURNO] El vehículo en espera " << siguientePeticion->placa 
                          << " entró al espacio " << parqueadero[indiceLiberar].id << " (Quedan en fila: " << colaEspera.size() << ")\n";

                std::string respuesta = "✅ ACCESO CONCEDIDO (Desde la fila de espera): Estacionado en el espacio [" + std::to_string(parqueadero[indiceLiberar].id) + "]\n\n" + obtenerEstadoParqueadero();
                enviarMensaje(siguientePeticion->sock, respuesta);

                {
                    std::lock_guard<std::mutex> lockLocal(siguientePeticion->mtxLocal);
                    siguientePeticion->atendido = true;
                }
                siguientePeticion->cvLocal.notify_one();

            } else {
                parqueadero[indiceLiberar].ocupado = false;
                parqueadero[indiceLiberar].placaVehiculo = "";
            }
        }
    }
}


void manejarVehiculoEntrante(SOCKET clienteSocket, std::string placaRecibida) {
    std::unique_lock<std::mutex> lock(mtxParqueadero);
    std::random_device rd;
    std::mt19937 gen(rd());

   
    std::vector<int> espaciosLibresIndices;
    for (int i = 0; i < TOTAL_ESPACIOS; ++i) {
        if (!parqueadero[i].ocupado) {
            espaciosLibresIndices.push_back(i);
        }
    }

    if (!espaciosLibresIndices.empty()) {
        
        std::uniform_int_distribution<> dis(0, espaciosLibresIndices.size() - 1);
        int indiceAleatorio = espaciosLibresIndices[dis(gen)];

        parqueadero[indiceAleatorio].ocupado = true;
        parqueadero[indiceAleatorio].placaVehiculo = placaRecibida;

        std::cout << "✅ Vehículo " << placaRecibida << " estacionado al azar en espacio #" << parqueadero[indiceAleatorio].id << "\n";
        std::string respuesta = "✅ ACCESO CONCEDIDO: Estacionado en el espacio [" + std::to_string(parqueadero[indiceAleatorio].id) + "]\n\n" + obtenerEstadoParqueadero();
        
        enviarMensaje(clienteSocket, respuesta);
    } else {
        
        std::cout << "⏳ Parqueadero LLENO. Vehículo " << placaRecibida << " entra en la FILA DE ESPERA (Total en cola: " << colaEspera.size() + 1 << ").\n";

        auto peticion = std::make_shared<PeticionEspera>();
        peticion->sock = clienteSocket;
        peticion->placa = placaRecibida;
        peticion->atendido = false;

        colaEspera.push(peticion);

        std::unique_lock<std::mutex> lockLocal(peticion->mtxLocal);
        lock.unlock(); 

        peticion->cvLocal.wait(lockLocal, [&] { return peticion->atendido; });
    }
}

void atenderCliente(SOCKET cliente) {
    std::cout << "\n=== Cliente generador de vehículos conectado ===\n";

    while (true) {
        std::string placaRecibida = recibirMensaje(cliente);
        if (placaRecibida.empty()) {
            std::cout << "🔌 Cliente desconectado.\n";
            break;
        }
        
        std::cout << "\n📩 Solicitud de entrada - Vehículo placa: " << placaRecibida << "\n";
        std::thread(manejarVehiculoEntrante, cliente, placaRecibida).detach();
    }
}

int main() {
    inicializarParqueadero();

    std::thread hiloLiberacion(hiloLiberadorAutomatico);
    hiloLiberacion.detach();

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error inicializando Winsock\n";
        return 1;
    }

    SOCKET servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    int opcion = 1;
    setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&opcion), sizeof(opcion));

    struct sockaddr_in direccion {};
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port = htons(PORT);

    if (bind(servidor, reinterpret_cast<struct sockaddr*>(&direccion), sizeof(direccion)) == SOCKET_ERROR) {
        closesocket(servidor);
        WSACleanup();
        return 1;
    }

    if (listen(servidor, 5) == SOCKET_ERROR) {
        closesocket(servidor);
        WSACleanup();
        return 1;
    }

    std::cout << "🟢 Servidor con Selección Aleatoria y Fila Dinámica activo en el puerto " << PORT << "\n";

    while (true) {
        SOCKET cliente = accept(servidor, nullptr, nullptr);
        if (cliente == INVALID_SOCKET) continue;
        
        atenderCliente(cliente);
        closesocket(cliente);
        std::cout << "=== Esperando nueva sesión del cliente generador ===\n";
    }

    closesocket(servidor);
    WSACleanup();
    return 0;
}