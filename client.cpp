#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>
#include <chrono>
#include <random>
#pragma comment(lib, "ws2_32.lib")
#define PORT 8080
 
std::string generarPlaca() {
    std::string l = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string n = "0123456789";
    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<> dl(0, 25);
    std::uniform_int_distribution<> dn(0, 9);
    std::string p;
    for (int i = 0; i < 3; i++) p += l[dl(g)];
    p += "-";
    for (int i = 0; i < 3; i++) p += n[dn(g)];
    return p;
}
 
int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    std::string IP_SERVIDOR = "PON_LA_IP_DEL_SERVIDOR_AQUI";
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_port = htons(PORT);
    a.sin_addr.s_addr = inet_addr(IP_SERVIDOR.c_str());
    if (connect(s, (sockaddr*)&a, sizeof(a)) == SOCKET_ERROR) {
        std::cout << "NO SE PUDO CONECTAR" << std::endl;
        closesocket(s);
        WSACleanup();
        return 1;
    }
    std::cout << "CONECTADO AL SERVIDOR" << std::endl;
    while (true) {
        std::string placa = generarPlaca();
        std::cout << "ENVIANDO: " << placa << std::endl;
        uint32_t tam = htonl((uint32_t)placa.size());
        send(s, (char*)&tam, 4, 0);
        send(s, placa.c_str(), (int)placa.size(), 0);
        char ok[2];
        recv(s, ok, 2, 0);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    closesocket(s);
    WSACleanup();
    return 0;
}