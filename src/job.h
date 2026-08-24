#ifndef JOB_H
#define JOB_H

#include <sys/types.h>

#define MAX_JOBS 100

typedef struct {

    int id;
    pid_t pid;

} Job;

int adicionar_job(
    Job jobs[],
    int *quantidade_jobs,
    pid_t pid
);

#endif