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

        // Lê o comando digitado pelo usuário
        if (fgets(comando, sizeof(comando), stdin) == NULL) {
            break;
        }

        // Remove o \n do final
        comando[strcspn(comando, "\n")] = '\0';

        // Comando exit
        if (strcmp(comando, "exit") == 0) {
            break;
        }

        // Ignora linha vazia
        if (strlen(comando) == 0) {
            continue;
        }

        // Divide o comando em tokens
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
         *
         * Exemplo:
         * task listar /bin/ls -l
         */
        if (strcmp(tokens[0], "task") == 0) {
            cadastrar_task(
                tarefas,
                &quantidade_tasks,
                tokens,
                quantidade_tokens
            );
        }
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

            definir_input(task, tokens[2]);
        }
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
             *
             * Exemplo:
             * run sequential listar data processos
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

                    /*
                     * executar_task() faz:
                     *
                     * fork()
                     * execv()
                     * waitpid()
                     *
                     * Por isso a execução é sequencial.
                     */
                    executar_task(task);
                }
            }

            /*
             * RUN PARALLEL
             *
             * Exemplo:
             * run parallel listar data processos
             */
            else if (strcmp(tokens[1], "parallel") == 0) {
                if (quantidade_tokens < 3) {
                    printf(
                        "Erro: uso correto: "
                        "run parallel <tarefa1> [tarefa2...]\n"
                    );

                    continue;
                }

                /*
                 * Guarda os PIDs dos processos filhos.
                 */
                pid_t pids[MAX_TOKENS];
                int quantidade_pids = 0;

                /*
                 * Primeiro criamos TODOS os processos.
                 */
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

                    /*
                     * iniciar_task() cria o processo,
                     * mas NÃO espera ele terminar.
                     */
                    pid_t pid = iniciar_task(task);

                    if (pid != -1) {
                        pids[quantidade_pids] = pid;
                        quantidade_pids++;
                    }
                }

                /*
                 * Depois que TODOS foram iniciados,
                 * esperamos cada processo terminar.
                 */
                for (int i = 0; i < quantidade_pids; i++) {
                    int status;

                    if (waitpid(pids[i], &status, 0) == -1) {
                        perror("waitpid");
                    }
                }
            }

            /*
             * RUN SIMPLES
             *
             * Exemplo:
             * run listar
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

                    /*
                     * iniciar_task() cria o processo,
                     * mas NÃO espera ele terminar.
                     */
                    pid_t pid = iniciar_task(task);

                    if (pid != -1) {

                        pids[quantidade_pids] = pid;

                        quantidade_pids++;
                    }
                }

                /*
                 * Depois que TODOS foram iniciados,
                 * esperamos cada processo terminar.
                 */
                for (int i = 0; i < quantidade_pids; i++) {

                    int status;

                    if (waitpid(pids[i], &status, 0) == -1) {

                        perror("waitpid");
                    }
                }

            }

            /*
             * RUN SIMPLES
             *
             * Exemplo:
             * run listar
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