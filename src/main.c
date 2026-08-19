#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){

    char comando[100];

    while(1){
        printf("processflow> ");
        //faz o prompt ser mostrado na hora
        fflush(stdout);

        //le o comando e coloca dentro de comando
        fgets(comando, sizeof(comando), stdin);

        //remove /n
        comando[strcspn(comando, "\n")] = '\0';

        if(strcmp(comando, "exit") == 0){
            break;
        }
    }

    return 0;
}