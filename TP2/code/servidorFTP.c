
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <errno.h>

#include "tp_socket.h"

#define FILE_NAME_SIZE 1024	    // Tamanho máximo do buffer p/ troca de info da conexão e nome do arquivo
#define OFFSET 10               // 

typedef struct segment {
    int seq_no;
    char *ack;
    char *pkt_data;
    int pkt_data_size;
} Segment;


int main(int argc, char const *argv[]) {

	int status = 0;                             // Para verificar o retorno das funções
	int server_socket;                     	    // Descritor de socket (equivalente a um HANDLE)
    so_addr client_addr;

    int port = atoi(argv[1]);
    int tam_buffer = atoi(argv[2]);

    char file_Name[FILE_NAME_SIZE] = {0};	// buffer para...

    struct timeval start, end;    // Estruturas com variáveis tv_sec (tipo time_t) e tv_usec (tipo suseconds_t)
    gettimeofday(&start, NULL);   // Inicia a contagem de segundos e microsegundos desde 01/01/1970 00:00:00
    
    if (tp_init() != 0) {
        printf("\ttp_init falhou.\n");
        exit(1);
    }

    if ((server_socket = tp_socket((unsigned short) port)) < 0) {
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
    Segment *segment_send = (Segment *) malloc(sizeof(Segment));
    Segment *segment_recv = (Segment *) malloc(sizeof(Segment));

    segment_send->pkt_data = (char *) malloc(tam_buffer * sizeof(char) + OFFSET + 1);
    segment_send->ack = (char *) malloc(sizeof(char));

    segment_recv->pkt_data = (char *) malloc(tam_buffer * sizeof(char) + OFFSET + 1);
    segment_recv->ack = (char *) malloc(sizeof(char));

    //int segment_size = sizeof(Segment) + (tam_buffer * sizeof(char) + OFFSET + 1);

    //************************************************************************

    char buffer[tam_buffer];

    int data_to_read = 1;
    int ack_recv = 1;
    int wait_for_ack = 1;

    struct timeval temporizador;
    temporizador.tv_sec = 1; //1 segundo
    temporizador.tv_usec = 0; // 0 microsegundos

    //SO_RCVTIMEO : aceita uma struct "timeval" que indica o número em segundos e microsegundos a serem esperados até
    // que uma função se complete.
    if(setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &temporizador, sizeof(temporizador)) < 0){
	printf("\tsetsockopt failed\n");
	exit(1);
    }
    
    printf("\tsetsockopt ok\n");

    while (data_to_read) {

        char tmp[OFFSET];
        sprintf(tmp, "%d", segment_id);
        strcat(segment_send->pkt_data, tmp);
        strcat(segment_send->pkt_data, ":");

        // printf("tmp: %s\n", tmp);                                                        // DELETE
        // printf("segment_send->pkt_data (before read): %s\n", segment_send->pkt_data);    // DELETE

        if ((bytes_sent = fread(buffer, sizeof(char), tam_buffer - 1, File_read)) <= 0) {
            data_to_read = 0;
            //wait_for_ack = 0;
        }

        buffer[((bytes_sent < tam_buffer)? bytes_sent: tam_buffer)] = '\0';           // DELETE

        strcat(segment_send->pkt_data, buffer);
        
        //printf("buffer: %s, bytes_read = %d\n", buffer, bytes_sent);                      // DELETE
        //printf("segment_send->pkt_data (after read): %s\n", segment_send->pkt_data);      // DELETE
        
        bytes_sent_total += bytes_sent;             // Atualiza total de bytes lidos
        printf("%d bytes read\n", bytes_sent);

        if (ack_recv == 1) {

            //printf("segment_send->pkt_data: %s\n", segment_send->pkt_data);                     // DELETE
            
            //segment_send->seq_no = segment_id;                                                // DELETE
            //segment_send->ack = 0;                                                            // DELETE

            segment_send->pkt_data_size = strlen(segment_send->pkt_data);
            
            //printf("strlen(segment_send->pkt_data): %lu\n", strlen(segment_send->pkt_data));  // DELETE

            tp_sendto(server_socket, segment_send->pkt_data, segment_send->pkt_data_size, &client_addr);
            
            wait_for_ack = 1; // DELETE?????????????????

            printf("\tpkt sent\n");

        }

        if (data_to_read == 0) {
            wait_for_ack = 0;
        }

        while(wait_for_ack) {

            status = tp_recvfrom(server_socket, segment_recv->ack, sizeof(segment_recv->ack), &client_addr);

            printf("%d\n", atoi(segment_recv->ack));

            if (status > 0) {
                if (atoi(segment_recv->ack) == segment_id + 1) {
                    //ack_recv = 1;                                         // DELETE
                    wait_for_ack = 0;
                    printf("\tACK received\n");
                } else {
                    printf("\tACK not received\n");
                    //ack_recv = 0;                                         // DELETE
                    wait_for_ack = 1;
                }
            } else {
                if (errno == EWOULDBLOCK) {
                    tp_sendto(server_socket, segment_send->pkt_data, segment_send->pkt_data_size, &client_addr);
                    printf("\ttimeout event\n");
                    printf("\tpkt resent\n");
                    wait_for_ack = 1;
                } else {
                    printf("\terror %d: %s\n", errno, strerror(errno));
                    exit(1);
                }
            }

        }
        
        memset(buffer, 0x0, strlen(buffer));
        memset(segment_send->pkt_data, 0x0, strlen(segment_send->pkt_data));
        
        //printf("segment_send->pkt_data: %s\n", segment_send->pkt_data);       // DELETE

        segment_id++;
    }
    
    fclose(File_read);   // Fecha arquivo de leitura

    free(segment_send->pkt_data);
    free(segment_recv->pkt_data);
    free(segment_send->ack);
    free(segment_recv->ack);
    free(segment_send);
    free(segment_recv);

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


