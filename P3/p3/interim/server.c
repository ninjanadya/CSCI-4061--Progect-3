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
static volatile sig_atomic_t run = 1;

/* Stop the printing until the next interrupt. */
void exit_server(int signo) {
  printf("\nending server need to add more prints for graceful termination\n");
  exit(0);
}


request_t queue[MAX_queue_len];
int enqueue_index = 0;
int dequeue_index = 0;

request_t dequeue() {
  request_t temp = queue[dequeue_index];
  dequeue_index++;
  dequeue_index = dequeue_index % MAX_queue_len;
  return temp;
}

void enqueue(request_t r) {
  printf("in enqueue, adding %d, %s\n", r.fd, r.request);
  queue[enqueue_index] = r;
  enqueue_index++;
  dequeue_index = dequeue_index % MAX_queue_len;
}

bool empty_queue() {
  return enqueue_index == dequeue_index;
}


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
  printf("signature is %s\n",signature);
  printf("last char of signature is %c\n",signature[strlen(signature)-1]);
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
  printf("found type as %s\n",type);
	return type;
}

// Function to open and read the file from the disk into the memory
// Add necessary arguments as needed
int readFromDisk(char* filename, char* buffer, int n_bytes) {
    // Open and read the contents of file given the request
    int fd = open(filename, 'r');
    int nread = read(fd, buffer, n_bytes);
    printf("nread: %d\n", nread);
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
    printf("got request, buffer is %s\n", buffer);

    // Add the request into the queue
    disp.fd = fd;
    printf("accept connection fd = %d\n", fd);
    memset(disp.request, '\0', BUFF_SIZE);
    sprintf(disp.request, "%s%s", (char*) arg, buffer);
    printf("disp.request = %s\n", disp.request);
    //strcpy(disp.request, buffer);

    enqueue(disp);

   }
   return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {
  request_t work;
  char* buffer;
  char filetype[20];
  int size, nread;
  struct stat buf;
   while (1) {
    if (!empty_queue()) {

    // Get the request from the queue
    work = dequeue();
    printf("work.request = %s\n", work.request);
    if (stat(work.request, &buf) == 0) {
      printf("stat returned 0\n");
    }

    size = buf.st_size;
    printf("size %d\n", size);

    // Get the data from the disk or the cache (extra credit B)
    buffer = malloc(size);
    printf("work.fd is: %d\n", work.fd);
    nread = readFromDisk(work.request, buffer, size);
    //work.fd = open(work.request, 'r');
    //nread = read(work.fd, buffer, size);
    printf("nread: %d\n", nread);
    //close(work.fd);

    // Log the request into the file and terminal

    // return the result
    memset(filetype, '\0', 20);
    strcpy(filetype, getContentType(work.request));
    printf("filetype: %s, size: %d\n", filetype, size);
    if (return_result(work.fd, filetype, buffer, size) != 0) {
      printf("problem with return result\n");
    }

    free(buffer);
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
  /*int num_dispatchers = strtol(argv[3], NULL, 10);
  int num_workers = strtol(argv[4], NULL, 10);
  int dynamic_flag = strtol(argv[5], NULL, 10);
  int queue_length = strtol(argv[6], NULL, 10);
  int cache_entries = strtol(argv[7], NULL, 10);*/
  // Perform error checks on the input arguments

  // Change SIGINT action for grace termination
  act.sa_handler = exit_server;
  act.sa_flags=0;
  sigemptyset(&act.sa_mask);
  sigaction(SIGINT, &act, NULL);
  // Open log file
  // FILE* log_file = fopen("webserver_log", "w");
  // Change the current working directory to server root directory
  chdir(path);
  // Initialize cache (extra credit B)

  // Start the server
  init(port);

  // Create dispatcher and worker threads (all threads should be detachable)

  request_t temp;
  temp.fd = 1;
  strcpy(temp.request, "test");
  //temp.request = "test";
  if (empty_queue()) {
    printf("empty queue\n");
  }
  enqueue(temp);
  request_t r = dequeue();
  printf("%d, %s\n", r.fd, r.request);

  pthread_t dispatcherID;
  //char* dummy_arg = "this is just to test the pthread_create";
  if (pthread_create(&dispatcherID, NULL, dispatch, (void*) path)) {
    printf("failed to create dispatcher thread\n");
  } else {
    printf("created dispatcher thread\n");
  }

  pthread_t workerID;
  if (pthread_create(&workerID, NULL, worker, (void*) path)) {
    printf("failed to create worker thread\n");
  } else {
    printf("created worker thread\n");
  }


  // Create dynamic pool manager thread (extra credit A)

  // Terminate server gracefully
    // Print the number of pending requests in the request queue
    // close log file
    // Remove cache (extra credit B)
    while (1) {}
  return 0;
}
