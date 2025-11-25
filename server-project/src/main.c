/*
 * main.c
 *
 * TCP Server - Template for Computer Networks assignment
 *
 * Questo file contiene il codice di base per un server TCP
 * portabile su Windows, Linux e macOS.
 */

// Inclusioni librerie di rete e compatibilità Windows/Linux/macOS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "protocol.h"
#if defined WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define closesocket close
#endif

// Lista città supportate
static const char *SUPPORTED_CITIES[] = {
    "Bari", "Roma", "Milano", "Napoli", "Torino",
    "Palermo", "Genova", "Bologna", "Firenze", "Venezia"
};

// Funzioni di utilità per errori e cleanup Winsock
void errorhandler(char *errorMessage) { printf("%s\n", errorMessage); }
void clearwinsock() {
#if defined WIN32
    WSACleanup();
#endif
}

// Funzione per generare un numero float casuale in un range
float random_float(float min, float max) {
    float scale = rand() / (float) RAND_MAX;
    return min + scale * (max - min);
}

// Funzione di validazione della richiesta: controlla tipo e città
void valida(weather_request_t *req, weather_response_t *resp) {
    // Controllo tipo valido
    if(req->type != TYPE_TEMPERATURE && req->type != TYPE_HUMIDITY &&
       req->type != TYPE_WIND && req->type != TYPE_PRESSURE) {
        resp->status = STATUS_INVALID_REQUEST;
        return;
    }

    // Controllo città supportata (case-insensitive)
    int flag = 1;
    for (int i = 0; i < 10; i++) {
        if(strcasecmp(req->city, SUPPORTED_CITIES[i]) == 0) {
            flag = 0;
            break;
        }
    }

    // Imposta status in base al risultato
    if(flag == 1) {
        resp->status = STATUS_CITY_UNAVAILABLE;
    } else {
        resp->status = STATUS_SUCCESS;
    }
}

// Funzioni di generazione dati meteo nei range richiesti
float get_temperature(void) { return random_float(-10.0, 40.0); }
float get_humidity(void)    { return random_float(20.0, 100.0); }
float get_wind(void)        { return random_float(0.0, 100.0); }
float get_pressure(void)    { return random_float(950.0, 1050.0); }

int main(int argc, char *argv[]) {
#if defined WIN32
    // Inizializzazione Winsock su Windows
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2,2), &wsa_data) != 0) {
        printf("Error at WSAStartup()\n");
        return 0;
    }
#endif

    // Porta di default o da argomento
    int port = SERVER_PORT;
    if (argc > 2 && strcmp(argv[1], "-p") == 0) {
        port = atoi(argv[2]);
    }

    // Creazione socket TCP
    int my_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (my_socket < 0) {
        errorhandler("socket creation failed.\n");
        clearwinsock();
        return -1;
    }

    // Configurazione indirizzo server
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = htonl(INADDR_ANY);
    sad.sin_port = htons(port);

    // Binding socket alla porta
    if (bind(my_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
        errorhandler("bind() failed.\n");
        closesocket(my_socket);
        clearwinsock();
        return -1;
    }

    // Messa in ascolto
    if (listen(my_socket, QUEUE_SIZE) < 0) {
        errorhandler("listen() failed.\n");
        closesocket(my_socket);
        clearwinsock();
        return -1;
    }

    struct sockaddr_in cad;
    int client_socket;
    int client_len;

    srand(time(NULL));
    printf("Server in ascolto sulla porta %d...\n", port);

    // Loop infinito: accetta e gestisce richieste
    while (1) {
        client_len = sizeof(cad);

        // Accetta connessione da un client
        if ((client_socket = accept(my_socket, (struct sockaddr*) &cad, (socklen_t*)&client_len)) < 0) {
            errorhandler("accept() failed.\n");
            continue;
        }

        // Riceve la richiesta dal client
        weather_request_t request;
        if (recv(client_socket, (char*)&request, sizeof(request), 0) <= 0) {
            closesocket(client_socket);
            continue;
        }

        printf("Richiesta '%c %s' dal client ip %s\n", request.type, request.city, inet_ntoa(cad.sin_addr));

        // Valida la richiesta e prepara la risposta
        weather_response_t response;
        valida(&request, &response);

        if(response.status == STATUS_SUCCESS) {
            // Genera il dato meteo richiesto
            switch (request.type) {
                case TYPE_TEMPERATURE: response.value = get_temperature(); break;
                case TYPE_HUMIDITY:    response.value = get_humidity(); break;
                case TYPE_WIND:        response.value = get_wind(); break;
                case TYPE_PRESSURE:    response.value = get_pressure(); break;
            }
            response.type = request.type;
        } else {
            // In caso di errore: type nullo e value = 0
            response.type = '\0';
            response.value = 0.0;
        }

        // Invia la risposta al client
        if (send(client_socket, (char*)&response, sizeof(response), 0) != sizeof(response)) {
            errorhandler("send() failed");
        }

        // Chiude la connessione con il client
        closesocket(client_socket);
    }

    // Chiusura socket principale e cleanup
    closesocket(my_socket);
    clearwinsock();
    return 0;
}
