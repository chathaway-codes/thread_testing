/** Solve the 3n+1 problem using either mupltiple threads
 *   or multiple processes (using SYSV shared memory)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>

/* Easy way of avoiding locking; use atomic datatype for recording
 *  completed threads
 */
short threads_done = 0;

enum SpawnMethod {
  NONE,
  THREADS,
  PROCS
} typedef SpawnMethod;

struct DriverArgs {
  int64_t limit, inc, procs;
} typedef DriverArgs;

/** Assuming that the colatz conjecture is true,
 *   run through a given integer n
 */
void solve(int64_t n) {
  while (n != 1) {
    if (n % 2 == 0)
      n /= 2;
    else
      n = n * 3 + 1;
  }
}

/** Calls solve() with every item (i) in range (0, limit) where i % procs == inc
 */
void *driver(void *arguments) {
  DriverArgs *args = (DriverArgs*)arguments;
  int64_t i = args->inc;
  while(i < args->limit) {
    solve(i);
    i += args->inc;
  }
  threads_done++;
  free(arguments);
  return NULL;
}

/** Spawns procs using fork() */
void fork_spawn(int64_t limit, int procs) {
  int pid;
  DriverArgs *args;

  for(int i=0; i < procs; i++) {
    printf("Spawn proc...");
    fflush(stdout);
    pid = fork();
    if (pid == 0) {
      args = (DriverArgs*)malloc(sizeof(DriverArgs));
      args->limit = limit;
      args->inc = i + 1;
      args->procs = procs;
      driver(args);
      exit(0);
    } else if (pid == -1) {
      printf("Warning: failed to spawn child process!\n");
    }
    printf(" done!\n");
  }
  while(threads_done != procs) {
    wait(&pid);
    threads_done++;
  }
}

/** Spawns procs using pthread_create() */
void thread_spawn(int64_t limit, int procs) {
  pthread_t thread;
  int ret;
  DriverArgs *args;


  for(int i=0; i < procs; i++) {
    args = (DriverArgs*)malloc(sizeof(DriverArgs));
    args->limit = limit;
    args->inc = i + 1;
    args->procs = procs;
    printf("Spawn thread...");
    ret = pthread_create(&thread, NULL, driver, (void*)args);
    if (ret != 0) {
      printf("Warning: failed to spawn thread!\n");
    }
    printf(" done!\n");
  }
}

/** Usage: 3np1 [--threads | --procs] <count> N
 */
int main(int argc, char **args) {
  SpawnMethod spawnMethod = NONE;
  int threadCount = 0;
  int64_t value = 0;
  if (argc != 4) {
    printf("Usage: %s [--threads | --procs] <count> N\n", args[0]);
    return 1;
  }
  if (strcmp("--threads", args[1]) == 0) {
    spawnMethod = THREADS;
  } else if (strcmp("--procs", args[1]) == 0) {
    spawnMethod = PROCS;
  } else {
    printf("Unknown spawning mode: %s\n", args[1]);
  }

  threadCount = atoi(args[2]);
  if (threadCount == 0) {
    printf("Incorrect thread count: %d\n", threadCount);
  }

  value = atoi(args[3]);
  if (value == 0) {
    printf("Incorrect value for N: %ld\n", value);
  }

  if (spawnMethod == PROCS) {
    fork_spawn(value, threadCount);
  } else if (spawnMethod == THREADS) {
    thread_spawn(value, threadCount);
  }

  while(threads_done != threadCount) {
    usleep(100);
  }

  return 0;
}
