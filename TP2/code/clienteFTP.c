
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	int socket_fd;                     			// Descritor de socket (equivalente a um HANDLE)
    so_addr server_addr;                        // Estrutura do tipo endereço de socket

    const char *IP_ADDR = argv[1];      // Endereço IP do 
    int PORT = atoi(argv[2]);
    
    char buffer_Info[BUFFER_INFO_SIZE]= {0};    // buffer p/ troca de info da conexão e nome do arquivo
    
    int BUFFER_DATA_SIZE = atoi(argv[4]);
    char buffer_Data[BUFFER_DATA_SIZE];         // buffer p/ armazenar temporariamente os dados as serem enviados/recebidos do arquivo

    //struct timeval start, end;      // Estruturas com variáveis tv_sec (tipo time_t) e tv_usec (tipo suseconds_t)

    //gettimeofday(&start, NULL);     // Inicia a contagem de segundos e microsegundos desde 01/01/1970 00:00:00

    status = tp_init();
    if (status != 0) {
        printf("tp_init failed.\n");
        exit(1);
    }

    socket_fd = tp_socket((unsigned short) PORT);
    /*
    if (socket_fd < 0) {
        printf("tp_socket failed.\n");
        exit(1);
    }
    */
    printf("Socket do servidor criado com sucesso.\n");

    tp_build_addr(&server_addr, INADDR_ANY, PORT);

    
    strcpy(buffer_Info, argv[3]);
    status = tp_sendto(socket_fd, buffer_Info, BUFFER_INFO_SIZE, &server_addr);
    

	int frame_id = 0;
    Frame frame_send;
    Frame frame_recv;
    int ack_recv = 1;

    while(1){
        if (ack_recv == 1){
            frame_send.seq_no = frame_id;
            frame_send.frame_kind = 1;
            frame_send.ack = 0;

            printf("Enter data: ");
            scanf("%s", buffer_Data);
            strcpy(frame_send.packet.data, buffer_Data);

            tp_sendto(socket_fd, (char *) &frame_send, sizeof(Frame), &server_addr);
            printf("Frame sent\n");
        }
        int addr_size = sizeof(server_addr);
        int f_recv_size = tp_recvfrom(socket_fd, (char *) &frame_recv, sizeof(Frame), &server_addr);
        
        if (f_recv_size > 0 && frame_recv.seq_no == 0 && frame_recv.ack == frame_id + 1){
            printf("ACK Received\n");
            ack_recv = 1;
        } else {
            printf("ACK Not Received\n");
            ack_recv = 1;
        }
        frame_id++;
    }

    //gettimeofday(&end, NULL);      // Inicia a contagem de segundos e microsegundos desde 01/01/1970 00:00:00

    // Tempo gasto é dado pela diferença da última com a primeira contagem (end - start)
    //printf("t = %.4f segundos\nL = %d bytes enviados\n", 
    //  ((end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6)), bytes_sent_total);

	return 0;
}