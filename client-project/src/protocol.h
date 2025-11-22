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
typedef struct {
    char type;
    char city[64];
} weather_request_t;

// Struttura risposta (server → client)
typedef struct {
    uint32_t status;//codice di stato
    char type; //Eco del tipo richiesto
    float value; //Valore numerico del dato meteo
} weather_response_t;

// Prototipi
void print_usage_client(const char* program_name);
void print_usage_server(const char* program_name);

float get_temperature(void);
float get_humidity(void);
float get_wind(void);
float get_pressure(void);

#endif /* PROTOCOL_H_ */
