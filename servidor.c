
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdio.h>
#include <string.h>		// strcpy(), strlen()
#include <stdlib.h>   // exit()

#include <unistd.h>   // close(socket_fd)
#include <sys/time.h> // gettimeofday()

#define BACKLOG 5


int main(int argc, char *argv[]) {

    struct sockaddr_in serveraddr, clientaddr;
    int serverfd, clientfd;
    int status = 0;

    // Address format: An IP socket address is defined as a combination...
    // ...of an IP interface address and a 16-bit port number

    /*
	struct sockaddr_in {
        sa_family_t    sin_family; // address family: AF_INET
        in_port_t      sin_port;   // port in network byte order
        struct in_addr sin_addr;   // internet address
    };

    // Internet address

    struct in_addr {
        uint32_t	s_addr;			// address in network byte order
    };
 
    */

    int PORT = atoi(argv[1]);
    int BUFFER_SIZE = atoi(argv[2]);

    char buffer[BUFFER_SIZE];

    struct timeval start, end;

    gettimeofday(&start, NULL);

    /*
    start and end são structs do tipo timeval

    struct timeval {
      time_t      tv_sec;     // seconds
      suseconds_t tv_usec;    // microseconds
    };
    
    gives the number of seconds and microseconds since the Epoch

    Epoch: 1970-01-01 00:00:00 +0000 (UTC).

    int gettimeofday(struct timeval *tv, struct timezone *tz);

    */

    FILE * fp;

       /********************************************************************
       * A função socket() retorna um descritor de socket, que representa *
       * um ponto final (end point). Vai receber um descritor de soquete com base IPV4                                    *
       ********************************************************************/

    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (serverfd == -1)
    {
    	printf("Estabelecimento do socket falhou! Programa encerrado.\n");
    	exit(1);
    }


    /* Set the socket options for what ???? */

    int yes = 1;
    status = setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    
    if (status == -1)
    {
    	printf("Socket options falhou. Programa encerrado.\n");
    	exit(1);
    }

       /********************************************************************
       * Depois do descritor do socket ser criado, a função bind() vai   *
       * pegar um nome único para o socket.                               *
       ********************************************************************/

    //printf("%s\n", );
    memset(&serveraddr, 0, sizeof(serveraddr));	// preenche os campos da struct serveraddr do tipo sock addr com 0's, return void
    //printf("%s\n", );

    // preenche os campos da struct socket addr serveraddr
    serveraddr.sin_family = AF_INET;		// AF_INET: IPv4 Internet protocols
    serveraddr.sin_port   = htons(PORT);	// htons: converts the unsigned short integer PORT from host byte order to network byte order

    /*
    When a socket is created with socket(...), it exists in a name space
       (address family) but has no address assigned to it. bind() assigns
       the address specified by serveraddr to the socket referred to by the file
       descriptor serverfd. size(serveraddr) specifies the size, in bytes, of the
       address structure pointed to by serveraddr
    */

    status = bind(serverfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

	  if (status == -1) {
		  printf("Binding process falhou. Programa encerrado.\n");
    	exit(1);
    }

      /********************************************************************
       * A função listen() permite que o servidor aceite próximas conexões*
       * de clientes. Backlog representa que o servidor fará um fila  *
       * de 5 solicitações de conexões de clientes até que ele comece a   *
       * recusar novas solicitações.                                      *
       ********************************************************************/

	   /*
	   listen(): listen for connections on a socket - marks the socket referred to by serverfd as a passive socket,
       that is, as a socket that will be used to accept incoming connection requests using accept().
	   */

    status = listen(serverfd, BACKLOG);

    if (status == -1)
    {
    	printf("Listening process falhou. Programa encerrado.\n");
    	exit(1);
    }
 
    /* accept a connection on a socket
    The accept() system call is used with connection-based socket types
       (SOCK_STREAM, SOCK_SEQPACKET). It extracts the first connection
       request on the queue of pending connections for the listening socket,
       sockfd, creates a new connected socket, and returns a new file
       descriptor referring to that socket
	
	The argument serverfd is a socket that has been created with socket(),
       bound to a local address with bind(), and is listening for
       connections after a listen() call.
	
	The argument clientaddr is a pointer to a sockaddr_in structure. This
       structure is filled in with the address of the peer socket (the client), as known
       to the communications layer

    The client_len argument is a value-result argument: the caller must
       initialize it to contain the size (in bytes) of the structure pointed
       to by clientaddr; on return it will contain the actual size of the peer
       address.

    */

    socklen_t client_len = sizeof(clientaddr);
    
    clientfd = accept(serverfd, (struct sockaddr *) &clientaddr, &client_len);

    if (clientfd == -1)
    {
    	printf("accept connection falhou! Programa encerrado.\n");
    	exit(1);
    }

    /* char* strcpy(char* dest, const char* src)
	
    dest: Pointer to the destination array where the content is to be copied.
    src: string which will be copied.

	*/
    

    /* send(): used to send/transmit a message to another socket. 
	
	The send() call may be used only when the socket is in a connected
       state (so that the intended recipient is known)

    For send() and sendto(), the message is found in buffer and has length len.

    If the message is too long to pass atomically through the underlying
       protocol, the error EMSGSIZE is returned, and the message is not
       transmitted.
	
    On success, these calls return the number of bytes sent. On error,
       -1 is returned

    ssize_t send(int sockfd, const void *buffer, size_t len, int flags);
	
    */

    //ssize_t bytes_sent = 0;	// signed size_t == ssize_t
    // int strlen(const char *str): calculates the length of a given string, returns this length

    strcpy(buffer, "Estabelecendo conexão...\n\0");
    status = send(clientfd, buffer, strlen(buffer), 0);

    int bytes_sent_total = 0;
    int bytes_sent = 0;

    if (status != -1) {
        printf("Conexão estabelecida! \n");

        /*
		
		Receive a message from a socket
		The recv() call is normally used only on a connected socket
       	return the number of bytes received, or -1 if an error occurred
			
		ssize_t recv(int sockfd, void *buffer, size_t length_buffer, int flags);

        */

        //memset(buffer, 0, BUFFER_SIZE);
        int file_name_length;
        file_name_length = recv(clientfd, buffer, BUFFER_SIZE, 0);

        if(file_name_length > 0) {
            //buffer[file_name_length - 1] = '\0';	// to indicate the end of the msg received
            printf("Nome do arquivo recebido: %s\n", buffer);
            
            fp = fopen(buffer, "r");	// r+: open a file to update both reading and writing
        	
  	        if(fp == NULL){
  	            printf("Erro na abertura do arquivo. Programa encerrado. \n");
  	            exit(1);
  	        }

	          // Envia o conteudo do arquivo recebido

      	        /*
      			Function fread():
      			size_t fread(void *ptr, size_t size, size_t n, FILE *fp);
      			sizeof(char) = 1 byte;
      	        */
            memset(buffer, 0, BUFFER_SIZE);
            while( (bytes_sent = fread(buffer, sizeof(char), (BUFFER_SIZE-1)/sizeof(char), fp)) > 0 ){  // Lê o proximo pedaço da msg e coloca em buffer
              buffer[BUFFER_SIZE-1] = '\0';
              status = send(clientfd, buffer, BUFFER_SIZE, 0);     // Envia o buffer lido
              memset(buffer, 0, BUFFER_SIZE);
              bytes_sent_total += bytes_sent;
            }

        } else {
          printf("Nome de arquivo invalido.\n");
        }

    } else {
    	printf("Conexão falhou. Programa encerrado.\n");
    	exit(1);
    }


// Fecha a conexão

    close(clientfd);  // fecha socket cliente
    close(serverfd);  // fecha socket servidor
    fclose(fp);       // fecha arquivo de leitura

    printf("Conexao fechada.\n");

    gettimeofday(&end,NULL);

    printf(" %.4f segundos\n %d bytes enviados\n", 
      ((end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6)), bytes_sent_total);
        
	return 0;
}



/* REFERENCES

1 - memset: 			https://www.geeksforgeeks.org/memset-c-example/
2 - setsockopt: 		https://linux.die.net/man/3/setsockopt
3 - socket: 			http://man7.org/linux/man-pages/man2/socket.2.html
4 - Linux IPv4: 		http://man7.org/linux/man-pages/man7/ip.7.html
5 - htons():  http://man7.org/linux/man-pages/man3/htons.3.html
6 - bind: 				http://man7.org/linux/man-pages/man2/bind.2.html
7 - listen: 			http://man7.org/linux/man-pages/man2/listen.2.html
8 - send:				http://man7.org/linux/man-pages/man2/send.2.html
9 - accept:				http://man7.org/linux/man-pages/man2/accept.2.html
10 - receive: 			http://man7.org/linux/man-pages/man2/recv.2.html
11 - fread:				https://www.geeksforgeeks.org/fread-function-in-c/
12 - connect:			http://man7.org/linux/man-pages/man2/connect.2.html
13 - gettimeofday: http://man7.org/linux/man-pages/man2/gettimeofday.2.html

*/