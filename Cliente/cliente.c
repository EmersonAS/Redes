
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // inet_addr()

#include <stdio.h>
#include <string.h>	// strcpy(), strlen(), strcat()
#include <stdlib.h>

#include <unistd.h>
#include <sys/time.h> // gettimeofday()

#define NAME_LENGTH 100		// Tamanho máximo do nome do arquivo


int main(int argc, char const *argv[]) {

	struct sockaddr_in server_addr; 	// Estruturas do tipo endereço de socket
	int socket_fd;						// Descritor de socket (equivalente a um HANDLE)
	int status = 0;                     // Para verificar o retorno das funções

	const char *IP_ADDR = argv[1];
	int PORT = atoi(argv[2]);
	
	char FILE_NAME[NAME_LENGTH] = {0};
	strcpy(FILE_NAME, argv[3]);
	
	int BUFFER_SIZE = atoi(argv[4]);

	char buffer[BUFFER_SIZE];		// buffer para armazenar tempoariamente as msgs enviadas/recebidas

	struct timeval start, end;		// Estruturas com variáveis tv_sec (tipo time_t) e tv_usec (tipo suseconds_t)

    gettimeofday(&start, NULL);		// Inicia a contagem de segundos e microsegundos desde 01/01/1970 00:00:00

    /****************************************************************************
    * A função socket() retorna um descritor de socket, que representa um       *
    * ponto final (end point), com base IP4.                                    *
    ****************************************************************************/

    // Domínio de comunicação: AF_INET (protocolo IPv4)
    // Tipo de comunicação: SOCK_STREAM: TCP (Reliable, Connection oriented)
    // Valor do Protocolo para o IP: 0

    printf("Inicializando o cliente.\n");

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (socket_fd == -1) {
		printf("Estabelecimento so socket falhou.\n");
        exit(1);
	}
	printf("Socket do cliente criado com sucesso.\n");

	/********************************************************************************************
	* connect() conecta o socket referenciado por socket_fd ao endereço do servidor server_addr *
	********************************************************************************************/

	// Preenche os campos da struct server_addr (do tipo socketaddr)
    // AF_INET: (Protocolo IPv4)
    // htons: converte variável PORT de host byte order para network byte order
    // inet_addr(IP_ADDR): converte da notação de números e pontos p/ Notação binária em network byte order
    // Ex: IP_ADDR = 127.0.0.1 se refere ao host local via loopback device (equivalente a INADDR_LOOPBACK)

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDR);

	socklen_t server_addr_length = sizeof(server_addr);
	status = connect(socket_fd, (struct sockaddr *) &server_addr, server_addr_length);

	if (status == -1) {
		printf("Conexão com o servidor falhou.\n");
        exit(1);
	}

    /**********************************************************************************
	* recv() recebe uma msg de um socket (já conectado), retorna a qtde de bytes      *
    * recebidos, e os armazena em um buffer de tamanho BUFFER_SIZE.                   *
    **********************************************************************************/

	status = recv(socket_fd, buffer, BUFFER_SIZE, 0); // Recebe apres. inicial do servidor
	printf("Servidor: %s", buffer);

    memset(buffer, 0, BUFFER_SIZE);   // Limpa o buffer

	FILE * File_in;				// Ponteiro para o arquivo a ser gravado
	int bytes_recv_total = 0;   // Inicializa contador do total de bytes gravados no arquivo (recebidos)
	int bytes_recv = 0;			// Inicializa a contagem de bytes gravados no arquivo
	
	if (status != -1) {
		printf("Conexão estabelecida.\n");

		/*****************************************************************************************
    	* send() é usada para transmitir msgs a outro socket. Para ser enviada, a msg é colocada *
    	* em um buffer de tamanho sizeof(buffer). Retorna a qtde de bytes enviados em caso de    *
    	* sucesso.														                         *
    	*****************************************************************************************/

		// Envia nome do arquivo a ser aberto: string terminada em /0
		if (BUFFER_SIZE < strlen(FILE_NAME))
		{
			char *FILE_NAME_Ptr = FILE_NAME;
			int bytes_left = strlen(FILE_NAME);
			int bytes_to_copy = 0;

			while (bytes_left > 0){
				
				bytes_to_copy = (bytes_left > BUFFER_SIZE + 1) ? BUFFER_SIZE-1 : bytes_left;
				memcpy(buffer, FILE_NAME_Ptr, bytes_to_copy);
				FILE_NAME_Ptr += bytes_to_copy;
				bytes_left -= bytes_to_copy;
				
				//buffer[bytes_to_copy] = '\0';

				status = send(socket_fd, buffer, strlen(buffer), 0);

				if (status == -1) {
	            	printf("Erro ao enviar pedaço do nome do arquivo.\n");
	            	exit(1);
	        	}

	        	memset(buffer, 0, BUFFER_SIZE);   // Limpa o buffer

			}
		} else {
			
			status = send(socket_fd, FILE_NAME, sizeof(FILE_NAME), 0);

			if (status == -1) {
	           	printf("Erro ao enviar o nomde completo do arquivo.\n");
	           	exit(1);
	        }

		}
        printf("Nome do arquivo enviado.\n");

		char Name[] = "ClientVersion";
		
		File_in = fopen(strcat(Name,FILE_NAME), "w+");

		if(File_in == NULL){
	        printf("Erro na abertura do arquivo para gravação.\n");
	        exit(1);
	    }

		// Grava dados recebidos no arquivo de saida
		while( (status = recv(socket_fd, buffer, BUFFER_SIZE, 0)) > 0 ){
			
			if (status == -1) {
                printf("Erro ao receber dados.\n");
                exit(1);
            }

            // Grava no arquivo de saída o conteúdo atual (válido - Até o '\0') de buffer
			bytes_recv = fwrite(buffer, sizeof(char), strlen(buffer)/sizeof(char), File_in);
			bytes_recv_total += bytes_recv;		// Atualiza total de bytes recebidos/gravado
		}

	} else {
		printf("Conexão falhou.\n");
		exit(1);
	}

	fclose(File_in);	// Fecha arquivo de escrita

	close(socket_fd);	// Fecha a conexão com o socket
	
	printf("Conexão fechada.\n");

	gettimeofday(&end,NULL);	// Inicia a contagem de segundos e microsegundos desde 01/01/1970 00:00:00

	// Tempo gasto é dado pela diferença da última com a primeira contagem (end - start)
    printf("t = %.4f segundos\nL = %d bytes recebidos\n", 
      ((end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6)), bytes_recv_total);
	
	return 0;
}
