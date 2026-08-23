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

        /*
         * COMANDO TASK
         */
        if (strcmp(tokens[0], "task") == 0) {

            cadastrar_task(
                tarefas,
                &quantidade_tasks,
                tokens,
                quantidade_tokens
            );
        }

        /*
         * COMANDO INPUT
         */
        else if (strcmp(tokens[0], "input") == 0) {

            if (quantidade_tokens != 3) {
                printf(
                    "Erro: uso correto: input <tarefa> <arquivo>\n"
                );
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

            definir_input(
                task,
                tokens[2]
            );
        }

        /*
         * COMANDO OUTPUT
         */
        else if (strcmp(tokens[0], "output") == 0) {

            if (quantidade_tokens != 3) {
                printf(
                    "Erro: uso correto: output <tarefa> <arquivo>\n"
                );
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

            definir_output(
                task,
                tokens[2],
                0
            );
        }

        /*
         * COMANDO APPEND
         */
        else if (strcmp(tokens[0], "append") == 0) {

            if (quantidade_tokens != 3) {
                printf(
                    "Erro: uso correto: append <tarefa> <arquivo>\n"
                );
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

            definir_output(
                task,
                tokens[2],
                1
            );
        }

        /*
         * COMANDO RUN
         */
        else if (strcmp(tokens[0], "run") == 0) {

            if (quantidade_tokens < 2) {
                printf("Erro: uso correto: run <nome>\n");
                continue;
            }

            /*
             * RUN SEQUENTIAL
             */
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
            }

            /*
             * RUN PARALLEL
             */
            else if (strcmp(tokens[1], "parallel") == 0) {

                if (quantidade_tokens < 3) {
                    printf(
                        "Erro: uso correto: "
                        "run parallel <tarefa1> [tarefa2...]\n"
                    );
                    continue;
                }

                pid_t pids[MAX_TOKENS];
                int quantidade_pids = 0;

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
                        continue;
                    }

                    pid_t pid = iniciar_task(task);

                    if (pid != -1) {
                        pids[quantidade_pids] = pid;
                        quantidade_pids++;
                    }
                }

                for (int i = 0; i < quantidade_pids; i++) {

                    int status;

                    if (waitpid(
                        pids[i],
                        &status,
                        0
                    ) == -1) {

                        perror("waitpid");
                    }
                }
            }

            /*
             * RUN PIPE
             */
            else if (strcmp(tokens[1], "pipe") == 0) {

                if (quantidade_tokens < 4) {
                    printf(
                        "Erro: uso correto: "
                        "run pipe <tarefa1> <tarefa2> [tarefa3...]\n"
                    );
                    continue;
                }

                int quantidade_pipe =
                    quantidade_tokens - 2;

                Task *tasks_pipe[MAX_TOKENS];

                int erro = 0;

                for (
                    int i = 0;
                    i < quantidade_pipe;
                    i++
                ) {

                    tasks_pipe[i] = buscar_task(
                        tarefas,
                        quantidade_tasks,
                        tokens[i + 2]
                    );

                    if (tasks_pipe[i] == NULL) {

                        printf(
                            "Erro: task '%s' não encontrada.\n",
                            tokens[i + 2]
                        );

                        erro = 1;
                        break;
                    }
                }

                if (erro) {
                    continue;
                }

                executar_pipe(
                    tasks_pipe,
                    quantidade_pipe
                );
            }

            /*
             * RUN SIMPLES
             */
            else {

                if (quantidade_tokens != 2) {
                    printf(
                        "Erro: uso correto: run <nome>\n"
                    );
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
        }

        /*
         * COMANDO DESCONHECIDO
         */
        else {

            printf("Comando desconhecido.\n");
        }
    }

    return 0;
}