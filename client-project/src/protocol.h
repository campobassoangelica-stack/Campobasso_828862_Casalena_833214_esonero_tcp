/*
 * protocol.h
 *
 * Client header file
 * Definitions, constants and function prototypes for the client
 */


#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <stdint.h>

// Parametri applicazione
#define SERVER_PORT 27015
#define BUFFER_SIZE 512

// Tipi di dati meteo
#define TYPE_TEMPERATURE 't'
#define TYPE_HUMIDITY    'h'
#define TYPE_WIND        'w'
#define TYPE_PRESSURE    'p'

// Codici di stato
#define STATUS_SUCCESS          0
#define STATUS_CITY_UNAVAILABLE 1
#define STATUS_INVALID_REQUEST  2

// Struttura richiesta (client → server)
struct request {
    char type;      // 't'=temperatura, 'h'=umidità, 'w'=vento, 'p'=pressione
    char city[64];  // nome città (null-terminated)
};

// Struttura risposta (server → client)
struct response {
    unsigned int status;  // 0=successo, 1=città non trovata, 2=richiesta invalida
    char type;            // eco del tipo richiesto
    float value;          // dato meteo generato
};

// Prototipi
void print_usage_client(const char* program_name);
void print_usage_server(const char* program_name);

float get_temperature(void);
float get_humidity(void);
float get_wind(void);
float get_pressure(void);

#endif /* PROTOCOL_H_ */
