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
#include <ctype.h>
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

#include "protocol.h"
//funzione di gestione errori:stampa messaggio
void errorhandler (char *errorMessage) {
	printf("%s\n",errorMessage);
}
//funzione di cleanup per winsock (solo per windows)
void clearwinsock () {
#if defined WIN32
	WSACleanup();
#endif
}

int main(int argc, char *argv[]) {
//inizializzazione libreria winsock solo per windows
#if defined WIN32
	WSADATA wsa_data;
	if(WSAStartup(MAKEWORD(2,2),&wsa_data)!=0) {
		printf("Errore in WSAStartup()\n");
		return 0;
	}
#endif

	int port = SERVER_PORT; //Porta di default da protocol.h
	 char ip[32] = DEFAULT_IP; //IP di default 127.0.0.1
	char request_string[128]= ""; //stringa richiesta  di "tipo città"
	int richiesta = 0; //flag per sapere se -r è stato passato


	//struttura richiesta da inviare al server
	weather_request_t request;
	memset(&request, 0, sizeof(request));

 // Parsing degli argomenti

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 && i+1 < argc) {
            strncpy(ip, argv[++i], 31);
        } else if (strcmp(argv[i], "-p") == 0 && i+1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-r") == 0 && i+1 < argc) {
            strncpy (request_string, argv[++i],127);
            richiesta = 1;
        }
    }
if (!richiesta) {
	 printf("Uso: %s [-s server] [-p port] -r \"tipo città\"\n", argv[0]);
	        clearwinsock();
        return -1;
    }

//parsing della stringa richiesta
int i = 0;
while (request_string[i]== ' ' &&request_string[i] != '\0')i++;

request.type = request_string[i];
i++;

while (request_string[i]== ' ' &&request_string[i] != '\0')i++;
strncpy (request.city, &request_string[i], 63);
request.city[63] = '\0';

int c_socket =socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
if(c_socket < 0){
	errorhandler ("Creazione del socket fallita.\n");
	clearwinsock();
	return -1;
}
//configurazione indirizzo server
struct sockaddr_in sad;
memset (&sad, 0, sizeof(sad));
sad.sin_family = AF_INET;
sad.sin_addr.s_addr =inet_addr(ip);
sad.sin_port =htons(port);

//connesione al server
if (connect(c_socket, (struct sockaddr *)&sad,sizeof(sad))<0) {
	errorhandler("Connessione fallita.\n");
	closesocket(c_socket);
	clearwinsock();
	return -1;
}
//invio della richiesta
if(send(c_socket, (char*)&request,sizeof(request), 0) !=sizeof(request)) {
	errorhandler("send() ha inviato un numero di byte diverso dall'atteso");
	        closesocket(c_socket);
	        clearwinsock();
	        return -1;
}
//ricezione della risposta
weather_response_t response;
   if ((recv(c_socket, (char*)&response, sizeof(response), 0)) <= 0) {
       errorhandler("recv() fallita o connessione chiusa prematuramente");
       closesocket(c_socket);
       clearwinsock();
       return -1;
   }

   //stampa del risultato
   printf("Ricevuto risultato dal server ip %s. ", ip);
   if (response.status == STATUS_SUCCESS) {
          request.city[0] = toupper(request.city[0]); //prima lettere maiuscola
          switch (response.type) {
              case TYPE_TEMPERATURE:
            	  printf("%s: Temperatura = %.1f°C\n", request.city, response.value);
            	  break;
              case TYPE_HUMIDITY:
            	  printf("%s: Umidità = %.1f%%\n", request.city, response.value);
            	  break;
              case TYPE_WIND:
            	  printf("%s: Vento = %.1f km/h\n", request.city, response.value);
            	  break;
              case TYPE_PRESSURE:
            	  printf("%s: Pressione = %.1f hPa\n", request.city, response.value);
            	  break;
              default:  printf("Tipo sconosciuto ricevuto.\n");
          }
      } else if (response.status == STATUS_CITY_UNAVAILABLE) {
          printf("Città non disponibile\n");
      } else if (response.status == STATUS_INVALID_REQUEST) {
          printf("Richiesta non valida\n");
      } else {
          printf("Errore sconosciuto\n");
      }
//chiusura del socket e cleanup
      closesocket(c_socket);
      clearwinsock();
      return 0;
  }

