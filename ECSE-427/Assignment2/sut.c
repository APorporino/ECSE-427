#include "sut.h"
#include "queue.h"

// kernel threads and mutex
pthread_t k_thread_exe;
pthread_t k_thread_io;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// these are the parent conetxts for the 2 kernel threads
static ucontext_t mainContext;
static ucontext_t iContext;

int numthreads, curthread;
// this list will contain the contexts (the functions the user wants to run)
ucontext_t contexts[MAX_THREADS];
// this list of numbers is used to enter into the queue so that it does not override
int listNumbers[MAX_THREADS];
struct queue taskQ;

// the queue used for input and output contexts
struct queue waitQ;

// Since we only perform one IO operation at a time, we can have simple logic 
// to figure out what is being called using boolean values
bool opening_being_called = false;
bool closing_being_called = false;
bool reading_being_called = false;
bool writing_being_called = false;
bool hasFileBeenOpened = false;

// booleans to handle the shutdown of the threads securely
bool runningCEXEC = true;
bool runningIEXEC = true;
bool notDoneCEXEC = true;
bool notDoneIEXEC = true;

// These variables will be used to write and read messages
char *writeString;
int sizeOfMessage;
char *readString;

// global variables related to opening a connection
char *host;
int port;
int sockfd;

/**
 * The CEXEC Kernel thread.
 *
 * This thread will pop from the task queue (taskQ)
 * to change the context (the task running) to the task associated with the number popped
 * from the queue.
 * 
 **/
void *cexec(void *arg){
    getcontext(&mainContext);
    struct queue_entry *ptr = queue_peek_front(&taskQ);
    while (runningCEXEC) {
        if(ptr){
            notDoneCEXEC = true;

            pthread_mutex_lock(&mutex);
            ptr = queue_pop_head(&taskQ);
            curthread = *(int *)ptr->data;
            pthread_mutex_unlock(&mutex);

            swapcontext(&mainContext, &contexts[curthread]);
            
            ptr = queue_peek_front(&taskQ);
            notDoneCEXEC = false;
        }
        usleep(SLEEP_TIME);
        ptr = queue_peek_front(&taskQ);
    }
}

/**
 * The I-EXEC Kernel thread
 * 
 * This thread will pop from the wait queue (waitQ)
 * to change the context (the task running) to the task associated with the number popped
 * from the queue. It will only perform input and output operations, and determines which is to
 * be performed through the use of global booleans.
 * 
 * Once the task is finished, it is added back into the taskQ and the next element of the waitQ
 * is processed.
 **/
void *iexec(void *arg){
    getcontext(&iContext);
    struct queue_entry *ptr = queue_peek_front(&waitQ);
    while (runningIEXEC) {
        if(ptr){
            notDoneIEXEC = true;

            ptr = queue_pop_head(&waitQ);

            // must check what operation is being performed. As described in README.txt simple booleans can be used
            if (opening_being_called){   // open function
                const char* hostConst = host;
                if (connect_to_server(hostConst, port, &sockfd) < 0) {
                    fprintf(stderr, "Failed to connect to server\n");
                }
                hasFileBeenOpened = true;
                printf("Connected\n");
                opening_being_called = false;
            } else if (reading_being_called && hasFileBeenOpened){ // read funciton
                ssize_t byte_count = recv_message(sockfd, readString, sizeof(readString));
                if (byte_count <= 0) {
                    readString = "Cannot read from socket";
                }
                reading_being_called = false;
            }else if (writing_being_called && hasFileBeenOpened){ // write function
                send_message(sockfd, writeString,sizeOfMessage);
                writing_being_called = false;
            }else if (closing_being_called && hasFileBeenOpened){
                printf("Closing socket");
                hasFileBeenOpened = false;  // reset the boolean value
                close(sockfd);
            } else { // this will be called if a read/write or close was attempted before an open               
                printf("MUST CALL SUT OPEN BEFORE READ OR WRITE\n");
            }

            // insert the context back into the task queue so CEXEC can run it again
            struct queue_entry *node = queue_new_node(&listNumbers[*(int *)ptr->data]);
            queue_insert_tail(&taskQ, node);

            notDoneIEXEC = false;
        }
        usleep(SLEEP_TIME);
        ptr = queue_peek_front(&waitQ);
    }
}

/**
 * This function will initialize the library by firstly creating
 * a task queue and then the two kernel level threads.
 **/
void sut_init() {
    numthreads = 0;
    notDoneCEXEC = false;
    notDoneIEXEC = false;

    taskQ = queue_create();
    queue_init(&taskQ);
    waitQ = queue_create();
    queue_init(&waitQ);

    pthread_create(&k_thread_exe, NULL, cexec, NULL);
    pthread_create(&k_thread_io, NULL, iexec, NULL);
}

/**
 * This function takes in a function and will create a new thread 
 * associated with that functiom and will finally add it to the task queue.
 **/
bool sut_create(sut_task_f fn) {
    // create a new contex in the contexts array
    getcontext(&contexts[numthreads]);
	contexts[numthreads].uc_stack.ss_sp = (char *)malloc(THREAD_STACK_SIZE);;
	contexts[numthreads].uc_stack.ss_size = THREAD_STACK_SIZE;
    makecontext(&contexts[numthreads], fn, 0);

    // create a new int in the listNumbers array and store it in the queue
    listNumbers[numthreads] = numthreads;
    struct queue_entry *node = queue_new_node(&listNumbers[numthreads]);
    queue_insert_tail(&taskQ, node);

    numthreads++;
}

/**
 * This function will stop the current thread being executed and switch back
 * to the main thread. The thread stopped will then be added to the end of the
 * queue.
 **/
void sut_yield() {
    struct queue_entry *node = queue_new_node(&listNumbers[curthread]);
    queue_insert_tail(&taskQ, node);
    swapcontext(&contexts[curthread], &mainContext);
}

/**
 * This function will stop the current thread being executed and switch back 
 * to the main thread. It does not however add it back to the queue.
 **/
void sut_exit() {
    ucontext_t currentContext;
    getcontext(&currentContext);
    swapcontext(&currentContext, &mainContext);
}

/**
 * This function will shutdown the program safely by joining the kernel threads
 * only once the queue's are empty and no tasks are in the middle of execution!
 **/
void sut_shutdown() {
    struct queue_entry *ptr = queue_peek_front(&taskQ);
    struct queue_entry *ptr2 = queue_peek_front(&waitQ);
    // this will only stop CEXEC if taskQ and waitQ are empty and there are no tasks running
    while(true){
        if (!ptr && !ptr2 && !notDoneIEXEC && !notDoneCEXEC) {
            runningIEXEC = false;
            runningCEXEC = false;
            break;
        }
        ptr = queue_peek_front(&taskQ);
        ptr2 = queue_peek_front(&waitQ);
        usleep(SLEEP_TIME);
    }
    pthread_join(k_thread_exe, NULL);
    pthread_join(k_thread_io, NULL);
    return;
}

/**
 * This function will set an open boolean flag to true and then add the current context 
 * to the wait queue so the task can be handled by the IEXEC kernel thread.
 * The IEXEC will then open the socket conection.
 * 
 * It then switches back to the CEXEC context to not block.
 **/
void sut_open(char *dest, int p){
    opening_being_called = true;
    host = dest;
    port = p;
    struct queue_entry *node = queue_new_node(&listNumbers[curthread]);
    queue_insert_tail(&waitQ, node);
    swapcontext(&contexts[curthread], &mainContext);
}

/**
 * This function will set a write boolean flag to true and then add the current context 
 * to the wait queue so the task can be handled by the IEXEC kernel thread.
 * The IEXEC will then write to the connected socket.
 * 
 * It then switches back to the CEXEC context to not block.
 **/
void sut_write(char *buf, int size){
    writing_being_called = true;
    writeString = buf;
    sizeOfMessage = size;
    struct queue_entry *node = queue_new_node(&listNumbers[curthread]);
    queue_insert_tail(&waitQ, node);
    swapcontext(&contexts[curthread], &mainContext);
}

/**
 * This function will close the socket connected
 **/
void sut_close(){
    closing_being_called = true;
    struct queue_entry *node = queue_new_node(&listNumbers[curthread]);
    queue_insert_tail(&waitQ, node);
    swapcontext(&contexts[curthread], &mainContext);
}

/**
 * This function will set a read boolean flag to true and then add the current context 
 * to the wait queue so the task can be handled by the IEXEC kernel thread.
 * The IEXEC will then read from the connected socket untill there are no more read commands.
 * 
 * It then switches back to the CEXEC context to not block.
 **/
char *sut_read() {
    reading_being_called = true;
    struct queue_entry *node = queue_new_node(&listNumbers[curthread]);
    queue_insert_tail(&waitQ, node);
    swapcontext(&contexts[curthread], &mainContext);
    return readString;
}

int connect_to_server(const char *host, uint16_t port, int *sockfd) {
    struct sockaddr_in server_address = {0};

    // create a new socket
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd < 0) {
        perror("Failed to create a new socket\n");
        return -1;
    }

    // connect to server
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, host, &(server_address.sin_addr.s_addr));
    server_address.sin_port = htons(port);
    if (connect(*sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Failed to connect to server\n");
        return -1;
    }
    return 0;
}

ssize_t send_message(int sockfd, char *buf, size_t len) {
    return send(sockfd, buf, len, 0);
}

ssize_t recv_message(int sockfd, char *buf, size_t len) {
    return recv(sockfd, buf, len, 0);
}
