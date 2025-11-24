/*
 * main.c
 *
 * TCP Server - Template for Computer Networks assignment
 *
 * This file contains the boilerplate code for a TCP server
 * portable across Windows, Linux and macOS.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

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

// Lista città supportate
static const char* supported_cities[] = {
    "bari","roma","milano","napoli","torino",
    "palermo","genova","bologna","firenze","venezia",NULL
};

static int is_city_supported(const char *city) {
    char city_lower[64];
    strncpy(city_lower, city, sizeof(city_lower)-1);
    city_lower[sizeof(city_lower)-1] = '\0';
    for (char *p=city_lower; *p; p++) *p = tolower(*p);
    for (int i=0; supported_cities[i]!=NULL; i++)
        if (strcmp(city_lower, supported_cities[i])==0) return 1;
    return 0;
}

static int validate_city(const char* city) {
    if (strlen(city) >= 64) return 0;   // troppo lunga
    for (const char* p = city; *p; p++) {
        if (*p == '\t') return 0;       // tab vietato
    }
    return 1;
}

// Funzioni meteo simulate
float get_temperature(void) { return -10.0f + (rand() % 501) / 10.0f; }   // -10.0 → 40.0
float get_humidity(void)    { return 20.0f + (rand() % 801) / 10.0f; }    // 20.0 → 100.0
float get_wind(void)        { return (rand() % 1001) / 10.0f; }           // 0.0 → 100.0
float get_pressure(void)    { return 950.0f + (rand() % 1001) / 10.0f; }  // 950.0 → 1050.0

int main(int argc,char *argv[]) {
    srand((unsigned int)time(NULL));

    // Gestione porta da riga di comando
    int port = SERVER_PORT;  // default 27015
    if (argc == 3 && strcmp(argv[1], "-p") == 0) {
        port = atoi(argv[2]);
    }

    if (initialize_winsock() != 0) {
        fprintf(stderr, "Errore Winsock\n");
        return 1;
    }

    socket_t server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Errore socket");
        cleanup_winsock();
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Errore bind");
        CLOSESOCKET(server_sock);
        cleanup_winsock();
        return 1;
    }

    if (listen(server_sock, SOMAXCONN) < 0) {
        perror("Errore listen");
        CLOSESOCKET(server_sock);
        cleanup_winsock();
        return 1;
    }
    printf("Server TCP in ascolto sulla porta %d...\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        socket_t client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) { perror("Errore accept"); continue; }

        struct request req;          // <-- uso la struct request definita in protocol.h
        weather_response_t response; // <-- uso la struct risposta definita in protocol.h
        memset(&req, 0, sizeof(req));
        memset(&response, 0, sizeof(response));

        int bytes_received = recv(client_sock, (char*)&req, sizeof(req), 0);
        if (bytes_received <= 0) { perror("Errore recv"); CLOSESOCKET(client_sock); continue; }

        printf("Richiesta '%c %s' dal client ip %s\n",
               req.type, req.city, inet_ntoa(client_addr.sin_addr));

        // Elaborazione richiesta
        if (req.type!=TYPE_TEMPERATURE && req.type!=TYPE_HUMIDITY &&
            req.type!=TYPE_WIND && req.type!=TYPE_PRESSURE) {
            response.status = STATUS_INVALID_REQUEST;
        } else if (!validate_city(req.city) || !is_city_supported(req.city)) {
            response.status = STATUS_CITY_UNAVAILABLE;
        } else {
            response.status = STATUS_SUCCESS;
            response.type = req.type;
            switch(req.type) {
                case TYPE_TEMPERATURE: response.value = get_temperature(); break;
                case TYPE_HUMIDITY:    response.value = get_humidity();    break;
                case TYPE_WIND:        response.value = get_wind();        break;
                case TYPE_PRESSURE:    response.value = get_pressure();    break;
            }
        }

        // Conversione status in network order
        unsigned int net_status = htonl(response.status);
        memcpy(&response.status, &net_status, sizeof(net_status));

        // Invio risposta
        send(client_sock, (const char*)&response, sizeof(response), 0);

        CLOSESOCKET(client_sock);
    }

    CLOSESOCKET(server_sock);
    cleanup_winsock();
    return 0;
}
