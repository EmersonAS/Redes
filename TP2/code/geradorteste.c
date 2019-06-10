    #include <stdio.h>
    #include <stdlib.h>
    int main(void)
    {
      FILE *pont_arq; // cria variável ponteiro para o arquivo
      char palavra[20]; // variável do tipo string
      int i;
      //abrindo o arquivo com tipo de abertura w
      pont_arq = fopen("1000.txt", "w");
      
      //testando se o arquivo foi realmente criado
      if(pont_arq == NULL)
      {
      printf("Erro na abertura do arquivo!");
      return 1;
      }
      
      printf("Escreva uma palavra para testar gravacao de arquivo: ");
      scanf("%s", palavra);
      i=0;
      while (i<400){
      //usando fprintf para armazenar a string no arquivo
      fprintf(pont_arq, "%s", palavra);
      i=i+1;
      }
      //usando fclose para fechar o arquivo
      fclose(pont_arq);
      
      printf("Dados gravados com sucesso!");
      
      return(0);
    }