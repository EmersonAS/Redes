
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <errno.h>

#include "tp_socket.h"

#define FILE_NAME_SIZE 1024	    // Tamanho máximo do buffer p/ troca de info da conexão e nome do arquivo
#define OFFSET 10               // Tamanho máximo do campo seq_no quando encapsulado com o pacote de dados

typedef struct segment {
    int seq_no;                 // Número de sequência do pacote a ser enviado
    char *ack;                  // Número do ack (reconhecimento) a ser recebido
    char *pkt_data;             // Pacote de dados a ser enviado (a ser encapsulado junto com o seq_no)
    int pkt_data_size;          // Tamanho máximo do pacote de dados
} Segment;


int main(int argc, char const *argv[]) {

	int status = 0;                             // Para verificar o retorno das funções

	int server_socket;                     	    // Descritor de socket para o servidor
    so_addr client_addr;                        // Estrutura do tipo endereço de socket

    int port = atoi(argv[1]);                   // Número da porta
    int tam_buffer = atoi(argv[2]);             // Tamanho máximo do buffer para envio do pacotes de dados (pkt_data)

    char file_Name[FILE_NAME_SIZE] = {0};       // Buffer para receber o nome do arquivo

    struct timeval start, end;                  // Estruturas com variáveis tv_sec e tv_usec que guardam a contagem do tempo
    gettimeofday(&start, NULL);                 // Inicia contagem de tempo antes do início da transferência do arquivo
    
    if (tp_init() != 0) {                       // Marca o início do programa
        printf("\ttp_init failed.\n");
        exit(1);
    }

    if ((server_socket = tp_socket((unsigned short) port)) < 0) {   // Cria socket do servidor
        printf("\ttp_socket failed.\n");
        exit(1);
    }
    printf("\tserver socket created.\n");

    FILE * File_read;           // Ponteiro para o arquivo a ser lido
    int bytes_sent_total = 0;   // Inicializa contador do total de bytes lidos do arquivo (a serem enviados)
    int bytes_sent = 0;         // Inicializa a contagem de bytes lidos do arquivo

	if ((tp_recvfrom(server_socket, file_Name, FILE_NAME_SIZE, &client_addr)) < 0) { // Recebe o nome do arquivo do cliente
        printf("\tfile name not received.\n");
        exit(1);
    }
    printf("\tfile name received: %s\n", file_Name);
    
    File_read = fopen(file_Name, "r");          // Abre o arquivo como somente leitura
  	if(File_read == NULL){
  	    printf("\tcould not open file.\n");
  	    exit(1);
  	}
    
    // Inicializa a estrutura do tipo Segment alocando memória para pkt_data de acordo com tam_buffer e OFFSET

    Segment *segment_send = (Segment *) malloc(sizeof(Segment));
    segment_send->pkt_data = (char *) malloc(tam_buffer * sizeof(char) + OFFSET + 1);
    segment_send->ack = (char *) malloc(30*sizeof(char));

    char buffer[tam_buffer];        // Buffer para leitura dos dados do arquivo

    // Protocolo Stop-and-Wait com retranmissão de pkt (por temporização)

    struct timeval temporizador;    // Declara temporizador
    temporizador.tv_sec = 1;        // timeout_s = 1 segundo
    temporizador.tv_usec = 0;       // timeout_us = 0 microsegundos

    // SO_RCVTIMEO : aceita uma struct "timeval" que indica o número em segundos e microsegundos a serem esperados até
    // que uma função se complete.
    if(setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &temporizador, sizeof(temporizador)) < 0){
        printf("\tsetsockopt failed\n");
        exit(1);
    }
    printf("\tsetsockopt ok\n");

    int segment_id = 0;             // Numero de sequencia do primeiro pacote a ser recebido
    int data_to_read = 1;           // Variável para controlar o loop de leitura e envio dos dados
    int wait_for_ack = 1;           // Variável para controlar o loop de espera por um ack

    while (data_to_read) {          // Enquanto houver dados para ler e enviar ao cliente

        char tmp[OFFSET];                       // Armazena o valor de seq_no atual como uma string
        sprintf(tmp, "%d", segment_id);         // Converte o valor de int para string e coloca em tmp - Ex: "0"
        strcat(segment_send->pkt_data, tmp);    // Concatena o conteúdo de tmp com o de pkt_data (que inicialmente está vazio)
        strcat(segment_send->pkt_data, ":");    // Concatena ":" com o conteúdo de pkt_data - Ex: "0:\0"

        if ((bytes_sent = fread(buffer, sizeof(char), tam_buffer - 1, File_read)) <= 0) {
            data_to_read = 0;
        }

        // Acrescenta o caratere de terminação '\0' no fim da string
        buffer[((bytes_sent < tam_buffer)? bytes_sent: tam_buffer)] = '\0';

        strcat(segment_send->pkt_data, buffer);     // 
        
        bytes_sent_total += bytes_sent;             // Atualiza total de bytes lidos

        //if (ack_recv == 1) {

        segment_send->pkt_data_size = strlen(segment_send->pkt_data);

        tp_sendto(server_socket, segment_send->pkt_data, segment_send->pkt_data_size, &client_addr);
            
        wait_for_ack = 1; // 

        printf("\tpkt sent: seq_no = %d\n", segment_id);

        //}

        if (data_to_read == 0) {
            wait_for_ack = 0;
        }

        while(wait_for_ack) {

            status = tp_recvfrom(server_socket, segment_send->ack, (30*sizeof(char)), &client_addr);

            if (status > 0) {
                if (atoi(segment_send->ack) == segment_id + 1) {
                    printf("\tACK_no = %d received\n", atoi(segment_send->ack));
                    wait_for_ack = 0;
                } else {
                    printf("\tACK_no = %d not received\n", segment_id + 1);
                    wait_for_ack = 1;
                }
            } else {
                if (errno == EWOULDBLOCK) {
                    printf("\ttimeout event\n");
                    tp_sendto(server_socket, segment_send->pkt_data, segment_send->pkt_data_size, &client_addr);
                    printf("\tpkt resent: seq_no = %d\n", segment_id);
                    wait_for_ack = 1;
                } else {
                    printf("\terror %d: %s\n", errno, strerror(errno));
                    exit(1);
                }
            }
        }
        
        memset(buffer, 0x0, strlen(buffer));
        memset(segment_send->pkt_data, 0x0, strlen(segment_send->pkt_data));

        segment_id++;
    }
    
    fclose(File_read);              // Fecha arquivo de leitura

    free(segment_send->pkt_data);   // Desaloca memória para as variáveis membro e a strutura
    free(segment_send->ack);
    free(segment_send);

    gettimeofday(&end, NULL);       // Inicia contagem de tempo após o término da transferência dos dados

    printf("\nBuffer = %d byte(s)\nL = %d bytes enviados\nt = %.5f segundos\nR = L/t = %10.2f byte(s)/s\n\n", tam_buffer, bytes_sent_total, 
        ((end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6)), 
        (float) bytes_sent_total/((end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6)));

	return 0;
}
