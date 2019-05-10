
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "tp_socket.h"

#define FILE_NAME_SIZE 1024	// Tamanho máximo do buffer p/ troca de info da conexão e nome do arquivo


typedef struct packet{
    char data[512];
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

    //struct timeval start, end;    // Estruturas com variáveis tv_sec (tipo time_t) e tv_usec (tipo suseconds_t)
    //gettimeofday(&start, NULL);   // Inicia a contagem de segundos e microsegundos desde 01/01/1970 00:00:00
    
    if (tp_init() != 0) {
        printf("tp_init falhou.\n");
        exit(1);
    }

    if ((server_socket = tp_socket((unsigned short) PORT)) < 0) {
        printf("tp_socket falhou.\n");
        exit(1);
    }
    printf("\tserver socket created.\n");

    /*
    FILE * File_read;           // Ponteiro para o arquivo a ser lido
    int bytes_sent_total = 0;   // Inicializa contador do total de bytes lidos do arquivo ( a serem enviados)
    int bytes_sent = 0;         // Inicializa a contagem de bytes lidos do arquivo

    // Recebe o nome do arquivo completo
	if ((tp_recvfrom(server_socket, file_Name, FILE_NAME_SIZE, &client_addr)) < 0) {
        printf("File name not received.\n");
        exit(1);
    }
    printf("File name received: %s\n", file_Name);
            
    File_read = fopen(file_Name, "r");
  	if(File_read == NULL){
  	    printf("Can not open file.\n");
  	    exit(1);
  	}


        while( (bytes_sent = fread(buffer_Data, sizeof(char), (BUFFER_DATA_SIZE-1), File_read)) > 0 )
        bytes_sent_total += bytes_sent;             // Atualiza total de bytes lido/enviado

    */

    
    status = tp_recvfrom(server_socket, file_Name, FILE_NAME_SIZE, &client_addr);
    printf("%s\n", file_Name);

    
    /* LÓGICA DO STOP-AND-WAIT */

    int segment_id = 0;
    Segment segment_send;
    Segment segment_recv;
    
    //segment_recv.packet.data = malloc(BUFFER_DATA_SIZE);
    //segment_send.packet.data = malloc(BUFFER_DATA_SIZE);

    int ack_recv = 1;

    while(1){
        if (ack_recv == 1){
            segment_send.seq_no = segment_id;
            segment_send.ack = 0;

            printf("Enter data: ");
            scanf("%s", buffer_Data);
            strcpy(segment_send.packet.data, buffer_Data);

            tp_sendto(server_socket, (char *) &segment_send, sizeof(Segment), &client_addr);
            printf("Segment sent\n");
        }
        //int addr_size = sizeof(server_addr);
        int f_recv_size = tp_recvfrom(server_socket, (char *) &segment_recv, sizeof(Segment), &client_addr);
        
        if (f_recv_size > 0 && segment_recv.seq_no == 0 && segment_recv.ack == segment_id + 1){
            printf("ACK Received\n");
            ack_recv = 1;
        } else {
            printf("ACK Not Received\n");
            ack_recv = 0;
        }
        segment_id++;
    }

    
    //fclose(File_read);   // Fecha arquivo de leitura

    //gettimeofday(&end, NULL);      // Inicia a contagem de segundos e microsegundos desde 01/01/1970 00:00:00

    // Tempo gasto é dado pela diferença da última com a primeira contagem (end - start)
    //printf("t = %.4f segundos\nL = %d bytes enviados\n", 
    //  ((end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6)), bytes_sent_total);

    //free(segment_recv.packet.data);
    //free(segment_send.packet.data);

	return 0;
}