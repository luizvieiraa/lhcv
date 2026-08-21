#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

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

        } else if (strcmp(tokens[0], "run") == 0) {

    if (quantidade_tokens < 2) {
        printf("Erro: uso correto: run <nome>\n");
        continue;
    }

    if (strcmp(tokens[1], "sequential") == 0) {

        if (quantidade_tokens < 3) {
            printf(
                "Erro: uso correto: "
                "run sequential <tarefa1> [tarefa2...]\n"
            );
            continue;
        }

        for (int i = 2; i < quantidade_tokens; i++) {

            Task *task = buscar_task(
                tarefas,
                quantidade_tasks,
                tokens[i]
            );

            if (task == NULL) {

                printf(
                    "Erro: task '%s' não encontrada.\n",
                    tokens[i]
                );

                break;
            }

            executar_task(task);
        }

    } else {

        if (quantidade_tokens != 2) {
            printf("Erro: uso correto: run <nome>\n");
            continue;
        }

        Task *task = buscar_task(
            tarefas,
            quantidade_tasks,
            tokens[1]
        );

        if (task == NULL) {
            printf(
                "Erro: task '%s' não encontrada.\n",
                tokens[1]
            );
            continue;
        }

        executar_task(task);
    }
} else {

            printf("Comando desconhecido.\n");
        }
    }

    return 0;
}