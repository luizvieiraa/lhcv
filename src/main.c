#include <stdio.h>
#include <string.h>

#include "task.h"

#define MAX_INPUT 500
#define MAX_TOKENS 30

int main() {

    Task tarefas[MAX_TASKS];
    int quantidade_tasks = 0;

    char comando[MAX_INPUT];

    while (1) {

        printf("processflow> ");
        fflush(stdout);

        if (fgets(comando, sizeof(comando), stdin) == NULL) {
            break;
        }

        comando[strcspn(comando, "\n")] = '\0';

        if (strcmp(comando, "exit") == 0) {
            break;
        }

        if (strlen(comando) == 0) {
            continue;
        }

        char *tokens[MAX_TOKENS];
        int quantidade_tokens = 0;

        char *token = strtok(comando, " ");

        while (token != NULL && quantidade_tokens < MAX_TOKENS) {

            tokens[quantidade_tokens] = token;
            quantidade_tokens++;

            token = strtok(NULL, " ");
        }

        if (quantidade_tokens == 0) {
            continue;
        }

        if (strcmp(tokens[0], "task") == 0) {

            cadastrar_task(
                tarefas,
                &quantidade_tasks,
                tokens,
                quantidade_tokens
            );

        } else {

            printf("Comando desconhecido.\n");
        }
    }

    return 0;
}