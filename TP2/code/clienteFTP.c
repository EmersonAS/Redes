
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "tp_socket.h"

#define FILE_NAME_SIZE 1024	// Tamanho máximo do buffer p/ envio do nome do arquivo
#define OFFSET 10               // 

typedef struct segment {
    int seq_no;
    char *ack;
    char *pkt_data;
    int pkt_data_size;
} Segment;


int main(int argc, char const *argv[]) {

	int status = 0;                             // Para verificar o retorno das funções
	int client_socket, server_socket;           // Descritor de socket (equivalente a um HANDLE)
    so_addr server_addr;                        // Estrutura do tipo endereço de socket

    char *IP_ADDR = (char *) argv[1];           // Endereço IP do 
    int PORT = atoi(argv[2]);
    
    char file_Name[FILE_NAME_SIZE]= {0};        // buffer p/ envio do nome do arquivo
    
    int tam_buffer = atoi(argv[4]);

    struct timeval start, end;      // Estruturas com variáveis tv_sec (tipo time_t) e tv_usec (tipo suseconds_t)
    gettimeofday(&start, NULL);     // Inicia a contagem de segundos e microsegundos desde 01/01/1970 00:00:00

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

    FILE * File_write;          // Ponteiro para o arquivo a ser gravado
    int bytes_recv_total = 0;   // Inicializa contador do total de bytes gravados no arquivo (recebidos)
    int bytes_recv = 0;         // Inicializa a contagem de bytes gravados no arquivo
    
    char Name[] = "client_version_";
    File_write = fopen(strcat(Name, file_Name), "w+");

    /* LÓGICA DO STOP-AND-WAIT */

    int segment_id = 0;
    Segment *segment_send = (Segment *) malloc(sizeof(Segment));
    Segment *segment_recv = (Segment *) malloc(sizeof(Segment));

    segment_send->pkt_data = (char *) malloc(tam_buffer * sizeof(char) + OFFSET + 1);
    segment_send->ack = (char *) malloc(sizeof(char));

    segment_recv->pkt_data = (char *) malloc(tam_buffer * sizeof(char) + OFFSET + 1);
    segment_recv->ack = (char *) malloc(sizeof(char));
    
    segment_recv->pkt_data_size = tam_buffer * sizeof(char) + OFFSET + 1;

    //char buffer[tam_buffer];

    int data_to_recv = 1;

    while(data_to_recv) {

        status = tp_recvfrom(client_socket, segment_recv->pkt_data, segment_recv->pkt_data_size, &server_addr);

        segment_recv->seq_no = atoi(strtok(segment_recv->pkt_data, ":"));
        // printf("%d\n\n", segment_recv->seq_no);                              // DELETE

        char *buffer = strtok(NULL, "\0");
        
        //strcpy(buffer, );
        //printf("%s", buffer);                                            // DELETE
        
        printf("strcmp(buffer, NULL): %d\n", (buffer == NULL));

        //char tmp[];
        //sprintf(tmp, "%d", segment_recv->seq_no);
        //strcat(tmp, ":");

        if (buffer != NULL) {

            if (segment_recv->seq_no == segment_id) {

                //printf("\tpkt received:\n%s\n", segment_recv.packet.data);                                // EXCLUIR
                printf("\tpkt received\n");
                
                // Grava no arquivo de saída o conteúdo atual de buffer (somente os bytes válidos)
                bytes_recv = fwrite(buffer, sizeof(char), strlen(buffer), File_write);

                bytes_recv_total += bytes_recv;     // Atualiza total de bytes recebidos/gravado

                //segment_send->seq_no = 0;                                                                 // DELETE
                sprintf(segment_send->ack, "%d", segment_recv->seq_no + 1);

                printf("segment_send->ack: %s\n", segment_send->ack);                                       // DELETE

                tp_sendto(client_socket, segment_send->ack, sizeof(segment_send->ack), &server_addr);
                
                printf("\tACK sent\n");

            } else {
                
                printf("\tpkt not received\n");
            
            }

            memset(buffer, 0x0, strlen(buffer));

        } else {
            
            data_to_recv = 0;
            printf("data_to_recv = 0\n");
        
        }

        //

        printf("%s\n", "fim do while()");       // DELETE

        segment_id++;
    }

    fclose(File_write);   // Fecha arquivo de gravação
    
    free(segment_send->pkt_data);
    free(segment_recv->pkt_data);
    free(segment_send->ack);
    free(segment_recv->ack);
    free(segment_send);
    free(segment_recv);

    gettimeofday(&end, NULL);      // Inicia a contagem de segundos e microsegundos desde 01/01/1970 00:00:00

    // Tempo gasto é dado pela diferença da última com a primeira contagem (end - start)
    printf("t = %.4f segundos\nL = %d bytes recebidos\n", 
      ((end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6)), bytes_recv_total);

	return 0;
}
