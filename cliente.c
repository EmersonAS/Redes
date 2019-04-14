
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // inet_addr()

#include <stdio.h>
#include <string.h>	// strcpy(), strlen(), strcat()
#include <stdlib.h>

#include <unistd.h>
#include <sys/time.h> // gettimeofday()

#define MAX_NAME_LENGTH 100


int main(int argc, char const *argv[]) {

	struct sockaddr_in server_addr;
	int socket_fd;
	int status = 0;

	const char *IP_ADDR = argv[1];
	int PORT = atoi(argv[2]);
	
	char FILE_NAME[MAX_NAME_LENGTH];
	strcpy(FILE_NAME, argv[3]);
	
	int BUFFER_SIZE = atoi(argv[4]);

	char buffer[BUFFER_SIZE];

	struct timeval start, end;

    gettimeofday(&start, NULL);

	FILE * fp;

	socket_fd = socket(AF_INET, SOCK_STREAM, 0); // teste erro -1
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDR);

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

	int bytes_recv_total = 0;
	int bytes_recv = 0;
	
	if (status != -1) {
		
		status = send(socket_fd, FILE_NAME, sizeof(FILE_NAME), 0); // Envia nome do arquivo a ser aberto: string terminada em /0
		
		char Name[] = "ClientVersion";
		fp = fopen(strcat(Name,FILE_NAME), "w+");

		if(fp == NULL){
	        printf("Erro na abertura do arquivo. Programa encerrado.\n");
	        exit(1);
	    }

		/*
		size_t fwrite ( const void * ptr, size_t size, size_t count, FILE * stream );
			Writes an array of count elements, each one with a size of size bytes, 
			from the block of memory pointed by ptr to the current position in the stream.

			The position indicator of the stream is advanced by the total number of bytes written.

			Internally, the function interprets the block pointed by ptr as if it was an array of (size*count) 
			elements of type unsigned char, and writes them sequentially to stream as if fputc was called for each byte.
		*/

		// Grava dados recebidos no arquivo de saida
		while( (status = recv(socket_fd, buffer, BUFFER_SIZE, 0)) > 0 ){
			bytes_recv = fwrite(buffer, sizeof(char), strlen(buffer)/sizeof(char), fp);
			bytes_recv_total += bytes_recv;
		}

	} else {
		printf("Conexao falhou. Programa encerrado.\n");
		exit(1);
	}

	close(socket_fd);	// fecha a conexao
	fclose(fp);			// fecha arquivo de escrita

	printf("Conexao fechada.\n");

	gettimeofday(&end,NULL);

    printf(" %.4f segundos\n %d bytes recebidos\n", 
      ((end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6)), bytes_recv_total);
	
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