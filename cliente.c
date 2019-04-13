#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdio.h>
#include <string.h>	// strcpy, strlen, strcat
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <sys/time.h> 

#include <netinet/in.h>
#include <arpa/inet.h> // inet_addr

#define BUFFER_SIZE 100
#define PORT 4242
#define ADDR_SERVER "127.0.0.1"


int main(int argc, char const *argv[]) {

	struct sockaddr_in server_addr;
	int socket_fd;

	char buffer[BUFFER_SIZE];

	char FILE_NAME[] = "arquivoTeste2.txt";

	int status = 0;

	FILE * fp;

	socket_fd = socket(AF_INET, SOCK_STREAM, 0); // teste erro -1
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr(ADDR_SERVER);

	/*
	IP ADDR OF SERVER HOST:

	There are several special addresses: INADDR_LOOPBACK (127.0.0.1)
    always refers to the local host via the loopback device; INADDR_ANY
    (0.0.0.0) means any address for binding; INADDR_BROADCAST
    (255.255.255.255) means any host and has the same effect on bind as
    INADDR_ANY for historical reasons.
	
	inet_addr: Internet address manipulation routine

	in_addr_t inet_addr(const char *cp);

	The inet_addr() function converts the Internet host address ADDR_SERVER from 
	IPv4 numbers-and-dots notation into binary data in network byte order. 
	
	If the input is invalid, INADDR_NONE (usually -1) is returned. Use of 
	this function is problematic because -1 is a valid address 
	(255.255.255.255). Avoid its use in favor of inet_aton(), inet_pton(3), 
	or getaddrinfo(3) which provide a cleaner way to indicate error return. 

	*/

	// setsockopt(); NO NEED BECAUSE THE SERVER ALREADY RESERVED AND 
	//            ...IS LISTENNING ON THE PORT WE WANT TO COMMUNICATE


	// bind(); NO NEED - WHY??

	socklen_t server_addr_length = sizeof(server_addr);

	status = connect(socket_fd, (struct sockaddr *) &server_addr, server_addr_length);  // teste erro -1

	status = recv(socket_fd, buffer, BUFFER_SIZE, 0); // receive presentation msg from server // teste erro -1

	if (status != -1) {
		
		status = send(socket_fd, FILE_NAME, sizeof(FILE_NAME), 0); // Envia nome do arquivo a ser aberto: string terminada em /0
		
		char Name[] = "ClientVersion";
		fp = fopen(strcat(Name,FILE_NAME), "w+");

		if(fp == NULL){
	        printf("Erro na abertura do arquivo. Programa encerrado.\n");
	        exit(1);
	    }
	    /*
	    int msg_length = 0;
	    
		msg_length = recv(socket_fd, buffer, BUFFER_SIZE, 0); // rcv content
		*/
		/*
		size_t fwrite ( const void * ptr, size_t size, size_t count, FILE * stream );
			Writes an array of count elements, each one with a size of size bytes, 
			from the block of memory pointed by ptr to the current position in the stream.

			The position indicator of the stream is advanced by the total number of bytes written.

			Internally, the function interprets the block pointed by ptr as if it was an array of (size*count) 
			elements of type unsigned char, and writes them sequentially to stream as if fputc was called for each byte.
		*/
	    /*
		// Grava conteudo do arquivo recebido
		while(msg_length > 0){
			fwrite(buffer, 1, BUFFER_SIZE-1, fp);		// testar
			//buffer[sizeof(buffer)-1] = '\0';
			//memset(buffer, 0x0, BUFFER_SIZE); 						// zera o buffer
			msg_length = recv(socket_fd, buffer, BUFFER_SIZE, 0); 	// rcv content
		}
		*/
		//memset(buffer, 0x0, BUFFER_SIZE); // zera o buffer
		int tam = 0;
		while( (recv(socket_fd, buffer, BUFFER_SIZE, 0)) > 0 ){
			tam = fwrite(buffer, sizeof(char), strlen(buffer)/sizeof(char), fp);
			//memset(buffer, 0, BUFFER_SIZE);
			printf("%d\n", tam);
		}

		fclose(fp);	// fecha arquivo de escrita

	} else {
		printf("Conexao falhou. Programa encerrado.\n");
		exit(1);
	}

	close(socket_fd);
	printf("Conexao fechada.\n");
	
	return 0;
}

/* REFERENCES

1 - inet_addr: 			https://linux.die.net/man/3/inet_addr
2 - fwrite:				http://www.cplusplus.com/reference/cstdio/fwrite/
3 - 					https://www.geeksforgeeks.org/readwrite-structure-file-c/
4 - fwrite strlen:		http://forums.codeguru.com/showthread.php?452056-fwrite()-writes-garbage-at-the-end-of-the-file
5 - string em C:		https://www.geeksforgeeks.org/strings-in-c-2/
6 - strlen:				http://www.cplusplus.com/reference/cstring/strlen/
						https://www.geeksforgeeks.org/strlen-function-in-c/

*/