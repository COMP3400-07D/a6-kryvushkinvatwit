#include "parta.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * init_procs
 * -----------
 * Given an array of CPU burst times (bursts) of length blen,
 * allocate and initialize an array of PCBs on the heap.
 * Each PCB gets:
 *   - pid         = its index (0..blen-1)
 *   - burst_left  = bursts[i]
 *   - wait        = 0 (no waiting yet)
 *
 * Returns a pointer to the allocated array, or NULL on failure.
 */
struct pcb* init_procs(int* bursts, int blen) {
    if (blen <= 0 || bursts == NULL) {
        return NULL;
    }

    struct pcb *procs = (struct pcb *)malloc(sizeof(struct pcb) * blen);
    if (procs == NULL) {
        return NULL;
    }

    for (int i = 0; i < blen; i++) {
        procs[i].pid        = i;
        procs[i].burst_left = bursts[i];
        procs[i].wait       = 0;
    }

    return procs;
}

/**
 * printall
 * --------
 * Helper/debug function that prints the state of each PCB:
 * PID, remaining burst_left, and accumulated wait time.
 */
void printall(struct pcb* procs, int plen) {
    if (procs == NULL || plen <= 0) return;

    for (int i = 0; i < plen; i++) {
        printf("PID %d: burst_left=%d wait=%d\n",
               procs[i].pid, procs[i].burst_left, procs[i].wait);
    }
}

/**
 * run_proc
 * --------
 * "Runs" the process at index `current` for `amount` time units.
 *
 * - Decreases procs[current].burst_left by the actual amount run
 *   (capped at its remaining burst).
 * - Increases the wait time of all *other* processes that are not
 *   yet finished (burst_left > 0) by the same actual amount.
 *
 * Example: bursts = [5, 8, 2], current = 0, amount = 4
 *   P0 burst_left: 5 -> 1
 *   P1 wait: 0 -> 4
 *   P2 wait: 0 -> 4
 */
void run_proc(struct pcb* procs, int plen, int current, int amount) {
    if (procs == NULL || plen <= 0) return;
    if (current < 0 || current >= plen) return;
    if (amount <= 0) return;
    if (procs[current].burst_left <= 0) return;

    // Actual run time cannot exceed remaining burst
    int actual_run = procs[current].burst_left;
    if (actual_run > amount) {
        actual_run = amount;
    }

    // Decrease current process burst
    procs[current].burst_left -= actual_run;

    // Increase wait for all other unfinished processes
    for (int i = 0; i < plen; i++) {
        if (i == current) continue;
        if (procs[i].burst_left > 0) {
            procs[i].wait += actual_run;
        }
    }
}

/**
 * fcfs_run
 * --------
 * Simulates First-Come-First-Serve (FCFS) scheduling.
 *
 * Starting from pid 0 up to pid plen-1, each process runs
 * to completion (non-preemptive). This function uses run_proc
 * to account for wait times of other processes.
 *
 * Returns the total time elapsed when all processes are done.
 */
int fcfs_run(struct pcb* procs, int plen) {
    if (procs == NULL || plen <= 0) {
        return 0;
    }

    int current_time = 0;

    for (int i = 0; i < plen; i++) {
        if (procs[i].burst_left <= 0) {
            continue;
        }

        int amount = procs[i].burst_left;  // run to completion
        run_proc(procs, plen, i, amount);
        current_time += amount;
    }

    return current_time;
}

/**
 * rr_next
 * -------
 * Helper for round-robin scheduling.
 *
 * Given the index of the previously run process `current`,
 * returns the index of the next process to run in round-robin order.
 *
 * - It cycles through processes in increasing index order (circular).
 * - It skips any processes that are already complete (burst_left == 0).
 * - If all processes are complete, returns -1.
 *
 * If `current` is out of range (e.g., -1), the search starts at 0.
 */
int rr_next(int current, struct pcb* procs, int plen) {
    if (procs == NULL || plen <= 0) {
        return -1;
    }

    // Check if all processes are complete
    int all_done = 1;
    for (int i = 0; i < plen; i++) {
        if (procs[i].burst_left > 0) {
            all_done = 0;
            break;
        }
    }
    if (all_done) {
        return -1;
    }

    // Normalize starting point
    int start = current;
    if (start < 0 || start >= plen) {
        start = -1; // so start+1 becomes 0
    }

    int idx = (start + 1) % plen;
    int first = idx;

    // Search for the next process with burst_left > 0
    do {
        if (procs[idx].burst_left > 0) {
            return idx;
        }
        idx = (idx + 1) % plen;
    } while (idx != first);

    // Should not reach here due to all_done check
    return -1;
}

/**
 * rr_run
 * ------
 * Simulates Round-Robin scheduling with a given time quantum.
 *
 * Starting with the first runnable process, repeatedly:
 *   - choose the next process using rr_next
 *   - run it for min(quantum, burst_left) time units using run_proc
 * until all processes are complete.
 *
 * Returns the total time elapsed when all processes are done.
 */
int rr_run(struct pcb* procs, int plen, int quantum) {
    if (procs == NULL || plen <= 0 || quantum <= 0) {
        return 0;
    }

    int current_time = 0;
    int prev = -1;  // no previous process initially

    while (1) {
        int next = rr_next(prev, procs, plen);
        if (next == -1) {
            break;  // all processes finished
        }

        int amount = procs[next].burst_left;
        if (amount > quantum) {
            amount = quantum;
        }

        run_proc(procs, plen, next, amount);
        current_time += amount;
        prev = next;
    }

    return current_time;
}
