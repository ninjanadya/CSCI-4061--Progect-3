#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include "util.h"
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

#define MAX_THREADS 100
#define MAX_queue_len 100
#define MAX_CE 100
#define INVALID -1
#define BUFF_SIZE 1024

/*
  THE CODE STRUCTURE GIVEN BELOW IS JUST A SUGGESTION. FEEL FREE TO MODIFY AS NEEDED
*/

// structs:
typedef struct request_queue {
   int fd;
   char request[BUFF_SIZE];
} request_t;

typedef struct cache_entry {
    int len;
    char *request;
    char *content;
} cache_entry_t;

struct sigaction act;
static volatile sig_atomic_t run = 0;


/* Stop the printing until the next interrupt. */
void exit_server(int signo) {
  run = 1;
}

request_t queue[MAX_queue_len];
int enqueue_index = 0;
int dequeue_index = 0;
int global_queue_length;

// removes and returns the next request_t struct from the queue
request_t dequeue() {
  request_t temp = queue[dequeue_index];
  dequeue_index++;
  dequeue_index = dequeue_index % global_queue_length;
  return temp;
}

// adds a request_t struct to the queue
void enqueue(request_t r) {
  //printf("in enqueue, adding %d, %s\n", r.fd, r.request);
  queue[enqueue_index] = r;
  enqueue_index++;
  enqueue_index = enqueue_index % global_queue_length;
}

// returns true if the queue is empty
bool empty_queue() {
  return enqueue_index == dequeue_index;
}

// returns true if the queue is full
bool full_queue() {
  return (dequeue_index - enqueue_index) % MAX_queue_len == 1;
}

// initialize the thread synchronization variables
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t some_content = PTHREAD_COND_INITIALIZER;
pthread_cond_t free_slot = PTHREAD_COND_INITIALIZER;

FILE* log_file;


/* ******************** Dynamic Pool Code  [Extra Credit A] **********************/
// Extra Credit: This function implements the policy to change the worker thread pool dynamically
// depending on the number of requests
void * dynamic_pool_size_update(void *arg) {
  while(1) {
    // Run at regular intervals
    // Increase / decrease dynamically based on your policy
  }
}
/**********************************************************************************/

/* ************************ Cache Code [Extra Credit B] **************************/

// Function to check whether the given request is present in cache
int getCacheIndex(char *request){
  /// return the index if the request is present in the cache
  return 0;
}

// Function to add the request and its file content into the cache
void addIntoCache(char *mybuf, char *memory , int memory_size){
  // It should add the request at an index according to the cache replacement policy
  // Make sure to allocate/free memory when adding or replacing cache entries
}

// clear the memory allocated to the cache
void deleteCache(){
  // De-allocate/free the cache memory
}

// Function to initialize the cache
void initCache(){
  // Allocating memory and initializing the cache array
}

/**********************************************************************************/

/* ************************************ Utilities ********************************/
// Function to get the content type from the request
char* getContentType(char * mybuf) {
  // Should return the content type based on the file type in the request
  // (See Section 5 in Project description for more details)
  char *type;
  const char* delimiter = ",";
	char signature[BUFF_SIZE];
  memset(signature, '\0', BUFF_SIZE);
  strcpy(signature, strtok(mybuf, delimiter));
  //printf("signature is %s\n",signature);
  //printf("last char of signature is %c\n",signature[strlen(signature)-1]);
  char last_char = signature[strlen(signature)-1];

	if(last_char == 'l' || last_char == 'm'){
		type = "text/html";
	}
	else if (last_char == 'g'){
		type = "image/jpeg";
	}
	else if(last_char == 'f'){
		type = "image/gif";
	}
	else {
		type = "text/plain";
	}
  //printf("found type as %s\n",type);
	return type;
}

// Function to open and read the file from the disk into the memory
// Add necessary arguments as needed
int readFromDisk(char* filename, char* buffer, int n_bytes) {
    // Open and read the contents of file given the request
    int fd = open(filename, 'r');
    int nread = read(fd, buffer, n_bytes);
    //printf("nread: %d\n", nread);
    close(fd);
    //printf("%s\n", buffer);
    return nread;
}

/**********************************************************************************/

// Function to receive the request from the client and add to the queue
void * dispatch(void *arg) {
  int fd;
  char buffer[BUFF_SIZE];
  request_t disp;
  while (1) {

    // Accept client connection
    fd = accept_connection();

    // Get request from the client
    get_request(fd, buffer);
    //printf("got request, buffer is %s\n", buffer);

    // Add the request into the queue
    disp.fd = fd;
    //printf("accept connection fd = %d\n", fd);
    //memset(disp.request, '\0', BUFF_SIZE);
    //sprintf(disp.request, "%s%s", (char*) arg, buffer);
    //printf("disp.request = %s\n", disp.request);
    strcpy(disp.request, buffer);

    pthread_mutex_lock(&lock);
    while (full_queue()) {
      pthread_cond_wait(&some_content, &lock);
    }
    enqueue(disp);
    //printf("enq index: %d,  deq index: %d\n", enqueue_index, dequeue_index);
    pthread_cond_signal(&free_slot);
    pthread_mutex_unlock(&lock);
   }
   return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {
  request_t work;
  char* buffer;
  char full_path[BUFF_SIZE];
  char filetype[20];
  int size, nread, reqnum = 0;
  struct stat buf;
   while (1) {
    if (!empty_queue()) {
      reqnum++;

    // Get the request from the queue
    pthread_mutex_lock(&lock);
    while (empty_queue()) {
      pthread_cond_wait(&free_slot, &lock);
    }
    //printf("enq index: %d,  deq index: %d\n", enqueue_index, dequeue_index);
    work = dequeue();
    pthread_cond_signal(&some_content);
    pthread_mutex_unlock(&lock);

    memset(full_path, '\0', BUFF_SIZE);
    sprintf(full_path, "%s%s", (char*) arg, work.request);

    //printf("work.request = %s\n", work.request);
    if (stat(full_path, &buf) != 0) {
      printf("stat returned nonzero\n");
    }

    size = buf.st_size;
    //printf("size %d\n", size);

    // Get the data from the disk or the cache (extra credit B)
    buffer = malloc(size);
    //printf("work.fd is: %d\n", work.fd);
    nread = readFromDisk(full_path, buffer, size);
    //work.fd = open(work.request, 'r');
    //nread = read(work.fd, buffer, size);
    //printf("nread: %d\n", nread);
    //close(work.fd);

    // Log the request into the file and terminal

    // return the result
    memset(filetype, '\0', 20);
    strcpy(filetype, getContentType(full_path));
    //printf("filetype: %s, size: %d\n", filetype, size);
    if (return_result(work.fd, filetype, buffer, size) != 0) {
      printf("problem with return result\n");
    }

    if (nread > 0) {
      printf("[%ld][%d][%d][%s][%d]\n", pthread_self(), reqnum, work.fd, work.request, nread);
      fprintf(log_file, "[%ld][%d][%d][%s][%d]\n", pthread_self(), reqnum, work.fd, work.request, nread);
    } else {
      printf("[%ld][%d][%d][%s][%s]\n", pthread_self(), reqnum, work.fd, work.request, "Requested file not found.");
      fprintf(log_file, "[%ld][%d][%d][%s][%s]\n", pthread_self(), reqnum, work.fd, work.request, "Requested file not found.");
    }
    free(buffer);
    // need to close the file in graceful termination
  }
}
  return NULL;
}

/**********************************************************************************/
// ./web_server 9001 /home/berg2007/Desktop/4061/P3/CSCI-4061--Project-3/P3/p3/testing 1 1 0 100 0
int main(int argc, char **argv) {

  // Error check on number of arguments
  if(argc != 8){
    printf("usage: %s port path num_dispatcher num_workers dynamic_flag queue_length cache_size\n", argv[0]);
    return -1;
  }

  // Get the input args
  int port = strtol(argv[1], NULL, 10);
  char* path = argv[2];
  int num_dispatchers = strtol(argv[3], NULL, 10);
  int num_workers = strtol(argv[4], NULL, 10);
  int dynamic_flag = strtol(argv[5], NULL, 10);
  int queue_length = strtol(argv[6], NULL, 10);
  global_queue_length = queue_length;
  int cache_entries = strtol(argv[7], NULL, 10);

  // Perform error checks on the input arguments
  if (num_dispatchers > MAX_THREADS) {
    printf("number of dispatchers must be <= 100\n");
    exit(1);
  }
  if (num_workers > MAX_THREADS) {
    printf("number of workers must be <= 100\n");
    exit(1);
  }
  if (queue_length > MAX_queue_len) {
    printf("queue length must be <= 100\n");
    exit(1);
  }
  if (cache_entries > MAX_CE) {
    printf("cache size must be <= 100\n");
    exit(1);
  }
  if (strlen(path) > BUFF_SIZE) {
    printf("filename length must be <= 1024\n");
    exit(1);
  }

  // Change SIGINT action for grace termination
  act.sa_handler = exit_server;
  act.sa_flags=0;
  if(sigemptyset(&act.sa_mask) == INVALID || sigaction(SIGINT, &act, NULL) == INVALID){
	printf("Error SIGINT.\n");
	return INVALID;
  }
  sigemptyset(&act.sa_mask);
  sigaction(SIGINT, &act, NULL);

  // Open log file
  log_file = fopen("webserver_log", "w");

  // Change the current working directory to server root directory
  chdir(path);

  // Initialize cache (extra credit B)

  // Start the server
  init(port);

  // Create dispatcher and worker threads (all threads should be detachable)
  pthread_t dispatcherID[num_dispatchers];
  for (int i = 0; i < num_dispatchers; i++) {
    if (pthread_create(&(dispatcherID[i]), NULL, dispatch, (void*) path)) {
      printf("failed to create dispatcher thread\n");
    }
  }

  pthread_t workerID[num_workers];
  for (int i = 0; i < num_workers; i++) {
    if (pthread_create(&(workerID[i]), NULL, worker, (void*) path)) {
      printf("failed to create worker thread\n");
    }
  }

  // Create dynamic pool manager thread (extra credit A)

  // Terminate server gracefully
  // Print the number of pending requests in the request queue
  // close log file
  // Remove cache (extra credit B)
  while (!run) {
	   sleep(1);
  }
  return 0;
}
