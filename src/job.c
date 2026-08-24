#include <stdio.h>

#include "job.h"

int adicionar_job(
    Job jobs[],
    int *quantidade_jobs,
    pid_t pid
) {

    if (*quantidade_jobs >= MAX_JOBS) {

        printf(
            "Erro: limite máximo de jobs atingido.\n"
        );

        return -1;
    }

    int id = *quantidade_jobs + 1;

    jobs[*quantidade_jobs].id = id;
    jobs[*quantidade_jobs].pid = pid;

    (*quantidade_jobs)++;

    return id;
}