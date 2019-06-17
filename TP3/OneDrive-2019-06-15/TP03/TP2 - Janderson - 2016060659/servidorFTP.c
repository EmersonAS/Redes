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
    char buffer[TAM_BUFFER];
    char parteArquivo[TAM_BUFFER];
    char cabecalho[20];
    char cabecalhoRecebido[20];
    char buffer_aux[1000];
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
        tp_recvfrom(server_socket, buffer, TAM_BUFFER, clientAddr); // recebe o nome do arquivo a ser lido

        gettimeofday(&tempoInicial, NULL); // captura o tempo inicial da operação

        nomeArquivo = malloc(100*sizeof(char));
        sscanf(buffer, "%d##%d##%[^\n]", &SEQ_R, &ACK_R, nomeArquivo);
        sprintf(cabecalhoRecebido, "%d##%d##", SEQ_R, ACK_R);
        
        tamCabecalho = strlen(cabecalhoRecebido);

        FILE *arq = fopen(nomeArquivo, "r"); // abri o arquivo para ler os dados
        if(arq == NULL){
            close(server_socket);
            logExit("ARQUIVO");
        }

        // transmite os dados do arquivo
        while(!feof(arq)){
            fread(parteArquivo, TAM_BUFFER-tamCabecalho, 1, arq); // captura no buffer uma quantidade dos dados a serem enviados

            strcpy(buffer," "); // limpar o buffer
            sprintf(cabecalho, "%d##%d##", SEQ_S, ACK_S); // formatar cabeçalho do pacote
            if(strlen(cabecalho) > 20){ // confere se o tamanho do cabeçalho é maior que o permitido, caso seja gera erro
                logExit("CABECALHO_GRANDE");
            }
            strcpy(buffer, cabecalho);
            strcat(buffer, parteArquivo);

            sBytes = tp_sendto(server_socket, buffer, strlen(buffer), clientAddr);

            strcpy(buffer, " "); // limpar o buffer para receber os dados do recvfrom

            FD_ZERO(&readySock);
            FD_SET(server_socket, &readySock);

            if(select(server_socket+1, &readySock, NULL, NULL, &timeOut) < 0 ){
                logExit("SELECT");
            }
            else if(FD_ISSET(server_socket, &readySock)){
                dadosRecebidos = tp_recvfrom(server_socket, buffer, TAM_BUFFER, clientAddr);

                sscanf(buffer, "%d##%d##", &SEQ_R, &ACK_R);

                sprintf(cabecalhoRecebido, "%d##%d##", SEQ_R, ACK_R);
                tamCabecalho = strlen(cabecalhoRecebido);
                strcpy(buffer, " ");
                SEQ_S++;
            }
            else{
                // timeout
                while(tentativas <= 16){ // limite de 16 tentativas de timeout
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
                }
            }
        }

        // fim da transmissão
        strcpy(buffer,""); // limpar o buffer
        ACK_S = -111; // sinal para finalizar conexão
        SEQ_S = -111;
        sprintf(cabecalho, "%d##%d##", SEQ_S, ACK_S); // formatar cabeçalho do pacote
        strcpy(buffer, cabecalho);
        //strcat(buffer, " ");

        sBytes = tp_sendto(server_socket, buffer, strlen(buffer), clientAddr);
        printf("buffer enviado: %s\n\n", buffer);

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
