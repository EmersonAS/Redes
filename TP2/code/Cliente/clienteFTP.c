
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "tp_socket.h"

#define FILE_NAME_SIZE 1024	    // Tamanho máximo do buffer p/ envio do nome do arquivo
#define OFFSET 10               // Tamanho máximo do campo seq_no quando encapsulado com o pacote de dados

typedef struct segment {
    int seq_no;					// Número de sequência do pacote a ser enviado
    char *ack;					// Número do ack (reconhecimento) a ser recebido
    char *pkt_data;				// Pacote de dados a ser enviado (a ser encapsulado junto com o seq_no)
    int pkt_data_size;			// Tamanho máximo do pacote de dados
} Segment;


int main(int argc, char const *argv[]) {

	int status = 0;                             // Para verificar o retorno das funções
	
	int client_socket, server_socket;           // Descritores de socket para cliente e servidor
    so_addr server_addr;                        // Estrutura do tipo endereço de socket

    char *ip_addr = (char *) argv[1];           // Endereço IP do servidor
    int port = atoi(argv[2]);                   // Número da porta
    
    int tam_buffer = atoi(argv[4]);             // Tamanho máximo do buffer para envio do pacotes de dados (pkt_data)

    char file_Name[FILE_NAME_SIZE]= {0};        // Buffer para envio do nome do arquivo

    struct timeval start, end;      			// Estruturas com variáveis tv_sec e tv_usec que guardam a contagem do tempo
    gettimeofday(&start, NULL);     			// Inicia contagem de tempo antes do início da transferência do arquivo

    if (tp_init() != 0) {						// Marca o início do programa
        printf("tp_init failed.\n");
        exit(1);
    }

    if ((client_socket = tp_socket(0)) < 0) {	// Cria socket do cliente
        printf("tp_socket failed.\n");
        exit(1);
    }
    printf("\tclient socket created.\n");

    server_socket = tp_build_addr(&server_addr, ip_addr, port);		// Constroi addr e linka com o servidor
    
    strcpy(file_Name, argv[3]);
    status = tp_sendto(client_socket, file_Name, strlen(file_Name), &server_addr);	// Envia nome do arquivo para o servidor

    FILE * File_write;          // Ponteiro para o arquivo a ser gravado com os dados recebidos do servidor
    int bytes_recv_total = 0;   // Inicializa contador do total de bytes gravados no arquivo (recebidos)
    int bytes_recv = 0;         // Inicializa a contagem de bytes gravados no arquivo
    
    char Name[] = "client_version_";					// Nome do arquivo no cliente: client_version_NOME_DO_ARQUIVO_NO_SERVIDOR
    File_write = fopen(strcat(Name, file_Name), "w+");	// Abre o arquivo no modo de escrtia

    // Inicializa a estrutura do tipo Segment alocando memória para pkt_data de acordo com tam_buffer e OFFSET

    Segment *segment = (Segment *) malloc(sizeof(Segment));
    segment->pkt_data = (char *) malloc(tam_buffer * sizeof(char) + OFFSET + 1);
    segment->ack = (char *) malloc(30*sizeof(char));
    
    segment->pkt_data_size = tam_buffer * sizeof(char) + OFFSET + 1;	// Tamanho máximo do pacote de dados junto com o seq_no

    // Protocolo Stop-and-Wait

    int segment_id = 0;				// Numero de sequencia do primeiro pacote a ser recebido
    int data_to_recv = 1;			// Variável para controlar o loop de recebimento dos dados e gravação dos dados

    while (data_to_recv) {			// Enquanto houver dados para receber
        
        status = tp_recvfrom(client_socket, segment->pkt_data, segment->pkt_data_size, &server_addr);

        // Desempacota o seq_no e os dados (que estão no formato seq_no:dados...)
        segment->seq_no = atoi(strtok(segment->pkt_data, ":"));						// Extrai tudo que estiver antes do caractere ":" e converte para int
        char *buffer = strtok(NULL, "\0");											// Extrai tudo que estiver após ":" e antes de "\0"

        if (buffer != NULL) {														// Se o pacote de dados não estiver vazio
            if (segment->seq_no <= segment_id) {									// Verifica se o seq_no é o do próximo pkt esperado ou de um pkt antigo
                
                printf("\tpkt received: seq_no = %d\n", segment->seq_no);			// Indica o recebimento do pkt e seu número de sequência
                
                if (segment->seq_no == segment_id) {										// Se o seq_no for o do próximo pkt esperado 
	                bytes_recv = fwrite(buffer, sizeof(char), strlen(buffer), File_write);  // Grava no arquivo de saída os dados recebidos
	                bytes_recv_total += bytes_recv;     									// Atualiza total de bytes recebidos/gravado
	                segment_id++;															// Indica que esperará agora pelo próximo pkt
                }
                
                sprintf(segment->ack, "%d", segment->seq_no + 1);							// Prepara o ack para envio (converte de int para char*)
                tp_sendto(client_socket, segment->ack, strlen(segment->ack), &server_addr);	// Envia o ack do último pkt válido recebido
                printf("\tACK_no = %d sent\n", atoi(segment->ack));                			// Indica o envio do ack
            
            } else {																// Senão, um pacote com seq_no maior do que o esperado foi recebido
                printf("\tpkt not received\n");										// Indica que o pkt esperado ainda não foi recebido, data_to_recv continua 1
            }
            
            memset(buffer, 0x0, strlen(buffer));									// Limpa o buffer de recebimento dos dados
        
        } else {																	// Senão, se o pacote de dados estiver vazio
            data_to_recv = 0;        												// Indica que os dados acabaram - Interrompe o loop de recebimento de dados
            printf("\tdata transfer finished.\n");
        }
    }

    fclose(File_write);         // Fecha arquivo de gravação
    
    free(segment->pkt_data);	// Desaloca memória para as variáveis membro e a strutura
    free(segment->ack);
    free(segment);

    gettimeofday(&end, NULL);   // Inicia contagem de tempo após o término da transferência dos dados

    printf("\nBuffer = %d byte(s)\nL = %d bytes recebidos\nt = %.5f segundos\nR = L/t = %10.2f byte(s)/s\n\n", tam_buffer, bytes_recv_total, 
    	((end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6)), 
    	(float) bytes_recv_total/((end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6)));

	return 0;
}
