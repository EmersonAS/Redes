/**************************************************************************************
***************************************************************************************
**                          UNIVERSIDADE FEDERAL DE MINAS GERAIS
**                       Trabalho Prático 2 de Redes de Computadores
**                           ALUNO: JANDERSON BARBEITO DA SILVA
**
***************************************************************************************
**************************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "tp_socket.c"

struct pacoteBuffer{
    char ACK[5];
    char SEQ_NUM[10];
    char CHECKSUM[50];
    char* dados;
};

void logExit(const char *str){
    perror(str);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]){

    // Confere a quantidade de arqumentos recebidos na chamada do programa
	// Encerra o programa se a quantidade é inválida
    if(argc != 3){
        logExit("ARQUMENTOS");
    }

    // Declaração das variáveis
    int iniciar = tp_init();
    so_addr* clientAddr = malloc(sizeof(so_addr));
    int TAM_BUFFER = atoi(argv[2]);
    //char buffer[TAM_BUFFER];
    char parteArquivo[TAM_BUFFER];
    char cabecalho[20];
    char cabecalhoRecebido[20];
    char buffer_aux[500];
    char* nomeArquivo;
    int sBytes;
    unsigned totalBytes = 0;
    unsigned porta = atoi(argv[1]);
    int SEQ_R = 0;
    int ACK_R = 0;
    int SEQ_S = 0, ACK_S = 0;
    int tentativas = 0;
    int dadosRecebidos = 0;
    int tamCabecalho = 0;

    struct pacoteBuffer buffer;
    buffer.dados = malloc(TAM_BUFFER*sizeof(char));

    // variveis para controle timeout
    fd_set readySock;
    struct timeval timeOut;

    timeOut.tv_sec = 1; // espera por 1 segundo
    timeOut.tv_usec = 0;

    struct timeval tempoInicial;
    struct timeval tempoFinal;
    double seg, uSeg, tempoTotal;
    seg = uSeg = 0;

    if(TAM_BUFFER < 50){
        logExit("TAMANHO_BUFFER_PEQUENO");
    }

    // Criação do socket do servidor UDP
    int server_socket = tp_socket(porta);
    if(server_socket == -1 || server_socket == -2 || server_socket == -3)
        logExit("CRIAR_SOCKET");

    while(1){
        tp_recvfrom(server_socket, buffer_aux, 500, clientAddr); // recebe o nome do arquivo a ser lido

        gettimeofday(&tempoInicial, NULL); // captura o tempo inicial da operação

        nomeArquivo = malloc(100*sizeof(char));
        strcpy(nomeArquivo, buffer_aux);
        printf("\n Nome arquivo: %s", nomeArquivo);

        FILE *arq = fopen(nomeArquivo, "r"); // abri o arquivo para ler os dados
        if(arq == NULL){
            close(server_socket);
            logExit("ARQUIVO");
        }

        // transmite os dados do arquivo
        while(!feof(arq)){
            fread(parteArquivo, TAM_BUFFER, 1, arq); // captura no buffer uma quantidade dos dados a serem enviados

            memset(&buffer, 0, sizeof(buffer)); // limpar o buffer

            strcpy(buffer.dados, parteArquivo);

            sBytes = tp_sendto(server_socket, (char*)&buffer, sizeof(buffer), clientAddr);

            memset(&buffer, 0, sizeof(buffer));

            FD_ZERO(&readySock);
            FD_SET(server_socket, &readySock);

            if(select(server_socket+1, &readySock, NULL, NULL, &timeOut) < 0 ){
                logExit("SELECT");
            }
            else if(FD_ISSET(server_socket, &readySock)){
                dadosRecebidos = tp_recvfrom(server_socket, (char*)&buffer, sizeof(buffer), clientAddr);

                printf("\n\n Dados Enviados: ");
                printf("\n ACK: %s", buffer.ACK);
                printf("\n CHECKSUM: %s", buffer.CHECKSUM);
                printf("\n dados: %s", buffer.dados);
                printf("\n SEQ_NUM: %s", buffer.SEQ_NUM);

            }
            else{
                // timeout
                /*while(tentativas <= 16){ // limite de 16 tentativas de timeout
                    if(select(server_socket+1, &readySock, NULL, NULL, &timeOut) < 0 ){
                        logExit("SELECT");
                    }
                    else if(FD_ISSET(server_socket, &readySock)){
                        strcpy(buffer, cabecalho);
                        strcat(buffer, parteArquivo);
                        sBytes = tp_sendto(server_socket, buffer, strlen(buffer), clientAddr);

						strcpy(buffer, " ");

						dadosRecebidos = tp_recvfrom(server_socket, buffer, TAM_BUFFER, clientAddr);

						sscanf(buffer, "%d##%d##", &SEQ_R, &ACK_R);

						sprintf(cabecalhoRecebido, "%d##%d##", SEQ_R, ACK_R);
						tamCabecalho = strlen(cabecalhoRecebido);

						SEQ_S++;
                        tentativas = 0;
                        break;
                    }
                    else{
                        tentativas++;
                        continue;
                    }
                }*/
                printf("\n Deu ruim!");
            }
        }

        // fim da transmissão
        /*strcpy(buffer,""); // limpar o buffer
        ACK_S = -111; // sinal para finalizar conexão
        SEQ_S = -111;
        sprintf(cabecalho, "%d##%d##", SEQ_S, ACK_S); // formatar cabeçalho do pacote
        strcpy(buffer, cabecalho);
        //strcat(buffer, " ");

        sBytes = tp_sendto(server_socket, buffer, strlen(buffer), clientAddr);
        printf("buffer enviado: %s\n\n", buffer);*/

        fclose(arq);           // fecha o arquivo
        close(server_socket);  // encerra a conexão com o cliente
        free(clientAddr);
        free(nomeArquivo);

        gettimeofday(&tempoFinal, NULL); // captura o tempo final da operação

        break;
    }

    // Cálculo do tempo total de transmissão dos dados
    seg = (double)tempoFinal.tv_sec - (double)tempoInicial.tv_sec;
    uSeg = (double)tempoFinal.tv_usec - (double)tempoInicial.tv_usec;
    tempoTotal = seg + (uSeg/1000000);

    //printf("\n[SERVIDOR] >> Dados enviados!\n");
    printf("[SERVIDOR] >> Bytes enviados: %3u em %3.5f s\n", totalBytes, tempoTotal);

    exit(EXIT_SUCCESS);
}
