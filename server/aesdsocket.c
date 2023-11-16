#include "aesdsocket.h"
#define USE_AESD_CHAR_DEVICE 1;
#define PORT "9000"
#define MAXDATASIZE 1048
//Global variables
bool isDaemon;
bool isError;

//Linked list head represents clients
Node* head;;
//File variables
FILE *file;
#define DATA_FILE "/var/tmp/aesdsocketdata"
int server_fd;

pthread_mutex_t listMutex, fileMutex;


int main(int argc, char *argv[]) {
    //int status;
    isError = false;
    head = NULL;
    isDaemon=false;
    struct addrinfo hints, *servinfo, *p;
    struct sigaction new_action;
    int ret;
  
    int client_fd;
    struct sockaddr_storage client_addr; 
    socklen_t client_addr_len;
    int yes = 1;

    // openlog
    openlog("server", LOG_PID | LOG_CONS | LOG_NDELAY, LOG_USER);
    //Catch daemon flag(s)
    if (argc > 1 && strcmp(argv[1], "-d") == 0)
    {
        printf("Working as a Daemon\n");
        isDaemon = true;
    }
  
    //Set up the signal handling
    memset(&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = signal_handler;
    if (sigaction(SIGTERM, &new_action, NULL) != 0) {
        fprintf(stderr, "Error %d registering for SIGTERM", errno);
    } 

    if (sigaction(SIGINT, &new_action, NULL) != 0) {
        fprintf(stderr, "Error %d registering for SIGINT", errno);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    // get address info
    if ((ret = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        return 1;
    }

    // servinfo is a linked list, loop through and try

    for (p = servinfo ; p != NULL ; p = servinfo->ai_next){
    // Create the server socket
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            continue;
        }

        // Set options for reuseable address
        
        if ((setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) == -1) {
            perror("socket options");
            exit(1);
        }

        if (bind(server_fd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("bind");
            close(server_fd);
            continue;
        }
        break;
    }
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    // successfull bind
    if (isDaemon) {
        printf("%s\n","making daemon" );
        make_Daemon();
    }
    // Listen for incoming connections
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("Listening on port 9000\n");

    // init mutexes
    pthread_mutex_init(&fileMutex, NULL);
    pthread_mutex_init(&listMutex, NULL);
    
    // main accept loop
    while (!isError) {
        // Accept a new client connection
        client_addr_len = sizeof(client_addr);
        if ((client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len)) == -1) {
            perror("accept");
            continue;
        }
        char s[INET6_ADDRSTRLEN];
        inet_ntop(client_addr.ss_family,
            get_in_addr((struct sockaddr *)&client_addr),
            s, sizeof s);
        // sys log address
        syslog(LOG_INFO, "Accepted connection from %s", s);
        printf("Hello client from %s", s);

        // Create a thread to handle the client
        thread_data_t* data = (thread_data_t*)malloc(sizeof(thread_data_t));
        data->client_fd = client_fd;
        data->isComplete = 0;

        if (pthread_create(&(data->threadId), NULL, client_handler, data) == 0) {
            insertNode(data);
        } else {
            perror("pthread_create");
            free(data);
        }

        // Check for and remove completed threads
        removeCompletedThreads();
    }

    // Close the server socket 
    do_shutdown();

    return 0;
}
    
    
void* client_handler(void *arg)
{
    thread_data_t* thread_data = (thread_data_t*)arg;
    // Handle the client connection, e.g., read and write data
    char buffer[MAXDATASIZE]="";
    ssize_t bytes_received=0;

    pthread_mutex_lock(&fileMutex); // Lock for file access
    file = fopen(DATA_FILE, "a+");
    while ((bytes_received = recv(thread_data->client_fd, buffer, sizeof(buffer), 0)) > 0) {
            fwrite(buffer, 1, bytes_received, file);

        if (buffer[bytes_received-1]=='\n') {
            break;
        }
    }
    fclose(file);
    pthread_mutex_unlock(&fileMutex); 
    char* line = NULL;
    size_t len = 0;
    ssize_t read_size = 0;

    pthread_mutex_lock(&fileMutex); 
    file = fopen(DATA_FILE, "a+");
    while ((read_size = getline(&line, &len, file))  !=-1) {
            send(thread_data->client_fd, line, read_size, 0);
    }
    fclose(file);
    printf("saved file \n");
    pthread_mutex_unlock(&fileMutex); 

    // Mark the thread as complete
    thread_data->isComplete = 1;

    close(thread_data->client_fd);
    
    // get client addresses
    
    char client_IP[INET6_ADDRSTRLEN];
    struct sockaddr_storage client_addr;
    socklen_t leng = sizeof client_addr;

    getpeername(thread_data->client_fd, (struct sockaddr*) &client_addr, &leng);

    char s[INET6_ADDRSTRLEN];
    inet_ntop(client_addr.ss_family,
            get_in_addr((struct sockaddr *)&client_addr),
            s, sizeof s);
        // sys log address

    syslog(LOG_INFO, "Closed connection from %s", s);
    return NULL;
}

static void signal_handler (int signal_number) {
  syslog(LOG_INFO, "Caught signal, exiting");
  isError=true;
  
  do_shutdown();
}

void make_Daemon(void) {
    pid_t pid = fork();
    if (pid < 0) {
      fprintf(stderr, "couldn't fork");
      exit(EXIT_FAILURE);
    }
    if (pid > 0) 
    { //we are in parent, exit
      exit(EXIT_SUCCESS);
    }
    // in child, make new session
    pid_t sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }
    chdir("/");

    // close standard files, 
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

}


void* timestamp(void * arg){
    while (1) {
        sleep(10); // Wait for 10 seconds

        pthread_mutex_lock(&fileMutex); // Lock for file access
        FILE* file = fopen(DATA_FILE, "a+");
        if (file != NULL) {
            time_t rawtime;
            struct tm* timeinfo;

            // Get current time
            time(&rawtime);
            timeinfo = localtime(&rawtime);

            char timestamp[30];
            // Format timesteamp
            strftime(timestamp, sizeof(timestamp),"%a, %d %b %Y %H:%M:%S %z", timeinfo);

            fprintf(file, "timestamp: %s\n", timestamp);
            fclose(file);
        }
        pthread_mutex_unlock(&fileMutex); // Unlock file access
    }

}


void do_shutdown(void) {
    //join threads
    pthread_mutex_lock(&listMutex);
    Node* current = head;
    Node* prev = NULL;

    while (current != NULL) {
        // Join and cleanup the thread
        pthread_join(current->data->threadId, NULL);

        // Remove the node from the list
        if (prev == NULL) {
            head = current->next;
            free(current->data);
            free(current);
            current = head;
        } else {
            prev->next = current->next;
            free(current->data);
            free(current);
            current = prev->next;
        }
        if (unlink(DATA_FILE) == -1) {
            perror("unlink file");
        }
    }

    pthread_mutex_unlock(&listMutex);

    pthread_mutex_destroy(&fileMutex);
    pthread_mutex_destroy(&listMutex);
    //cleanup sockets
    close(server_fd);

    //cleanup files
    if (file !=NULL){
        fclose(file); 

    }
    // print shutdown now
    printf("shutdown now \n");
}


void insertNode(thread_data_t* data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;

    pthread_mutex_lock(&listMutex);

    if (head == NULL) {
        head = newNode;
    } else {
        Node* current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }

    pthread_mutex_unlock(&listMutex);
}

void removeCompletedThreads() {
    pthread_mutex_lock(&listMutex);

    Node* current = head;
    Node* prev = NULL;

    while (current != NULL) {
        if (current->data->isComplete) {
            // Join and cleanup the thread
            pthread_join(current->data->threadId, NULL);

            // Remove the node from the list
            if (prev == NULL) {
                head = current->next;
                free(current->data);
                free(current);
                current = head;
            } else {
                prev->next = current->next;
                free(current->data);
                free(current);
                current = prev->next;
            }
        } else {
            prev = current;
            current = current->next;
        }
    }

    pthread_mutex_unlock(&listMutex);
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}