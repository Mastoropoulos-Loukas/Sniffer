#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>

#define READ 0
#define WRITE 1
#define BUFF_SIZE 100

using namespace std;

int main(int argc, char** argv){

    //check input
    if(argc != 4){
        printf("Usage: ./listener fdREAD fdWRITE path\n");
        perror("Worng usage of listener");
        exit(1);
    }


    // printf("pth (listener): %s\n", argv[3]);
    int fd[2];

    fd[0] = atoi(argv[1]);
    fd[1] = atoi(argv[2]);
    
    close(fd[READ]);
    dup2(fd[WRITE], 1);
    close(fd[WRITE]);

    execlp("inotifywait", "inotifywait", "-m", "-e", "moved_to","-e", "create", argv[3], NULL);
    

    return 0;
}