#ifndef PROTOCOL_H_
#define PROTOCOL_H_

// Shared application parameters
#define SERVER_PORT 56700   // Server port (change if needed)
#define BUFFER_SIZE 512     // Buffer size for messages
#define DEFAULT_IP "127.0.0.1"
#define QUEUE_SIZE 5        // Max pending connections

//Tipi di dati meteo validi
#define TYPE_TEMPERATURE 't'
#define TYPE_HUMIDITY 'h'
#define TYPE_WIND 'w'
#define TYPE_PRESSURE 'p'

//codici di stato validi

#define STATUS_SUCCESS 0 //Successo
#define STATUS_CITY_UNAVAILABLE 1 //città non disponibile
#define STATUS_INVALID_REQUEST 2 //richiesta non valida


//struttura richiesta client--> server
typedef struct {
    char type;        // Weather data type: 't', 'h', 'w', 'p'
    char city[64];    // City name (null-terminated string)
} weather_request_t;

//struttura risposta server-->client
typedef struct {
    unsigned int status;  // Response status code
    char type;            // Echo of request type
    float value;          // Weather data value
} weather_response_t;


//prototipi funzioni comuni
void errorhandler(char *errorMessage);
void clearwinsock();

//prototipi funzioni di generazione dati server
float get_temperature(void);    // Range: -10.0 to 40.0 °C
float get_humidity(void);       // Range: 20.0 to 100.0 %
float get_wind(void);           // Range: 0.0 to 100.0 km/h
float get_pressure(void);       // Range: 950.0 to 1050.0 hPa
#endif /* PROTOCOL_H_ */
