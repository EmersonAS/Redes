
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <errno.h>

#include "tp_socket.h"

#define FILE_NAME_SIZE 1024	// Tamanho máximo do buffer p/ troca de info da conexão e nome do arquivo


typedef struct packet{
    char data[20];
} Packet;


typedef struct segment {
    Packet packet;
    int seq_no;
    int ack;
} Segment;


int main(int argc, char const *argv[]) {

	int status = 0;                             // Para verificar o retorno das funções
	int server_socket;                     	    // Descritor de socket (equivalente a um HANDLE)
    so_addr client_addr;

    int PORT = atoi(argv[1]);
    int BUFFER_DATA_SIZE = atoi(argv[2]);

    char file_Name[FILE_NAME_SIZE] = {0};	// buffer para...
    
    char buffer_Data[BUFFER_DATA_SIZE];    // EXCLUIR	// buffer para armazenar tempoariamente os bytes (conteúdo) das msgs a serem enviadas

    struct timeval start, end;    // Estruturas com variáveis tv_sec (tipo time_t) e tv_usec (tipo suseconds_t)
    gettimeofday(&start, NULL);   // Inicia a contagem de segundos e microsegundos desde 01/01/1970 00:00:00
    
    if (tp_init() != 0) {
        printf("\ttp_init falhou.\n");
        exit(1);
    }

    if ((server_socket = tp_socket((unsigned short) PORT)) < 0) {
        printf("\ttp_socket falhou.\n");
        exit(1);
    }
    printf("\tserver socket created.\n");

    
    FILE * File_read;           // Ponteiro para o arquivo a ser lido
    int bytes_sent_total = 0;   // Inicializa contador do total de bytes lidos (a serem enviados) do arquivo 
    int bytes_sent = 0;         // Inicializa a contagem de bytes lidos do arquivo

    // Recebe o nome do arquivo completo
	if ((tp_recvfrom(server_socket, file_Name, FILE_NAME_SIZE, &client_addr)) < 0) {
        printf("\tfile name not received.\n");
        exit(1);
    }
    printf("\tfile name received: %s\n", file_Name);
            
    File_read = fopen(file_Name, "r");
  	if(File_read == NULL){
  	    printf("\tcould not open file.\n");
  	    exit(1);
  	}

    
    /* LÓGICA DO STOP-AND-WAIT */

    int segment_id = 0;
    Segment segment_send;
    Segment segment_recv;

    int ack_recv = 1;
    int wait_for_ack = 1;

    while( (bytes_sent = fread(segment_send.packet.data, sizeof(char), sizeof(segment_send.packet.data)-1, File_read)) > 0 ) {
        
        bytes_sent_total += bytes_sent;             // Atualiza total de bytes lido/enviado

        if (ack_recv == 1){

            segment_send.packet.data[((bytes_sent < 20)? bytes_sent : 20)] = '\0'; //BUFFER_DATA_SIZE = 20 bytes
            segment_send.seq_no = segment_id;
            segment_send.ack = 0;

            tp_sendto(server_socket, (char *) &segment_send, sizeof(Segment), &client_addr);
            wait_for_ack = 1;

            printf("\tpkt sent\n");

        }

        while(wait_for_ack == 1) {

            status = tp_recvfrom(server_socket, (char *) &segment_recv, sizeof(Segment), &client_addr);

            if (status > 0) {
                if (segment_recv.seq_no == 0 && segment_recv.ack == segment_id + 1) {
                    ack_recv = 1;
                    wait_for_ack = 0;
                    printf("\tACK received\n");
                } else {
                    printf("\tACK not received\n");
                    //ack_recv = 0;
                    wait_for_ack = 1;
                }
            } else {
                if (errno == EWOULDBLOCK) {
                    tp_sendto(server_socket, (char *) &segment_send, sizeof(Segment), &client_addr);
                    printf("\ttimeout event\n");
                    printf("\tpkt resent\n");
                    wait_for_ack = 1;
                } else {
                    printf("\terror %d: %s\n", errno, strerror(errno));
                    exit(1);
                }
            }

        }

        segment_id++;
    }
    
    fclose(File_read);   // Fecha arquivo de leitura

    gettimeofday(&end, NULL);      // Inicia a contagem de segundos e microsegundos desde 01/01/1970 00:00:00

    // Tempo gasto é dado pela diferença da última com a primeira contagem (end - start)
    printf("t = %.4f segundos\nL = %d bytes enviados\n", 
      ((end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6)), bytes_sent_total);

	return 0;
}

    /* Início - Lógica do Temporizador */

    /*
    int timeout = 5;                // 5s
    struct timeval temporizador;
    temporizador.tv_sec = timeout;
    temporizador.tv_usec = 0;
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*) &temporizador, sizeof(temporizador));
    */

    /* Fim - Lógica do Temporizador*/


    /* // Identificando um evento de timout

    
    */
