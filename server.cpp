#define _WIN32_WINNT 0x0600
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define MAX_CLIENTES_EN_ESPERA 5

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

void atenderCliente(SOCKET cliente) {
    std::cout << "\n=== Cliente conectado ===\n";

    while (true) {
        // 📥 Esperar mensaje del cliente
        std::string recibido = recibirMensaje(cliente);
        if (recibido.empty()) {
            std::cout << "🔌 Cliente desconectado.\n";
            break;
        }
        std::cout << "📩 Cliente: " << recibido << "\n";

        // 📤 Enviar respuesta
        std::string respuesta;
        std::cout << "Tú: ";
        std::getline(std::cin, respuesta);

        if (respuesta == "salir") {
            std::cout << "Finalizando conversación...\n";
            break;
        }

        if (!enviarMensaje(cliente, respuesta)) {
            std::cout << "❌ Error al enviar. Conexión perdida.\n";
            break;
        }
        std::cout << "✅ Mensaje enviado\n";
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error inicializando Winsock: " << WSAGetLastError() << "\n";
        return 1;
    }

    SOCKET servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor == INVALID_SOCKET) {
        std::cerr << "Error creando socket: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    int opcion = 1;
    setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, 
               reinterpret_cast<char*>(&opcion), sizeof(opcion));

    struct sockaddr_in direccion {};
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port = htons(PORT);

    if (bind(servidor, reinterpret_cast<struct sockaddr*>(&direccion), 
             sizeof(direccion)) == SOCKET_ERROR) {
        std::cerr << "Error en bind: " << WSAGetLastError() << "\n";
        closesocket(servidor);
        WSACleanup();
        return 1;
    }

    if (listen(servidor, MAX_CLIENTES_EN_ESPERA) == SOCKET_ERROR) {
        std::cerr << "Error en listen: " << WSAGetLastError() << "\n";
        closesocket(servidor);
        WSACleanup();
        return 1;
    }

    std::cout << "🟢 Servidor escuchando en el puerto " << PORT << "\n";

    while (true) {
        SOCKET cliente = accept(servidor, nullptr, nullptr);
        if (cliente == INVALID_SOCKET) {
            std::cerr << "Error aceptando conexión: " << WSAGetLastError() << "\n";
            continue;
        }
        atenderCliente(cliente);
        shutdown(cliente, SD_BOTH);
        closesocket(cliente);
        std::cout << "=== Esperando nuevo cliente ===\n";
    }

    closesocket(servidor);
    WSACleanup();
    return 0;
}