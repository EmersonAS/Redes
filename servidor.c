#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <sys/time.h> 

#include <string.h>




int main() {

    struct sockaddr_in serveraddr, clientaddr;
    int tbuffer = 100;
    char buffer[tbuffer];
    int serverfd, clientfd;
    int porta = 4242; 
    int  i, flag = 0;

    struct timeval start, end;
    gettimeofday(&start,NULL);

    FILE * fp;

        /********************************************************************
       * A função socket() retorna um descritor de socket, que representa *
       * um ponto final. Vai receber um descritor de soquete com base IPV4                                    *
       ********************************************************************/

    serverfd = socket(AF_INET, SOCK_STREAM, 0);  

      /********************************************************************
       * Depois do descritor do soquete ser criado, a função bind() vai   *
       * pegar um nome único para o socket.                               *
       ********************************************************************/

    int yes = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR,&yes, sizeof(int)); // Testar o retorno dessa funcao
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port   = htons(porta);


    bind(serverfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

	// Testar o retorno dessa funcao

      /********************************************************************
       * A função listen() permite que o servidor aceite próximas conexões*
       * de clientes. O backlog 5 representa que o servidor fará um fila  *
       * de 5 solicitações de conexões de clientes até que ele comece a   *
       * recusar novas solicitações.                                      *
       ********************************************************************/

    listen(serverfd, 5);	// Testar o retorno dessa funcao
 
    socklen_t client_len = sizeof(clientaddr);
    clientfd = accept(serverfd, (struct sockaddr *) &clientaddr, &client_len ); // Testar o retorno dessa funcao

    strcpy(buffer, "Estabelecendo conexão ...\n\0");
   
    if (send(clientfd, buffer, strlen(buffer), 0)) { // Encerrar se não enviar
        printf("Conexão estabelecida! \n");

       
    //Abertura do arquivo
        do {

            int message_len;
          	if((message_len = recv(clientfd, buffer, tbuffer, 0)) > 0) {
                buffer[message_len - 1] = '\0';
                printf("Nome do arquivo recebido: %s\n", buffer);
            }

            fp = fopen (buffer, "r+");
            if(fp == NULL){
                printf("Erro abertura de arquivo! \n");   
                exit(1);             
            } else{
                break;
            }
          
        } while(1); // While até receber um nome válido de um arquivo

    //Envio dos dados do arquivo
        do {

            for (i=0; i<=tbuffer-1; i++){

                buffer[i] = fgetc(fp);

                if( feof(fp) ) { 
                    flag = 1;
                    break;
                }
            }
            if (flag == 1){
            	send(clientfd, buffer, i, 0);	// Testar o retorno dessa funcao
            	break;
            }

            send(clientfd, buffer, i, 0);	// Testar o retorno dessa funcao

            memset(buffer, 0x0, tbuffer);
           
        } while(1);
    }


//Fechando conexão 
    close(clientfd);	// ??
    close(serverfd);
    gettimeofday(&end,NULL);	// Testar o retorno dessa funcao


    printf("%ld segundos; %d bits \n", ((end.tv_sec * 1000000 + end.tv_usec)
          - (start.tv_sec * 1000000 + start.tv_usec))/1000000,i);
    

	return 0;
}

