#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")

#define PORT 8080

ssize_t enviarTodo(SOCKET sock, const char* datos, size_t longitud) {
    size_t enviado = 0;
    while (enviado < longitud) {
        int res = send(sock, datos + enviado, 
                       static_cast<int>(longitud - enviado), 0);
        if (res <= 0) return res;
        enviado += res;
    }
    return static_cast<ssize_t>(longitud);
}

ssize_t recibirTodo(SOCKET sock, char* buffer, size_t longitud) {
    size_t recibido = 0;
    while (recibido < longitud) {
        int res = recv(sock, buffer + recibido, 
                       static_cast<int>(longitud - recibido), 0);
        if (res <= 0) return res;
        recibido += res;
    }
    return static_cast<ssize_t>(longitud);
}

bool enviarMensaje(SOCKET sock, const std::string& texto) {
    uint32_t tam = htonl(static_cast<uint32_t>(texto.size()));
    if (enviarTodo(sock, reinterpret_cast<char*>(&tam), sizeof(tam)) <= 0)
        return false;
    if (enviarTodo(sock, texto.c_str(), texto.size()) <= 0)
        return false;
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

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error inicializando Winsock: " << WSAGetLastError() << "\n";
        return 1;
    }

    std::string ip_servidor;
    std::cout << "Ingrese IP del servidor (predeterminado 127.0.0.1): ";
    std::getline(std::cin, ip_servidor);
    if (ip_servidor.empty()) ip_servidor = "127.0.0.1";

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Error creando socket: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    struct sockaddr_in serv_addr {};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(ip_servidor.c_str());
    if (serv_addr.sin_addr.s_addr == INADDR_NONE) {
        std::cerr << "Dirección IP inválida\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    if (connect(sock, reinterpret_cast<struct sockaddr*>(&serv_addr), 
                sizeof(serv_addr)) == SOCKET_ERROR) {
        std::cerr << "Error de conexión: " << WSAGetLastError() << "\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "✅ Conectado al servidor. Escribe 'salir' para terminar.\n";

    while (true) {
        // 📤 Enviar mensaje
        std::string mensaje;
        std::cout << "Tú: ";
        std::getline(std::cin, mensaje);

        if (mensaje == "salir") {
            std::cout << "Cerrando conexión...\n";
            break;
        }

        if (!enviarMensaje(sock, mensaje)) {
            std::cerr << "❌ Error al enviar mensaje\n";
            break;
        }

        // 📥 Esperar respuesta
        std::string respuesta = recibirMensaje(sock);
        if (respuesta.empty()) {
            std::cout << "🔌 El servidor cerró la conexión.\n";
            break;
        }
        std::cout << "Servidor: " << respuesta << "\n";
    }

    shutdown(sock, SD_BOTH);
    closesocket(sock);
    WSACleanup();
    std::cout << "Conexión finalizada.\n";
    return 0;
}