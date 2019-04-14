
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdio.h>
#include <string.h>		// strcpy(), strlen()
#include <stdlib.h>   // exit()

#include <unistd.h>   // close(socket_fd)
#include <sys/time.h> // gettimeofday()

#define BACKLOG 5     // Tamanho máximo da fila de conexões pendentes


int main(int argc, char *argv[]) {

    struct sockaddr_in server_addr, client_addr;  // Estruturas do tipo endereço de socket
    int server_fd, client_fd;                     // Descritor de socket (equivalente a um HANDLE)
    int status = 0;                               // Para verificar o retorno das funções

    int PORT = atoi(argv[1]);
    int BUFFER_SIZE = atoi(argv[2]);

    char buffer[BUFFER_SIZE];     // buffer para armazenar tempoariamente as msgs enviadas/recebidas

    struct timeval start, end;    // Estruturas com variáveis tv_sec (tipo time_t) e tv_usec (tipo suseconds_t)

    gettimeofday(&start, NULL);   // Inicia a contagem de segundos e microsegundos desde 01/01/1970 00:00:00

    /****************************************************************************
    * A função socket() retorna um descritor de socket, que representa um       *
    * ponto final (end point), com base IPV4.                                   *
    ****************************************************************************/

    // Domínio de comunicação: AF_INET (protocolo IPv4)
    // Tipo de comunicação: SOCK_STREAM: TCP (Reliable, Connection oriented)
    // Valor do Protocolo para o IP: 0

    printf("Inicializando o servidor.\n");

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
                                                
    if (server_fd == -1) {
        printf("Estabelecimento do socket falhou.\n");
        exit(1);
    }
    printf("Socket do servidor criado com sucesso.\n");

    /****************************************************************************
    * Manipula as opções do socket referenciado por server_fd. Ajuda a detectar *
    * se uma porta/endereço já estão em uso (Opcional).                         *
    ****************************************************************************/

    int yes = 1;
    status = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    
    if (status == -1) {
        printf("Alterar socket options falhou.\n");
        exit(1);
    }

    /********************************************************************
    * Depois da criação do socket, é nessário associá-lo a um endereço  *
    * (Host address e Port). A função bind() faz essa associação.       *
    ********************************************************************/

    // Inicializa os campos da struct server_addr do tipo sockaddr com 0's
    memset(&server_addr, 0, sizeof(server_addr));

    // Preenche os campos da struct server_addr (do tipo socketaddr)
    // AF_INET: (Protocolo IPv4)
    // htons: converte variável PORT de host byte order para network byte order
    // INADDR_ANY: Qualquer addr, pois será conectado ao host local (Opcional, pois memset() já havia deixado esse campo com 0.0.0.0)

    server_addr.sin_family = AF_INET;		      
    server_addr.sin_port   = htons(PORT);	    
    server_addr.sin_addr.s_addr = INADDR_ANY; 

    status = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

	if (status == -1) {
	    printf("Binding socket to addr falhou.\n");
        exit(1);
    }

    /************************************************************************************
    * A função listen() coloca o servidor em modo passivo, aguardando que um cliente    *
    * faça a requição p/ uma conexão por meio de accept(). Backlog define o tamanho     *
    * máximo da fila de conexões pendentes. O servidor encherá uma fila de 5 requsições *
    * de conexões de clientes até que ele comece a recusar novas solicitações.          *
    ************************************************************************************/

    status = listen(server_fd, BACKLOG);

    if (status == -1) {
        printf("Listening process falhou.\n");
        exit(1);
    }
    printf("Escutando na porta: %d\n", PORT);
    
    /************************************************************************************
    * Accept() extrai a primeira requisição de conexão da fila para o socket que está   *
    * ouvindo (listening), o servidor (server_fd), cria um novo socket conectado e      *
    * retorna uma file descriptor referenciando esse socket (client_fd). server_fd deve *
    * ser um socket criado com socket(), associado a um addr local com bind() e deve    *
    * está escutando (recebendo) novas conexões após a chamada da função listen().      *
    ************************************************************************************/

    socklen_t client_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_len);

    if (client_fd == -1) {
        printf("Accept connection falhou.\n");
        exit(1);
    }
    
    /*****************************************************************************************
    * send() é usada para transmitir msgs a outro socket. Para ser enviada, a msg é colocada *
    * em um buffer de tamanho strlen(buffer). Retorna a qtde de bytes enviados em caso de    *
    * sucesso.                                                                               *
    *                                                                                        *
    * int strlen(const char *str): calcula o tamanho de uma string, sem considerar o /0.     *
    *****************************************************************************************/

    strcpy(buffer, "Estabelecendo conexão...\n\0");
    status = send(client_fd, buffer, strlen(buffer), 0);

    FILE * File_out;            // Ponteiro para o arquivo a ser lido
    int bytes_sent_total = 0;   // Inicializa contador do total de bytes lidos do arquivo (enviados)
    int bytes_sent = 0;         // Inicializa a contagem de bytes lidos do arquivo

    if (status != -1) {
        printf("Conexão estabelecida!\n");

        /**********************************************************************************
		* recv() recebe uma msg de um socket (já conectado), retorna a qtde de bytes      *
        * recebidos, e os armazena em um buffer de tamanho BUFFER_SIZE.                   *
        **********************************************************************************/

        int file_name_length = recv(client_fd, buffer, BUFFER_SIZE, 0);

        if(file_name_length > 0) {
            
            buffer[file_name_length - 1] = '\0';	             // Indica o fim da msg recebida
            printf("Nome do arquivo recebido: %s\n", buffer);
            
            File_out = fopen(buffer, "r");	// r+: open a file to update both reading and writing
        	
  	        if(File_out == NULL){
  	            printf("Erro na abertura do arquivo a ser enviado.\n");
  	            exit(1);
  	        }

            memset(buffer, 0, BUFFER_SIZE);   // Zera o buffer

            // Lê o proximo pedaço da msg no arquivo, o coloca em buffer e testa se o arquivo acabou
            while( (bytes_sent = fread(buffer, sizeof(char), (BUFFER_SIZE-1)/sizeof(char), File_out)) > 0 ){
                
                buffer[BUFFER_SIZE - 1] = '\0';          // Para indicar o término da string
                
                status = send(client_fd, buffer, BUFFER_SIZE, 0);  // Envia os bytes lidos
                
                if (status == -1) {
                    printf("Erro ao enviar dados.\n");
                    exit(1);
                }

                memset(buffer, 0, BUFFER_SIZE);   // Zera o buffer
                bytes_sent_total += bytes_sent;   // Atualiza total de bytes lido/enviado
            }

        } else {
            printf("Nome de arquivo inválido.\n");
        }

    } else {
        printf("Conexão falhou.\n");
        exit(1);
    }

    fclose(File_out);   // Fecha arquivo de leitura

    close(client_fd);   // Fecha conexão com o socket cliente
    close(server_fd);   // Fecha socket servidor
    
    printf("Conexão fechada.\n");

    gettimeofday(&end,NULL);      // Inicia a contagem de segundos e microsegundos desde 01/01/1970 00:00:00

    // Tempo gasto é dado pela diferença da última com a primeira contagem (end - start)
    printf("t = %.4f segundos\nL = %d bytes enviados\n", 
      ((end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6)), bytes_sent_total);
        
	return 0;
}
