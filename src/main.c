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
                printf("Erro: task '%s' não encontrada.\n", tokens[1]);
                continue;
            }

            /*
             * A partir daqui entra fork(), execv() e waitpid().
             */

            pid_t pid = fork();

            if (pid == -1) {

                perror("fork");
                continue;

            } else if (pid == 0) {

                /*
                 * PROCESSO FILHO
                 */

                execv(task->programa, task->argumentos);

                /*
                 * Se chegou aqui, o execv() falhou.
                 */

                perror("execv");
                return 1;

            } else {

                /*
                 * PROCESSO PAI
                 */

                int status;

                waitpid(pid, &status, 0);
            }

        } else {

            printf("Comando desconhecido.\n");
        }
    }

    return 0;
}