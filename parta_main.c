#include "parta.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

/**
 * main
 * ----
 * Command-line interface for the CPU scheduler simulator.
 *
 * Usage:
 *   FCFS:
 *     ./parta_main fcfs burst0 burst1 ...
 *
 *   Round-robin:
 *     ./parta_main rr quantum burst0 burst1 ...
 *
 * - For "fcfs", all remaining arguments are CPU bursts.
 * - For "rr", the first argument after "rr" is the time quantum,
 *   and the remaining arguments are CPU bursts.
 *
 * The program prints:
 *   - Which algorithm is being used
 *   - The list of accepted processes and their bursts
 *   - The average wait time (to 2 decimal places)
 *
 * If the arguments are missing or invalid, it prints:
 *   "ERROR: Missing arguments"
 * and exits with status 1.
 */
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("ERROR: Missing arguments\n");
        return 1;
    }

    char *algo = argv[1];

    /* ---------------------- FCFS ---------------------- */
    if (strcmp(algo, "fcfs") == 0) {
        // Need at least one burst: ./parta_main fcfs 5 ...
        if (argc < 3) {
            printf("ERROR: Missing arguments\n");
            return 1;
        }

        int plen = argc - 2;      // number of processes
        int *bursts = malloc(sizeof(int) * plen);
        if (bursts == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }

        for (int i = 0; i < plen; i++) {
            bursts[i] = atoi(argv[i + 2]);
        }

        struct pcb *procs = init_procs(bursts, plen);
        if (procs == NULL) {
            fprintf(stderr, "Failed to initialize processes\n");
            free(bursts);
            return 1;
        }

        printf("Using FCFS\n\n");

        for (int i = 0; i < plen; i++) {
            printf("Accepted P%d: Burst %d\n", i, bursts[i]);
        }

        // Run FCFS scheduler (updates waits inside procs)
        (void) fcfs_run(procs, plen);

        // Compute average wait time
        int total_wait = 0;
        for (int i = 0; i < plen; i++) {
            total_wait += procs[i].wait;
        }
        double avg_wait = (double) total_wait / (double) plen;

        printf("Average wait time: %.2f\n", avg_wait);

        free(procs);
        free(bursts);
        return 0;
    }

    /* ------------------- Round-Robin ------------------ */
    else if (strcmp(algo, "rr") == 0) {
        // Need quantum + at least one burst:
        // ./parta_main rr 2 5 8 2
        if (argc < 4) {
            printf("ERROR: Missing arguments\n");
            return 1;
        }

        int quantum = atoi(argv[2]);
        int plen = argc - 3;      // number of processes

        if (quantum <= 0 || plen <= 0) {
            printf("ERROR: Missing arguments\n");
            return 1;
        }

        int *bursts = malloc(sizeof(int) * plen);
        if (bursts == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }

        for (int i = 0; i < plen; i++) {
            bursts[i] = atoi(argv[i + 3]);
        }

        struct pcb *procs = init_procs(bursts, plen);
        if (procs == NULL) {
            fprintf(stderr, "Failed to initialize processes\n");
            free(bursts);
            return 1;
        }

        printf("Using RR(%d).\n\n", quantum);

        for (int i = 0; i < plen; i++) {
            printf("Accepted P%d: Burst %d\n", i, bursts[i]);
        }

        // Run RR scheduler
        (void) rr_run(procs, plen, quantum);

        int total_wait = 0;
        for (int i = 0; i < plen; i++) {
            total_wait += procs[i].wait;
        }
        double avg_wait = (double) total_wait / (double) plen;

        printf("Average wait time: %.2f\n", avg_wait);

        free(procs);
        free(bursts);
        return 0;
    }

    /* ------------------- Unknown algo ----------------- */
    else {
        // Treat unknown algorithm as bad arguments, per spec
        printf("ERROR: Missing arguments\n");
        return 1;
    }
}
