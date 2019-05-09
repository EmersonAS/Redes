
#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
#include <netinet/in.h>

#include "tp_socket.h"

#define BUFFER_INFO_SIZE 1024	// Tamanho máximo do buffer p/ troca de info da conexão e nome do arquivo


typedef struct packet{
    char data[10];
} Packet;


typedef struct frame {
    int frame_kind;
    int seq_no;
    int ack;
    Packet packet;
} Frame;


int main(int argc, char const *argv[]) {

	int status = 0;                             // Para verificar o retorno das funções
	int server_fd;                     			// Descritor de socket (equivalente a um HANDLE)
    so_addr newAddr;

    int PORT = atoi(argv[1]);
    int BUFFER_DATA_SIZE = atoi(argv[2]);

    char buffer_Info[BUFFER_INFO_SIZE] = {0};	// buffer para...
    char buffer_Data[BUFFER_DATA_SIZE];     	// buffer para armazenar tempoariamente os bytes (conteúdo) das msgs a serem enviadas

    //struct timeval start, end;    // Estruturas com variáveis tv_sec (tipo time_t) e tv_usec (tipo suseconds_t)
    //gettimeofday(&start, NULL);   // Inicia a contagem de segundos e microsegundos desde 01/01/1970 00:00:00

    status = tp_init();
    if (status != 0) {
        printf("tp_init falhou.\n");
        exit(1);
    }

    server_fd = tp_socket((unsigned short) PORT);
    if (server_fd < 0) {
        printf("tp_socket falhou.\n");
        exit(1);
    }
    printf("Socket do servidor criado com sucesso.\n");

    /*
    FILE * File_out;            // Ponteiro para o arquivo a ser lido
    int bytes_sent_total = 0;   // Inicializa contador do total de bytes lidos do arquivo (enviados)
    int bytes_sent = 0;         // Inicializa a contagem de bytes lidos do arquivo

    // Recebe o nome do arquivo completo
	status = tp_recvfrom(server_fd, buffer_Info, BUFFER_INFO_SIZE, 0);
	if (status == -1) {
        printf("Erro ao receber o nome do arquivo.\n");
        exit(1);
    }
    printf("Nome do arquivo recebido: %s\n", buffer_Info);
            
    File_out = fopen(buffer_Info, "r");
        	
  	if(File_out == NULL){
  	    printf("Erro na abertura do arquivo a ser enviado.\n");
  	    exit(1);
  	}
    */

    
    status = tp_recvfrom(server_fd, buffer_Info, BUFFER_INFO_SIZE, &newAddr);
    printf("%s\n", buffer_Info);
    

    /* LÓGICA DO STOP-AND-WAIT */

    int frame_id = 0;
    Frame frame_send;
    Frame frame_recv;
    
    while(1){
        int f_recv_size = tp_recvfrom(server_fd, (char *) &frame_recv, sizeof(Frame), &newAddr);

        if (f_recv_size > 0 && frame_recv.frame_kind == 1 && frame_recv.seq_no == frame_id){
            printf("Frame Received: %s\n", frame_recv.packet.data);

            frame_send.seq_no = 0;
            frame_send.frame_kind = 0;
            frame_send.ack = frame_recv.seq_no + 1;

            tp_sendto(server_fd, (char *) &frame_send, sizeof(Frame), &newAddr);
            printf("ACK Sent\n");
        } else {
            printf("Frame Not Received\n");
        }
        frame_id++;
    }

    /**/
    //fclose(File_out);   // Fecha arquivo de leitura

    //gettimeofday(&end, NULL);      // Inicia a contagem de segundos e microsegundos desde 01/01/1970 00:00:00

    // Tempo gasto é dado pela diferença da última com a primeira contagem (end - start)
    //printf("t = %.4f segundos\nL = %d bytes enviados\n", 
    //  ((end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6)), bytes_sent_total);

	return 0;
}