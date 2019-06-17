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


// Função para identificar erro de execução do programa
void logExit(const char *str){
    perror(str);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]){

	// Confere a quantidade de arqumentos recebidos na chamada do programa
	// Encerra o programa se a quantidade é inválida
	if(argc != 5){
		logExit("ARGUMENTOS");
	}

	// Declaração das variáveis do programa
	int clienteSocket;
	int TAM_BUFFER = atoi(argv[4]);     // armazena o tamnaho do buffer passado na chamada do programa
	//char buffer[TAM_BUFFER];            // buffer utilizado para a troca de mensagens
	char buffer_aux[500];
	char *ipServer = argv[1];           // endereço ip do servidor
	int porta = atoi(argv[2]);
	char cabecalho[20];
	char *nomeArquivo = argv[3];        // nome do arquivo a ser lido no servidor
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
	char* dadosRecebidos;
	int tentativas = 0;
	int tamCabecalho = 0;
	int dadosFim = 0;
	char* token;
	int exec = 0;

	struct pacoteBuffer buffer;
	buffer.dados = malloc(TAM_BUFFER*sizeof(char));

	// variáveis para controle timeout
    fd_set readySock;
    struct timeval timeOut;

    timeOut.tv_sec = 1; // espera por 1 segundo
    timeOut.tv_usec = 0;

	if(TAM_BUFFER < 50){
        logExit("TAMANHO_BUFFER_PEQUENO");
    }

    tp_init();

	// Cria e verifica a criação do socket do lado cliente
	clienteSocket = tp_socket(0);
	if(clienteSocket == -1){
		logExit("SOCKET");
	}

	tp_build_addr(sockEndereco, ipServer, porta);

	//printf("[CLIENTE] Executando...\n");
	strcpy(buffer_aux, cabecalho);

	// enviar o nome do arquivo
	if(tp_sendto(clienteSocket, buffer_aux, strlen(buffer_aux), sockEndereco) <= 0){
		logExit("SEND");
	}

	gettimeofday(&tempoInicial, NULL);

	arq = fopen("novo_arquivo.txt", "w+"); // abre o arquivo para salvar os dados vindos do servidor
	if(arq == NULL){
		logExit("ARQUIVO");
	}

	while(1){

		memset(&buffer, 0, sizeof(buffer));

		FD_ZERO(&readySock);
		FD_SET(clienteSocket, &readySock);

		if(select(clienteSocket+1, &readySock, NULL, NULL, &timeOut) < 0 ){
			logExit("SELECT");
		}
		else if(FD_ISSET(clienteSocket, &readySock)){
			nBytes = tp_recvfrom(clienteSocket, (char*)&buffer, sizeof(buffer), sockEndereco);

			printf("\n\nBUffer recebido: \n");
			printf("\n ACK: %s", buffer.ACK);
			printf("\n CHECKSUM: %s", buffer.CHECKSUM);
			printf("\n dados: %s", buffer.dados);
			printf("\n SEQ_NUM: %s", buffer.SEQ_NUM);

			/*dadosRecebidos = malloc(TAM_BUFFER*sizeof(char));
			sscanf(buffer, "%d##%d##", &SEQ_R, &ACK_R);

			if(SEQ_R == -111 && ACK_R == -111){
				break;
			}

			token = strtok(buffer, "##");
			while(token != NULL){
				if(exec = 2){
					sprintf(dadosRecebidos, "%s", token);
				}
				exec++;
				token = strtok(NULL, "##");
			}

			fwrite(dadosRecebidos, strlen(dadosRecebidos)-1, 1, arq);

			ACK_S = SEQ_R;
			sprintf(cabecalho, "%d##%d##", SEQ_S, ACK_S); //
			strcpy(buffer, "");
			strcpy(buffer, cabecalho);

			totalBytes = totalBytes + nBytes;

			if(tp_sendto(clienteSocket, buffer, strlen(buffer), sockEndereco) <= 0){
				logExit("SEND");
			}*/
		}
		else{
			// timeout
            /*while(tentativas < 16){
				if(select(clienteSocket+1, &readySock, NULL, NULL, &timeOut) < 0 ){
					logExit("SELECT");
				}
				else if(FD_ISSET(clienteSocket, &readySock)){
					strcpy(buffer, cabecalho);
					if(tp_sendto(clienteSocket, buffer, strlen(buffer), sockEndereco) <= 0){
						logExit("SEND");
					}
					tentativas = 0;
					break;
				}
				else{
					tentativas++;
					continue;
				}
            }*/
            printf("\n Não deu certo!");
		}


	}

	gettimeofday(&tempoFinal, NULL);

	fclose(arq);
	close(clienteSocket);
	free(sockEndereco);
	free(dadosRecebidos);
	free(buffer.dados);

	segTotal = tempoFinal.tv_sec - tempoInicial.tv_sec;
	uSegTotal = tempoFinal.tv_usec - tempoInicial.tv_usec;
	tempoTotal = segTotal + (uSegTotal/1000000);
	vel = ((totalBytes*8)/1000)/tempoTotal;

	printf("[CLIENTE] BUffer = %5u byte(s), %10.2f kbps (%u bytes em %3.5f s)\n", totalBytes, vel, totalBytes, tempoTotal);
	exit(EXIT_SUCCESS);
}
