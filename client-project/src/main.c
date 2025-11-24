/*
 * main.c
 *
 * TCP Client - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP client
 * portable across Windows, Linux and macOS.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define CLOSESOCKET(s) closesocket(s)
typedef SOCKET socket_t;
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define CLOSESOCKET(s) close(s)
typedef int socket_t;
#endif

#include "protocol.h"

#ifdef _WIN32
static int initialize_winsock(void) {
    WSADATA wsa_data;
    return WSAStartup(MAKEWORD(2,2), &wsa_data);
}
static void cleanup_winsock(void) { WSACleanup(); }
#else
static int initialize_winsock(void) { return 0; }
static void cleanup_winsock(void) {}
#endif

int main(int argc, char *argv[]) {
    char *server_ip = "localhost";   // default server
    int port = SERVER_PORT;          // default port 27015
    char type = 0;
    char city[64] = {0};

    // Parsing degli argomenti
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i+1 < argc) {
            server_ip = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i+1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-r") == 0 && i+2 < argc) {
            type = argv[i+1][0];
            strncpy(city, argv[i+2], sizeof(city)-1);
            break;
        }
    }

    if (type == 0 || strlen(city) == 0) {
        print_usage_client(argv[0]);
        return 1;
    }

    if (initialize_winsock() != 0) {
        fprintf(stderr, "Errore Winsock\n");
        return 1;
    }

    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Errore socket");
        cleanup_winsock();
        return 1;
    }

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP

    char port_str[16];
    sprintf(port_str, "%d", port);

    if (getaddrinfo(server_ip, port_str, &hints, &res) != 0) {
        fprintf(stderr, "Errore risoluzione DNS\n");
        CLOSESOCKET(sock);
        cleanup_winsock();
        return 1;
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        perror("Errore connect");
        freeaddrinfo(res);
        CLOSESOCKET(sock);
        cleanup_winsock();
        return 1;
    }

    freeaddrinfo(res);

    // --- Struttura richiesta coerente con protocol.h ---
    struct request req;
    memset(&req, 0, sizeof(req));
    req.type = type;
    strncpy(req.city, city, sizeof(req.city)-1);

    send(sock, (const char*)&req, sizeof(req), 0);

    // --- Struttura risposta coerente con protocol.h ---
    weather_response_t response;
    int bytes_received = recv(sock, (char*)&response, sizeof(response), 0);
    if (bytes_received <= 0) {
        perror("Errore recv");
        CLOSESOCKET(sock);
        cleanup_winsock();
        return 1;
    }

    // Conversione endianness del campo status
    response.status = ntohl(response.status);

    printf("Ricevuto risultato dal server ip %s. ", server_ip);

    if (response.status == STATUS_SUCCESS) {
        switch(response.type) {
            case TYPE_TEMPERATURE:
                printf("%s: Temperatura = %.1f°C\n", req.city, response.value);
                break;
            case TYPE_HUMIDITY:
                printf("%s: Umidità = %.1f%%\n", req.city, response.value);
                break;
            case TYPE_WIND:
                printf("%s: Vento = %.1f km/h\n", req.city, response.value);
                break;
            case TYPE_PRESSURE:
                printf("%s: Pressione = %.1f hPa\n", req.city, response.value);
                break;
        }
    } else if (response.status == STATUS_CITY_UNAVAILABLE) {
        printf("Città non disponibile\n");
    } else if (response.status == STATUS_INVALID_REQUEST) {
        printf("Richiesta non valida\n");
    } else {
        printf("Errore sconosciuto\n");
    }

    CLOSESOCKET(sock);
    cleanup_winsock();
    return 0;
}

void print_usage_client(const char* program_name) {
    printf("Uso: %s [-s server] [-p porta] -r <tipo> <città>\n", program_name);
    printf("Esempio: %s -s 127.0.0.1 -p 27015 -r t Roma\n", program_name);
    printf("Tipi: t=temperatura, h=umidità, w=vento, p=pressione\n");
}
