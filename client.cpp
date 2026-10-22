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
 
int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error inicializando Winsock\n";
        return 1;
    }
 
    std::string ip_servidor = "172.24.182.125";
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }
 
    struct sockaddr_in serv_addr {};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, ip_servidor.c_str(), &serv_addr.sin_addr);
 
    if (connect(sock, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(serv_addr)) == SOCKET_ERROR) {
        std::cerr << "No se pudo conectar al servidor\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }
 
    std::cout << "Conectado al servidor\n";
 
    while (true) {
        Vehiculo miVehiculo(generarPlacaAleatoria());
        std::string placa = miVehiculo.getPlaca();
        std::cout << "Enviando: [" << placa << "]\n";
 
        if (!enviarMensaje(sock, placa)) {
            std::cerr << "Conexión perdida\n";
            break;
        }
 
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
 
    closesocket(sock);
    WSACleanup();
    return 0;
}
