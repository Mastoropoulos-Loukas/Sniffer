#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <queue>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <fcntl.h>

#define PERMS 0666

#define READ 0
#define WRITE 1
#define BUFF_SIZE 100

using namespace std;

queue<pid_t> workers;
pid_t lpid;
char path[30];


void showq(queue<pid_t> gq){
    queue<pid_t> g = gq;
    if(!g.empty())cout << g.front();
    g.pop();
    while(!g.empty()){
        cout << "\t" << g.front();
        g.pop();
    }

    cout << endl;
}

char* getFileName(char* buffer){
    int c = 0;
    for(int i = 0; i < 2; i++)while(buffer[c++]!=' ');

    char* res = buffer+c;
    while(buffer[c] && buffer[c]!=' ')c++;
    buffer[c] = '\0';
    return res;
}

void exitManager(int sig){

    char buff[100];
    kill(lpid, SIGINT);
    int childs = workers.size();

    if(workers.empty()){wait(NULL);exit(0);}

    while(!workers.empty()){
        pid_t pid = workers.front();
        workers.pop();
        sprintf(buff, "/tmp/fifo.%i", pid);
        if(unlink(buff)<0)perror("cannot unlink fifo");

        kill(pid, SIGCONT);
        kill(pid, SIGINT);
    }

    childs++;
    for(int i = 0; i < childs; i++)wait(NULL);
    exit(0);
}

void writeToWorker(pid_t wpid, char* message){

    char fifoName[30];
    sprintf(fifoName, "/tmp/fifo.%i", wpid);   

    // printf("Manager trying to open fifo file (to write) %s\n", fifoName);
    int fd = open(fifoName, O_WRONLY);
    if(fd < 0)perror("writing to fifo error");
    write(fd, message, strlen(message));

    // printf("manager wrote message and closed file\n");
    kill(wpid, SIGCONT);
}

pid_t newWorker(char * message){
    // printf("making a new worker\n"); 
    int wpid = fork();

    if(wpid == -1){
        perror("Fork error during worker creation\n");
        exit(1);
    }
    if(!wpid){
        execl("./worker", "worker", path, NULL);
    }

    char fifoName[30];
    sprintf(fifoName, "/tmp/fifo.%i", wpid);

    if(mkfifo(fifoName, PERMS) < 0){
        perror("can't create fifo");
        exit(1);
    }

    // printf("created fifo file %s\n", fifoName);
    
    writeToWorker(wpid, message);
    return wpid;
}

void childStopped(int sig){
    signal(SIGCHLD, childStopped);
    pid_t pid;
    while(!(pid = waitpid(-1, NULL, WNOHANG|WUNTRACED)));
    printf("worker %i stopped\n", pid);
    workers.push(pid);
}

int main(int argc, char** argv){

    pid_t pid, lpid;

    char buff1[BUFF_SIZE], buff2[BUFF_SIZE];
    

    signal(SIGCHLD, childStopped);
    signal(SIGINT, exitManager);

    // printf("Manager sarted (%i). Workes queue is %s empty\n", getpid(), (workers.empty()? " ": "NOT "));
    mkdir("res", 0777);

    //check input;
    if(argc !=1 && argc != 3){
        printf("Usage: ./sniffer [-p paht]\n");
        perror("Wrong Usage");
        exit(1);
    }
    if(argc == 1)strcpy(path, ".");
    else strcpy(path, argv[2]);

    //first pipe (listener - manager)
    int fd1[2];
    if(pipe(fd1) == -1) {perror("pipe creation"); exit(1);}

    //crate listener
    string a1 = to_string(fd1[READ]);
    string a2 = to_string(fd1[WRITE]);

    // printf("argv = %s argc = %i pth: %s\n", argv[2], argc, path);


    lpid = fork();
    if(lpid == -1){perror("fork from sniffer"); exit(1);}
    if(!lpid){
        execl("./listener", "listener", a1.c_str(), a2.c_str(), path, NULL);
    }

    //manager is reader of first pipe
    close(fd1[WRITE]);
    int size;
    // bool test = false;
    while(size = read(fd1[READ], buff1, BUFF_SIZE)){
        buff1[size-1] = '\0';

        // char buffer [30]; strcpy(buffer, "test message");
        char* filename = getFileName(buff1);
        printf("filename : %s\n", filename);
        pid_t wpid;

        //call worker;
        if(workers.empty()){
            wpid = newWorker(filename);
        }
        else{
            pid_t a = workers.front();
            workers.pop();
            writeToWorker(a, filename);
        }
        // if(test) break;
    }
    
    close(fd1[READ]); 
    exitManager(0);
}
