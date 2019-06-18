#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include<stdint.h>
#include "tp_socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <strings.h>



void logExit(const char *str){
    perror(str);
    exit(EXIT_FAILURE);
}

int main(){

    // Confere a quantidade de arqumentos recebidos na chamada do programa
	// Encerra o programa se a quantidade é inválida
 

    // Declaração das variáveis
    int iniciar = tp_init();
    so_addr* clientAddr = malloc(sizeof(so_addr));
    int TAM_BUFFER = 1000;
    char buffer[TAM_BUFFER+100];
    char parteArquivo[TAM_BUFFER];
    char cabecalho[100];
    char cabecalhoRecebido[100];
    char buffer_aux[500];
    char* nomeArquivo;
    int sBytes;
    unsigned totalBytes = 0;
    unsigned porta = 4242;
    int SEQ_R = 0;
    int ACK_R = 0;
    int pctBase = 0;
    int SEQ_S = 0, ACK_S = 0;
    int tentativas = 0;
    int dadosRecebidos = 0;
    int tamCabecalho = 0;
    int tamJanela = 5;
    int tamArquivo = 0;
    int numPct = 0;
    int k = 0;
    char* token;

    // variveis para controle timeout
    struct timeval timeOut;

    timeOut.tv_sec = 1; // espera por 1 segundo
    timeOut.tv_usec = 0;

    struct timeval tempoInicial;
    struct timeval tempoFinal;
    double seg, uSeg, tempoTotal;
    seg = uSeg = 0;

    // Criação do socket do servidor UDP
    int server_socket = tp_socket(porta);
    if(server_socket == -1 || server_socket == -2 || server_socket == -3)
        logExit("CRIAR_SOCKET");


    while(1){
        memset(buffer_aux, 0, sizeof(buffer_aux));
        tp_recvfrom(server_socket, buffer_aux, 500, clientAddr); // recebe o nome do arquivo a ser lido

        gettimeofday(&tempoInicial, NULL); // captura o tempo inicial da operação

        nomeArquivo = malloc(100*sizeof(char));
        strcpy(nomeArquivo, buffer_aux);

        printf("\n buffer recebido: %s", buffer_aux);

        FILE *arq = fopen(nomeArquivo, "r"); // abri o arquivo para ler os dados
        if(arq == NULL){
            close(server_socket);
            logExit("ARQUIVO");
        }

        fseek(arq, 0, SEEK_END);

        tamArquivo = ftell(arq); // tamanho do arquivo a ser lido

        numPct = tamArquivo/TAM_BUFFER;
        if(tamArquivo%TAM_BUFFER){
            numPct++;
        }

        fclose(arq);

        arq = fopen(nomeArquivo, "r");

        printf("\nTamanho arquivo: %d", numPct);

        char* vetorPct[numPct];

        for(int c = 0; c < numPct; c++){
            vetorPct[c] = malloc(TAM_BUFFER*sizeof(char));
            memset(vetorPct[c], 0, sizeof(vetorPct[c]));
        }

        while(!feof(arq) || k < numPct){
            fread(parteArquivo, TAM_BUFFER, 1, arq);
            strcpy(vetorPct[k], parteArquivo);
            k++;
        }

        k = 0;

        if(setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, &timeOut, sizeof(timeOut)) < 0){
            logExit("SETSOCKETOPT");
        }

        // transmite os dados do arquivo
        while(tentativas < 16 || k == 5){
            k++;
            for(int i = 0; i < tamJanela; i++){
                memset(buffer, 0, sizeof(buffer));
                sprintf(cabecalho, "%d##%d##%d##", SEQ_S, ACK_S, (int)strlen(vetorPct[SEQ_S])); // formatar cabeçalho do pacote

                strcpy(buffer, cabecalho);
                strcat(buffer, vetorPct[SEQ_S]);

                SEQ_S++;

                sBytes = tp_sendto(server_socket, buffer, strlen(buffer), clientAddr);

                printf("\n Buffer %d enviado: %s\n", i, buffer);

                printf("\nPause janela de transmissao send\n");
                getchar();
                getchar();
            }

            printf("\nPause depois da janela de transmissao antes de recv\n");

            getchar();
            getchar();

            /// implementação timeout com setsockopt

            memset(buffer, 0, sizeof(buffer));

            if(tp_recvfrom(server_socket, buffer, sizeof(buffer), clientAddr) < 0){
                // ocorreu timeout reenviar os pacotes;
                tentativas++;
                SEQ_S = pctBase;
                printf("\nPacote Base: %d", pctBase);
                printf("\nPause timeout recv\n");
                getchar();
                getchar();
                continue;
            }

            sscanf(buffer, "%d##%d##", &SEQ_R, &ACK_R);

            printf("\nbuffer recebido: %s\n", buffer);
            printf("\nValores recebidos de SEQ: %d e ACK: %d\n", SEQ_R, ACK_R);

            printf("\nPause depois recfrom\n");
            getchar();
            getchar();

            if(ACK_R == SEQ_S+1){
                printf("\n\n Sucesso ACK \n\n");
                pctBase = SEQ_S++;
                printf("\n Numero seq pct: %d\n", SEQ_S);
                tentativas = 0;

                getchar();
                getchar();
            }
            else{
                if(ACK_R > 0 && ACK_R < numPct){
                    printf("\nPacotes perdidos, reenviar desde: %d\n", ACK_R);
                    pctBase = ACK_R;
                    SEQ_S = pctBase;
                    tentativas = 0;

                    getchar();
                    getchar();
                }
                else{
                    printf("\nVoltando do inicio\n");
                    SEQ_S = 0; // inicia transmissão do começo
                    pctBase = 0;
                    tentativas++;

                    getchar();
                    getchar();
                }
            }
        }

        printf("\n\n Pause fim do envio de dados \n");
        getchar();
        getchar();

        // fim da transmissão
        memset(buffer, 0, sizeof(buffer));
        ACK_S = -111; // sinal para finalizar conexão
        SEQ_S = -111;
        sprintf(cabecalho, "%d##%d##%d##", SEQ_S, ACK_S, 0); // formatar cabeçalho do pacote
        strcpy(buffer, cabecalho);
        //strcat(buffer, " ");

        sBytes = tp_sendto(server_socket, buffer, strlen(buffer), clientAddr);
        printf("buffer enviado: %s\n\n", buffer);

        fclose(arq);           // fecha o arquivo
        close(server_socket);  // encerra a conexão com o cliente
        free(clientAddr);
        free(nomeArquivo);
        for(int c = 0; c < numPct; c++){
            free(vetorPct[c]);
        }

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
