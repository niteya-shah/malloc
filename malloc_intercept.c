#define _GNU_SOURCE

#include <dlfcn.h>
#include <errno.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

static FILE *fptr_malloc, *fptr_calloc, *fptr_realloc, *fptr_free;
static void *(*real_malloc)(size_t) = NULL;
static void *(*real_calloc)(size_t, size_t) = NULL;
static void *(*real_realloc)(void *, size_t) = NULL;
static void (*real_free)(void *) = NULL;
static bool CAPTURE_INFO = false;
static bool use_real_funcs = true;

inline struct timespec sub_timespec(const struct timespec *restrict t1,
                                    const struct timespec *restrict t2) {
  enum { NS_PER_SECOND = 1000000000 };

  struct timespec diff = {.tv_sec = t1->tv_sec - t2->tv_sec,
                          .tv_nsec = t1->tv_nsec - t2->tv_nsec};

  if (diff.tv_nsec < 0) {
    diff.tv_nsec += NS_PER_SECOND;
    diff.tv_sec--;
  }

  return diff;
}

inline void write_data(FILE *restrict file, size_t size,
                       const struct timespec *restrict start,
                       const struct timespec *restrict stop) {
  size_t data[5];

  data[0] = size;
  data[1] = start->tv_sec;
  data[2] = start->tv_nsec;
  data[3] = stop->tv_sec;
  data[4] = stop->tv_nsec;

  fwrite(&data, sizeof(size_t), 5, file);
}

__attribute__((constructor)) static void initValues(void) {
  bool previous_value = use_real_funcs;
  use_real_funcs = true;
  if (!real_malloc) {
    real_malloc = dlsym(RTLD_NEXT, "malloc");
  }
  if (!real_calloc) {
    real_calloc = dlsym(RTLD_NEXT, "calloc");
  }
  if (!real_realloc) {
    real_realloc = dlsym(RTLD_NEXT, "realloc");
  }
  if (!real_free) {
    real_free = dlsym(RTLD_NEXT, "free");
  }
  use_real_funcs = previous_value;
}

void start_capture(char *restrict path) {
  use_real_funcs = true;
  if (!path) {
    printf("No path provided");
    exit(0);
  }
  char temp_path[100];
  strcpy(temp_path, path);
  if (!fptr_malloc) {
    strcpy(temp_path + strlen(path), "malloc_data.txt");
    fptr_malloc = fopen(temp_path, "w+");
    if (fptr_malloc == NULL) {
      printf("Failed to open malloc file \n %s \n due to%d\n", temp_path,
             errno);
      exit(0);
    }
  } else {
    printf("Failed to start malloc capture as already capturing");
    exit(0);
  }
  if (!fptr_calloc) {
    strcpy(temp_path + strlen(path), "calloc_data.txt");
    fptr_calloc = fopen(temp_path, "w+");
  } else {
    printf("Failed to start calloc capture as already capturing");
    exit(0);
  }
  if (!fptr_realloc) {
    strcpy(temp_path + strlen(path), "realloc_data.txt");
    fptr_realloc = fopen(temp_path, "w+");
  } else {
    printf("Failed to start realloc capture as already capturing");
    exit(0);
  }
  if (!fptr_free) {
    strcpy(temp_path + strlen(path), "free_data.txt");
    fptr_free = fopen(temp_path, "w+");
  } else {
    printf("Failed to start free capture as already capturing");
    exit(0);
  }
  CAPTURE_INFO = true;
  use_real_funcs = false;
}

void stop_capture() {
  use_real_funcs = true;
  CAPTURE_INFO = false;
  if (fptr_malloc) {
    fclose(fptr_malloc);
    fptr_malloc = NULL;
  } else {
    printf("Failed to stop malloc capture as not capturing");
    exit(0);
  }
  if (fptr_calloc) {
    fclose(fptr_calloc);
    fptr_calloc = NULL;
  } else {
    printf("Failed to stop calloc capture as not capturing");
    exit(0);
  }

  if (fptr_realloc) {
    fclose(fptr_realloc);
    fptr_realloc = NULL;
  } else {
    printf("Failed to stop realloc capture as not capturing");
    exit(0);
  }

  if (fptr_free) {
    fclose(fptr_free);
    fptr_free = NULL;
  } else {
    printf("Failed to stop free capture as not capturing");
    exit(0);
  }
}

void *malloc(size_t size) {
  if (use_real_funcs || !CAPTURE_INFO) {
    return real_malloc(size);
  }

  struct timespec start, stop;
  use_real_funcs = true;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
  // use_real_funcs = false;
  char *p = real_malloc(size);
  // use_real_funcs = 1;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);
  write_data(fptr_malloc, size, &start, &stop);
  use_real_funcs = false;
  return p;
}

void *calloc(size_t num, size_t size) {
  if (use_real_funcs || !CAPTURE_INFO) {
    return real_calloc(num, size);
  }

  struct timespec start, stop;
  use_real_funcs = true;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
  // use_real_funcs = 0;
  char *p = real_calloc(num, size);
  // use_real_funcs = 1;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);
  write_data(fptr_calloc, size * num, &start, &stop);
  use_real_funcs = false;

  return p;
}

void *realloc(void *ptr, size_t size) {
  if (use_real_funcs || !CAPTURE_INFO) {
    return real_realloc(ptr, size);
  }

  struct timespec start, stop;
  use_real_funcs = true;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
  // use_real_funcs = 0;
  char *p = real_realloc(ptr, size);
  // use_real_funcs = 1;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);
  write_data(fptr_realloc, size, &start, &stop);
  use_real_funcs = false;

  return p;
}

void free(void *ptr) {
  if (use_real_funcs || !CAPTURE_INFO) {
    return real_free(ptr);
  }

  struct timespec start, stop;
  use_real_funcs = true;
  size_t size = malloc_usable_size(ptr);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
  // use_real_funcs = 0;
  real_free(ptr);
  // use_real_funcs = 1;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);
  write_data(fptr_free, size, &start, &stop);
  use_real_funcs = false;
  return;
}
