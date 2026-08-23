#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "task.h"

#define MAX_INPUT 500
#define MAX_TOKENS 30
#define MAX_WORKDIR 500

int main() {

    Task tarefas[MAX_TASKS];
    int quantidade_tasks = 0;

    char diretorio_trabalho[MAX_WORKDIR] = "";

    char comando[MAX_INPUT];

    while (1) {

        printf("processflow> ");
        fflush(stdout);

        /*
         * Lê uma linha do terminal.
         *
         * Se fgets() retornar NULL, significa por exemplo
         * que houve CTRL-D (EOF).
         */
        if (fgets(comando, sizeof(comando), stdin) == NULL) {
            break;
        }

        /*
         * Remove o '\n' colocado pelo fgets().
         */
        comando[strcspn(comando, "\n")] = '\0';

        /*
         * COMANDO EXIT
         */
        if (strcmp(comando, "exit") == 0) {
            break;
        }

        /*
         * Ignora linhas vazias.
         */
        if (strlen(comando) == 0) {
            continue;
        }

        /*
         * Divide o comando em tokens.
         */
        char *tokens[MAX_TOKENS];
        int quantidade_tokens = 0;

        char *token = strtok(comando, " ");

        while (
            token != NULL &&
            quantidade_tokens < MAX_TOKENS
        ) {

            tokens[quantidade_tokens] = token;
            quantidade_tokens++;

            token = strtok(NULL, " ");
        }

        if (quantidade_tokens == 0) {
            continue;
        }

        /*
         * ==================================================
         * COMANDO TASK
         *
         * task <nome> <programa> [argumentos...]
         *
         * Exemplo:
         * task listar /bin/ls -l
         * ==================================================
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
         * ==================================================
         * COMANDO INPUT
         *
         * input <tarefa> <arquivo>
         *
         * Exemplo:
         * input mostrar nomes.txt
         * ==================================================
         */
        else if (strcmp(tokens[0], "input") == 0) {

            if (quantidade_tokens != 3) {

                printf(
                    "Erro: uso correto: "
                    "input <tarefa> <arquivo>\n"
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
         * ==================================================
         * COMANDO OUTPUT
         *
         * output <tarefa> <arquivo>
         * ==================================================
         */
        else if (strcmp(tokens[0], "output") == 0) {

            if (quantidade_tokens != 3) {

                printf(
                    "Erro: uso correto: "
                    "output <tarefa> <arquivo>\n"
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

            /*
             * 0 significa que não é append.
             */
            definir_output(
                task,
                tokens[2],
                0
            );
        }

        /*
         * ==================================================
         * COMANDO APPEND
         *
         * append <tarefa> <arquivo>
         * ==================================================
         */
        else if (strcmp(tokens[0], "append") == 0) {

            if (quantidade_tokens != 3) {

                printf(
                    "Erro: uso correto: "
                    "append <tarefa> <arquivo>\n"
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

            /*
             * 1 significa append.
             */
            definir_output(
                task,
                tokens[2],
                1
            );
        }

        /*
         * ==================================================
         * COMANDO WORKDIR
         *
         * workdir <diretorio>
         *
         * Exemplo:
         * workdir /tmp
         * ==================================================
         */
        else if (strcmp(tokens[0], "workdir") == 0) {

            if (quantidade_tokens != 2) {

                printf(
                    "Erro: uso correto: "
                    "workdir <diretorio>\n"
                );

                continue;
            }

            struct stat info;

            /*
             * Verifica se o caminho existe.
             */
            if (stat(tokens[1], &info) != 0) {

                printf(
                    "Erro: diretório '%s' não existe.\n",
                    tokens[1]
                );

                continue;
            }

            /*
             * Verifica se realmente é um diretório.
             */
            if (!S_ISDIR(info.st_mode)) {

                printf(
                    "Erro: '%s' não é um diretório.\n",
                    tokens[1]
                );

                continue;
            }

            /*
             * Guarda o diretório.
             *
             * O processo pai não chama chdir().
             * O chdir será executado nos processos filhos.
             */
            strcpy(
                diretorio_trabalho,
                tokens[1]
            );
        }

        /*
         * ==================================================
         * COMANDO RUN
         * ==================================================
         */
        else if (strcmp(tokens[0], "run") == 0) {

            if (quantidade_tokens < 2) {

                printf(
                    "Erro: uso correto: run <nome>\n"
                );

                continue;
            }

            /*
             * ==============================================
             * RUN SEQUENTIAL
             *
             * run sequential tarefa1 tarefa2 ...
             * ==============================================
             */
            if (
                strcmp(
                    tokens[1],
                    "sequential"
                ) == 0
            ) {

                if (quantidade_tokens < 3) {

                    printf(
                        "Erro: uso correto: "
                        "run sequential "
                        "<tarefa1> [tarefa2...]\n"
                    );

                    continue;
                }

                for (
                    int i = 2;
                    i < quantidade_tokens;
                    i++
                ) {

                    Task *task = buscar_task(
                        tarefas,
                        quantidade_tasks,
                        tokens[i]
                    );

                    if (task == NULL) {

                        printf(
                            "Erro: task '%s' "
                            "não encontrada.\n",
                            tokens[i]
                        );

                        break;
                    }

                    /*
                     * executar_task cria o filho
                     * e espera ele terminar.
                     */
                    executar_task(
                        task,
                        diretorio_trabalho
                    );
                }
            }

            /*
             * ==============================================
             * RUN PARALLEL
             *
             * run parallel tarefa1 tarefa2 ...
             * ==============================================
             */
            else if (
                strcmp(
                    tokens[1],
                    "parallel"
                ) == 0
            ) {

                if (quantidade_tokens < 3) {

                    printf(
                        "Erro: uso correto: "
                        "run parallel "
                        "<tarefa1> [tarefa2...]\n"
                    );

                    continue;
                }

                pid_t pids[MAX_TOKENS];
                int quantidade_pids = 0;

                /*
                 * Primeiro iniciamos todas as tarefas.
                 */
                for (
                    int i = 2;
                    i < quantidade_tokens;
                    i++
                ) {

                    Task *task = buscar_task(
                        tarefas,
                        quantidade_tasks,
                        tokens[i]
                    );

                    if (task == NULL) {

                        printf(
                            "Erro: task '%s' "
                            "não encontrada.\n",
                            tokens[i]
                        );

                        continue;
                    }

                    pid_t pid = iniciar_task(
                        task,
                        diretorio_trabalho
                    );

                    if (pid != -1) {

                        pids[quantidade_pids] = pid;
                        quantidade_pids++;
                    }
                }

                /*
                 * Depois esperamos todos terminarem.
                 */
                for (
                    int i = 0;
                    i < quantidade_pids;
                    i++
                ) {

                    int status;

                    if (
                        waitpid(
                            pids[i],
                            &status,
                            0
                        ) == -1
                    ) {

                        perror("waitpid");
                    }
                }
            }

            /*
             * ==============================================
             * RUN PIPE
             *
             * run pipe tarefa1 tarefa2 ...
             * ==============================================
             */
            else if (
                strcmp(
                    tokens[1],
                    "pipe"
                ) == 0
            ) {

                /*
                 * Precisamos de pelo menos:
                 *
                 * run pipe tarefa1 tarefa2
                 *
                 * 4 tokens.
                 */
                if (quantidade_tokens < 4) {

                    printf(
                        "Erro: uso correto: "
                        "run pipe "
                        "<tarefa1> <tarefa2> "
                        "[tarefa3...]\n"
                    );

                    continue;
                }

                int quantidade_pipe =
                    quantidade_tokens - 2;

                Task *tasks_pipe[MAX_TOKENS];

                int erro = 0;

                /*
                 * Localiza todas as tasks antes
                 * de iniciar o pipe.
                 */
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
                            "Erro: task '%s' "
                            "não encontrada.\n",
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
                    quantidade_pipe,
                    diretorio_trabalho
                );
            }

            /*
             * ==============================================
             * RUN SIMPLES
             *
             * run <nome>
             *
             * Exemplo:
             * run listar
             * ==============================================
             */
            else {

                if (quantidade_tokens != 2) {

                    printf(
                        "Erro: uso correto: "
                        "run <nome>\n"
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
                        "Erro: task '%s' "
                        "não encontrada.\n",
                        tokens[1]
                    );

                    continue;
                }

                /*
                 * CORREÇÃO IMPORTANTE:
                 *
                 * O run simples também precisa receber
                 * diretorio_trabalho.
                 */
                executar_task(
                    task,
                    diretorio_trabalho
                );
            }
        }

        /*
         * ==================================================
         * COMANDO DESCONHECIDO
         * ==================================================
         */
        else {

            printf(
                "Comando desconhecido.\n"
            );
        }
    }

    return 0;
}