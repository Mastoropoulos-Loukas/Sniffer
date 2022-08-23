#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>
#include <string>
#include <fcntl.h>
#include <map>

#define READ 0
#define WRITE 1
#define BUFF_SIZE 100

#define RES_DIR "./res/"

using namespace std;

int fd;

void exitWorker(int sig){
    close(fd);
    exit(0);
}

map<string, int> links;

void getlinks(char *buffer){
    int c ;
    int cl;
    char link[30];
    for(c = 0; c < BUFF_SIZE; c++){
        if( buffer[c] == 'h' && buffer[c+1] == 't' && buffer[c+2] == 't' && buffer[c+3] == 'p' && buffer[c+4] == ':' && buffer[c+5] == '/' && buffer[c+6] == '/' ){
            c += 7; //skip http://
            c += 4; //skip www.
            cl = 0;
            while(buffer[c] && buffer[c] != ' '){
                link[cl] = buffer[c];
                c++; cl++;
            }
            link[cl] = '\0';
            map<string, int>::iterator itr;
            if( links.empty() || links.find(link) == links.end()){
                //add key
                links.insert(pair<string, int>(link, 1));
            }
            else{
                itr = links.find(link);
                itr->second++;
            }
        }
    }

}

void printLinks(){
    for(map<string, int>::iterator itr = links.begin(); itr != links.end(); ++itr){
        cout << itr->first << " " << itr->second << endl;
    }
}

void writeLinks(int fd){
    string line;
    for(map<string, int>::iterator itr = links.begin(); itr != links.end(); ++itr){
        line =  itr->first + " " + to_string(itr->second) + "\n";
        write(fd, line.c_str(), line.length());
    }
}

void work(string filename, string path){
    string home = RES_DIR;
    string ext = ".out";
    string dest = home+filename+ext;
    string src = path+ "/" + filename;

    int fdr, fdw;
    
    fdr = open(src.c_str(), O_RDONLY);
    if(fdr < 0)perror("read error");
   
    mode_t fdmode = (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    fdw = open(dest.c_str(), O_WRONLY|O_CREAT|O_TRUNC, fdmode);

    char buffer[BUFF_SIZE];

    string line;
    int n;
    while((n=read(fdr, buffer, BUFF_SIZE))> 0){
        getlinks(buffer);
    }


    // printf("got \n");
    // printLinks();
    // printf("got here\n");
    writeLinks(fdw);

    close(fdw);
    close(fdr);
}

int main(int argc, char** argv){

    signal(SIGINT, exitWorker);

    if(argc != 2){perror("Wrong worker usage"); exit(1);}
    // printf("Worker %i (%i) created\n", getpid(), getppid());
    
    char filename[50];
    sprintf(filename, "/tmp/fifo.%i", getpid());
    char buff[BUFF_SIZE];
    fd = open(filename, O_RDONLY);
    

    while(1){
        int size = read(fd, buff, BUFF_SIZE);
        buff[size] = '\0';
        printf("Worker read: %s\n", buff);
        //do work
        work(buff, argv[1]);
        links.clear();
        kill(getpid(), SIGSTOP);
    }
    
    return 0;
}
