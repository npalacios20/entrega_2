#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#pragma comment(lib, "ws2_32.lib")

#define PORT 8080

class Vehiculo {
private:
    std::string placa;
public:
    Vehiculo(const std::string& p) : placa(p) {}
    std::string getPlaca() const { return placa; }
};

std::string generarPlacaAleatoria() {
    std::string letras = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string numeros = "0123456789";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> disLetras(0, 25);
    std::uniform_int_distribution<> disNums(0, 9);

    std::string placa = "";
    for (int i = 0; i < 3; ++i) placa += letras[disLetras(gen)];
    placa += "-";
    for (int i = 0; i < 3; ++i) placa += numeros[disNums(gen)];
    return placa;
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

void escucharServidor(SOCKET sock) {
    while (true) {
        std::string respuesta = recibirMensaje(sock);
        if (respuesta.empty()) {
            std::cout << "🔌 Conexión cerrada por el servidor.\n";
            break;
        }
        std::cout << "\n📥 [RESPUESTA DEL SERVIDOR]:\n" << respuesta << "\n--------------------------------------------------\n";
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error inicializando Winsock\n";
        return 1;
    }

    std::string ip_servidor = "192.168.1.147";
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    struct sockaddr_in serv_addr {};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(ip_servidor.c_str());

    if (connect(sock, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) == SOCKET_ERROR) {
        std::cerr << "Error conectando al servidor de parqueadero.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "🚗 [CLIENTE] Generador continuo de vehículos activo (Envía un carro cada 5s sin frenar la cola)...\n\n";

    std::thread hiloEscucha(escucharServidor, sock);
    hiloEscucha.detach();

    while (true) {
        Vehiculo miVehiculo(generarPlacaAleatoria());
        std::string payload = miVehiculo.getPlaca();

        std::cout << "📤 Enviando solicitud para vehículo con placa: " << payload << "\n";
        if (!enviarMensaje(sock, payload)) {
            std::cerr << "❌ Conexión perdida con el servidor.\n";
            break;
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}