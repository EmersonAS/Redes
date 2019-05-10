
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>

#include "tp_socket.h"

#define FILE_NAME_SIZE 1024	// Tamanho máximo do buffer p/ envio do nome do arquivo


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
	int client_socket, server_socket;           // Descritor de socket (equivalente a um HANDLE)
    so_addr server_addr;                        // Estrutura do tipo endereço de socket

    char *IP_ADDR = (char *) argv[1];           // Endereço IP do 
    int PORT = atoi(argv[2]);
    
    char file_Name[FILE_NAME_SIZE]= {0};    // buffer p/ envio do nome do arquivo
    
    int BUFFER_DATA_SIZE = atoi(argv[4]);
    
    char buffer_Data[BUFFER_DATA_SIZE];      // EXCLUIR   // buffer p/ armazenar temporariamente os dados as serem enviados/recebidos do arquivo

    //struct timeval start, end;      // Estruturas com variáveis tv_sec (tipo time_t) e tv_usec (tipo suseconds_t)

    //gettimeofday(&start, NULL);     // Inicia a contagem de segundos e microsegundos desde 01/01/1970 00:00:00

    if (tp_init() != 0) {
        printf("tp_init failed.\n");
        exit(1);
    }

    if ((client_socket = tp_socket(0)) < 0) {
        printf("tp_socket failed.\n");
        exit(1);
    }
    printf("\tclient socket created.\n");

    server_socket = tp_build_addr(&server_addr, IP_ADDR, PORT);

    // Envia nome do arquivo para o servidor
    strcpy(file_Name, argv[3]);
    status = tp_sendto(client_socket, file_Name, strlen(file_Name), &server_addr);






    /*
    FILE * File_write;          // Ponteiro para o arquivo a ser gravado
    int bytes_recv_total = 0;   // Inicializa contador do total de bytes gravados no arquivo (recebidos)
    int bytes_recv = 0;         // Inicializa a contagem de bytes gravados no arquivo
    
    char Name[] = "ClientVersion";
    File_write = fopen(strcat(Name, file_Name), "w+");

                    // Grava no arquivo de saída o conteúdo atual de buffer (somente os bytes válidos)
            bytes_recv = fwrite(buffer_Data, sizeof(char), strlen(buffer_Data), File_write);
            bytes_recv_total += bytes_recv;     // Atualiza total de bytes recebidos/gravado

    */




    /* LÓGICA DO STOP-AND-WAIT */

    int segment_id = 0;
    Segment segment_send;
    Segment segment_recv;
    //segment_recv.packet.data = malloc(BUFFER_DATA_SIZE);
    //segment_send.packet.data = malloc(BUFFER_DATA_SIZE);
    
    while(1){
        int f_recv_size = tp_recvfrom(client_socket, (char *) &segment_recv, sizeof(Segment), &server_addr);

        if (f_recv_size > 0 && segment_recv.seq_no == segment_id){
            printf("Segment Received: %s\n", segment_recv.packet.data);

            segment_send.seq_no = 0;
            segment_send.ack = segment_recv.seq_no + 1;

            tp_sendto(client_socket, (char *) &segment_send, sizeof(Segment), &server_addr);
            printf("ACK Sent\n");
        } else {
            printf("Segment Not Received\n");
        }
        segment_id++;
    }

    fclose(File_write);   // Fecha arquivo de gravação

    //gettimeofday(&end, NULL);      // Inicia a contagem de segundos e microsegundos desde 01/01/1970 00:00:00

    // Tempo gasto é dado pela diferença da última com a primeira contagem (end - start)
    //printf("t = %.4f segundos\nL = %d bytes enviados\n", 
    //  ((end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6)), bytes_sent_total);

    //free(segment_recv.packet.data);
    //free(segment_send.packet.data);

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

    if (errno == EWOULDBLOCK) {
        printf("\tError: %s\n", strerror(errno));
        printf("\tError number: %d\n", errno);
    }
    */