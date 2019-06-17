/**************************************************************************************
***************************************************************************************
**                          UNIVERSIDADE FEDERAL DE MINAS GERAIS
**                       Trabalho Prático 3 de Redes de Computadores
**                           ALUNO: JANDERSON BARBEITO DA SILVA
**
***************************************************************************************
**************************************************************************************/

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
// Função para identificar erro de execução do programa


int main(){

	// Confere a quantidade de arqumentos recebidos na chamada do programa
	// Encerra o programa se a quantidade é inválida


	// Declaração das variáveis do programa
	int clienteSocket;
	int TAM_BUFFER =100;     // armazena o tamnaho do buffer passado na chamada do programa
	char buffer[TAM_BUFFER+100];            // buffer utilizado para a troca de mensagens
	char buffer_aux[500];
	char *ipServer = "127.0.0.1";           // endereço ip do servidor
	int porta = 4242;
	char cabecalho[100];
	char *nomeArquivo = "4000.txt";        // nome do arquivo a ser lido no servidor
	unsigned totalBytes = 0;            // total de bytes recebidos do servidor
	int nBytes;                         // armazena a quantidade de bytes
	FILE *arq;                          // posição de memória em que começa o arquivo que irá conter os dados recebidos do servidor
	struct timeval tempoInicial;        // tempo inicial, onde inicia a comunicação com o servidor
	struct timeval tempoFinal;          // tempo final, onde encerra a comunicação com o servidor
	double segTotal = 0;                // total de segundos decorridos na operação
	double uSegTotal = 0;               // total de microsegundos decorridos da operação
	double tempoTotal = 0;              // tempo total da operação segundos + microsegundos, em segundos
	double vel = 0;                     //velocidade de trasmissão dos dados trafegados kbps
	so_addr* sockEndereco = malloc(sizeof(so_addr));

	int SEQ_R = 0, ACK_R = 0;
	int SEQ_S = 0, ACK_S = 0;
	int CHECK_SUM = 0;
	int SEQ_PACKT = -1;
	char* dadosRecebidos;
	int tentativas = 0;
	int tamCabecalho = 0;
	int dadosFim = 0;
	char* token;
	int exec = 0;
	int tamJanela = 5;
	int pctEsperado = 0;
	int closeConnection = 0;
	char teste;

	// variáveis para controle timeout
    fd_set readySock;
    struct timeval timeOut;

    timeOut.tv_sec = 1; // espera por 1 segundo
    timeOut.tv_usec = 0;



    tp_init();

	// Cria e verifica a criação do socket do lado cliente
	clienteSocket = tp_socket(0);
	if(clienteSocket == -1){
		exit(1);
	}

	tp_build_addr(sockEndereco, ipServer, porta);

	strcpy(buffer_aux, nomeArquivo);

	printf("\nNomeArquivo enviado: %s", buffer_aux);

	if(tp_sendto(clienteSocket, buffer_aux, 500, sockEndereco) <= 0){
		exit(1);
	}

	printf("\nBuffer enviado: %s", buffer_aux);

	arq = fopen("novo_arquivo.txt", "w+"); // abre o arquivo para salvar os dados vindos do servidor
	if(arq == NULL){
		exit(1);
	}

	if(setsockopt(clienteSocket, SOL_SOCKET, SO_RCVTIMEO, &timeOut, sizeof(timeOut)) < 0)
        exit(1);

	while(1){

		/// implementação com setsockopt
		for(int i = 0; i < tamJanela; i++){
			printf("\nEsperando receber pacotes... \n");
			memset(buffer, 0, sizeof(buffer));
			nBytes = tp_recvfrom(clienteSocket, buffer, sizeof(buffer), sockEndereco);

			if(nBytes < 0){
				//ocorreu timeout
				if(tentativas == 16){break;} // condição de parada de retransmisão
				printf("\nOcorreu timeout, retransmitindo ack\n");
				tentativas++;
				ACK_S = pctEsperado;
				sprintf(cabecalho, "%d##%d##", SEQ_S, ACK_S); //
				memset(buffer, 0, sizeof(buffer));
				strcpy(buffer, cabecalho);



				if(tp_sendto(clienteSocket, buffer, strlen(buffer), sockEndereco) < 0){
					exit(1);
				}

				printf("\nbuffer enviado: %s\n", buffer);

				printf("\nPause depois de renviar ACK\n");
				teste = getchar();
				getchar();
				continue;
			}

			printf("\nBuffer %d recebido: %s \n", i, buffer);

			dadosRecebidos = malloc(TAM_BUFFER*sizeof(char));
			sscanf(buffer, "%d##%d##%d##", &SEQ_R, &ACK_R, &CHECK_SUM);

			if(SEQ_R == -111 && ACK_R == -111 & CHECK_SUM == 0){ // condição de parada da transmissão
				printf("\nEncerrando conexão pelo servidor... \n");
				closeConnection = 1;
				break;
			}

			token = strtok(buffer, "##");
			while(token != NULL){
				if(exec = 3){
					sprintf(dadosRecebidos, "%s", token);
				}
				exec++;
				token = strtok(NULL, "##");
			}

			printf("\n CHECK SUM: %d \n tamanho dados: %d\n", CHECK_SUM, (int)strlen(dadosRecebidos));

			printf("\nPause janela de transmissao recv \n");
			teste = getchar();
			getchar();

			if(CHECK_SUM == strlen(dadosRecebidos) && SEQ_R == pctEsperado){
				printf("\nDados recebidos com sucesso\n");
				tentativas = 0;
				pctEsperado++;
				ACK_S = pctEsperado;
				fwrite(dadosRecebidos, strlen(dadosRecebidos)-1, 1, arq);
				totalBytes = totalBytes + nBytes;
				printf("\nProximo ACK: %d PCT esperado: %d\n", ACK_S, pctEsperado);
			}
			else{
				// ocorreu erro em algum dos pacotes recebidos, os outros pacotes serão descartados
				// até o fim do laço da janela
				ACK_S = pctEsperado;
				tentativas = 0;
				printf("\nErro no pacote Valor de ACK: %d \n", ACK_S);
				printf("\nDescartar pacote\n");
				teste = getchar();
				getchar();
			}
		}

		printf("\nPause depois da janela antes de send\n");
		teste = getchar();
		getchar();

		if(tentativas == 16 || closeConnection == 1){break;}

		sprintf(cabecalho, "%d##%d##", SEQ_S, ACK_S);
		memset(buffer, 0, sizeof(buffer));
		strcpy(buffer, cabecalho);

		if(tp_sendto(clienteSocket, buffer, strlen(buffer), sockEndereco) < 0){
			exit(1);
		}

		printf("\nbuffer enviado: %s\n", buffer);

		printf("\nPause depois de send do ACK\n");
		getchar();
		getchar();
	}


	printf("\nPause depois de while (fim programa)\n");
	getchar();
	getchar();

	gettimeofday(&tempoFinal, NULL);

	fclose(arq);
	close(clienteSocket);
	free(sockEndereco);
	free(dadosRecebidos);

	segTotal = tempoFinal.tv_sec - tempoInicial.tv_sec;
	uSegTotal = tempoFinal.tv_usec - tempoInicial.tv_usec;
	tempoTotal = segTotal + (uSegTotal/1000000);
	vel = ((totalBytes*8)/1000)/tempoTotal;

	printf("[CLIENTE] BUffer = %5u byte(s), %10.2f kbps (%u bytes em %3.5f s)\n", totalBytes, vel, totalBytes, tempoTotal);
	exit(EXIT_SUCCESS);
}
