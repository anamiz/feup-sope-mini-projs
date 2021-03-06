#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/times.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <signal.h>


enum events
{
    PROC_CREAT,  
    PROC_EXIT,   
    SIGNAL_RECV,
    SIGNAL_SENT,
    FILE_MODF
};

typedef enum events event_type;

struct tms * time_struct;
clock_t begin_time;
int old_permission;
char path_name[1000];
int nftot;
int nfmod;
pid_t pid;
char * filename;
char msg[1000000000];


int getChmod(const char *path){
    struct stat ret;
    
    if (stat(path, &ret) == -1) {
        return -1;
    }
    //bits is an octal number -> we use int but is octal
    mode_t bits = ret.st_mode % 32768; 
    // 8⁵ -> mode from 101777 to 1777, for example 
    //printf("mode -> %o\n", bits); //print octal, if we use printf("mode -> %d\n", bits); the values are not the same!!
    return bits;
}

char* getPrintedMode(int permission){
    char* final = "";
    final = malloc(10);
    char* medium = "";
    char perm [10];
    sprintf(perm, "%o", permission);
    for (int i = 0; i < 3; i++){
        if (perm[i] == '0') {
            medium = "---";
        } else if (perm[i] == '1'){
            medium = "--x";
        } else if (perm[i] == '2'){
            medium = "-w-";
        } else if (perm[i] == '3'){
            medium = "-wx";
        } else if (perm[i] == '4'){
            medium = "r--";
        } else if (perm[i] == '5'){
            medium = "r-x";
        } else if (perm[i] == '6'){
            medium = "rw-";
        } else if (perm[i] == '7'){
            medium = "rwx";
        }
        strcat(final, medium);
    }
    return final;
}

int getOptions(const char *path, char* option, int previous_permission, int permission){
    
    if (strcmp(option, "-v" ) == 0 || strcmp(option, "-vR") == 0 || strcmp(option, "-Rv") == 0 ||
        strcmp(option, "-v-R") == 0 || strcmp(option, "-R-v") == 0){
        if (previous_permission == permission) return 0;
        printf("mode of '%s' retained as %o (%s)\n", path, permission, getPrintedMode(permission));
        return 0;
    } else if (strcmp(option, "-c") == 0 || strcmp(option, "-vc") == 0 || strcmp(option, "-cv") == 0 || 
                strcmp(option, "-cR") == 0 || strcmp(option, "-Rc") == 0 || strcmp(option, "-Rcv") == 0 ||
                strcmp(option, "-Rvc") == 0 || strcmp(option, "-cvR") == 0 || strcmp(option, "-cRv") == 0 ||
                strcmp(option, "-vcR") == 0 || strcmp(option, "-vRc") == 0 || strcmp(option, "-R -c -v") == 0 || 
                strcmp(option, "-c-R-v") == 0 || strcmp(option, "-c-v-R") == 0 || strcmp(option, "-v-R-c") == 0 ||
                strcmp(option, "-v-c-R") == 0 || strcmp(option, "-R-v-c") == 0 || strcmp(option, "-v-c") == 0 ||
                strcmp(option, "-c-R") == 0 || strcmp(option, "-R-c") == 0 || strcmp(option, "-c-v") == 0 ||
                strcmp(option, "-R") == 0 ){
        if (previous_permission == permission) return 0;
        printf("mode of '%s' changed from %o (%s) to %o (%s)\n", path, previous_permission, getPrintedMode(previous_permission), permission, getPrintedMode(permission));
        return 0;
    } /*else if (strcmp(option, "-R") == 0){
        if (previous_permission == permission) return 0;
        printf("mode of '%s' changed from %o (%s) to %o (%s)\n", path, previous_permission, getPrintedMode(previous_permission), permission, getPrintedMode(permission));
        return 0;
    }*/ else {
        printf("xmod: invalid option -- '%s'\n", option);
    }
    printf("Error in option.");
    return 1;
}

int checkPermissions(char*mode, char* manip)
{
    int result = 00;
    if (mode[1] == '+' || mode[1] == '=')
    {
        if ((strcmp(manip, "rwx") == 0) || (strcmp(manip, "rxw") == 0) || (strcmp(manip, "xrw") == 0) || (strcmp(manip, "xwr") == 0) || (strcmp(manip, "wrx") == 0) || (strcmp(manip, "wxr") == 0)) {
            result = 07;
        } else if ((strcmp(manip, "rw") == 0) || (strcmp(manip, "wr") == 0)) {
            result = 06;
        } else if ((strcmp(manip, "rx") == 0) || (strcmp(manip, "xr") == 0)) {
            result = 05;
        } else if (strcmp(manip, "r") == 0) {
            result = 04;
        } else if ((strcmp(manip, "wx") == 0) || (strcmp(manip, "xw") == 0)) {
            result = 03;
        } else if (strcmp(manip, "w") == 0) {
            result = 02;
        } else if (strcmp(manip, "x") == 0) {
            result = 01;
        } else {
            printf("xmod; invalid mode: '%s'\n", mode);
            return -1;
        }
    } else if (mode[1] == '-'){
        if ((strcmp(manip, "rwx") == 0) || (strcmp(manip, "rxw") == 0) || (strcmp(manip, "xrw") == 0) || (strcmp(manip, "xwr") == 0) || (strcmp(manip, "wrx") == 0) || (strcmp(manip, "wxr") == 0)) {
            result = 00;
        } else if ((strcmp(manip, "rw") == 0) || (strcmp(manip, "wr") == 0)) {
            result = 01;
        } else if ((strcmp(manip, "rx") == 0) || (strcmp(manip, "xr") == 0)) {
            result = 02;
        } else if (strcmp(manip, "r") == 0) {
            result = 03;
        } else if ((strcmp(manip, "wx") == 0) || (strcmp(manip, "xw") == 0)) {
            result = 04;
        } else if (strcmp(manip, "w") == 0) {
            result = 05;
        } else if (strcmp(manip, "x") == 0) {
            result = 06;
        } else {
            printf("xmod: invalid mode: '%s'\n", mode);
            return -1;
        }
    } else {
        printf("xmod: invalid mode: '%s'\n", mode);
        return -1;
    }

    return result;
}

int checkMode(char* mode, int permission)
{   
    
    int result = 00;
    char* manip = &mode[2];   // accepts mode[2] and following characters
    printf("%o", permission);
    printf("\n");

    //everything checked until here
    int temp = permission;
    if (mode[0] == 'u'){
        //result = permission ^ (checkPermissions(mode, manip)*64);
        if (mode[1] == '=') {
            temp = temp & 07077;
            printf("%o\n", temp);
            result = temp | (checkPermissions(mode, manip)*64);
        }
        else if (mode[1] == '+'){
            result = permission | ((checkPermissions(mode, manip)*64) & 00700);
        }
        else if (mode[1] == '-'){
            result = permission & ((checkPermissions(mode, manip)*64) | 07077);
        }
    } else if (mode[0] == 'g'){
        //result = permission ^ (checkPermissions(mode, manip)*8);
        if (mode[1] == '=') {
            temp = temp & 07707;
            printf("%o\n", temp);
            result = temp | (checkPermissions(mode, manip)*8);
        }
        else if (mode[1] == '+'){
            result = permission | ((checkPermissions(mode, manip)*8) & 00070);
        }
        else if (mode[1] == '-'){
            result = permission & ((checkPermissions(mode, manip)*8) | 07707);
        }
    } else if (mode[0] == 'o'){
        //result = permission ^ checkPermissions(mode, manip);
        if (mode[1] == '=') {
            temp = temp & 07770; // temp = permission & 07770 better
            printf("%o\n", temp);
            result = temp | checkPermissions(mode, manip);
        }
        else if (mode[1] == '+'){
            result = permission | (checkPermissions(mode, manip) & 00007);
        }
        else if (mode[1] == '-'){
            result = permission & (checkPermissions(mode, manip) | 07770);
        }
    } else if (mode[0] == 'a'){
        //result = permission ^ (checkPermissions(mode, manip) + checkPermissions(mode, manip)*8 + checkPermissions(mode, manip)*64);
        if (mode[1] == '=') {
            temp = temp & 07000;
            printf("%o\n", temp);
            result = temp | checkPermissions(mode, manip) | (checkPermissions(mode, manip)*8) | (checkPermissions(mode, manip)*64);
        }
        else if (mode[1] == '+'){
            result = permission | (checkPermissions(mode, manip) & 00007) | ((checkPermissions(mode, manip)*8) & 00070) | ((checkPermissions(mode, manip)*64) & 00700);
        }
        else if (mode[1] == '-'){
            result = permission & (checkPermissions(mode, manip) | 07770) & ((checkPermissions(mode, manip)*8) | 07707) & ((checkPermissions(mode, manip)*64) | 07077);
        }
    } else {
        printf("xmod: invalid mode: '%s'\n", mode);
        return -1;
    }
    printf("%o", result);
    printf("\n");
    return result;
    
}

void argToStr(int argc, char* argv[], char* msg){
    strcpy(msg, "");
    for (unsigned i = 1; i < argc; i++) {
        strcat(msg, argv[i]);
        if (i < argc - 1)
            strcat(msg, " ");
    }
}

char * getEventInfo(event_type events, int sig, int current_permission){
    //events
    char buf[10000];
    switch(events){
        case PROC_CREAT:
            snprintf(buf, 1024, "%s ; ","PROC_CREAT");
            snprintf(buf+strlen(buf),strlen(msg)+1, "%s\n",msg);
            break;

        case PROC_EXIT:
            snprintf(buf, 1024, "%s ; ","PROC_EXIT");
            snprintf(buf+strlen(buf), 1024, "%d\n", sig);
            break;

        case SIGNAL_RECV:
            snprintf(buf, 1024, "%s ; ","SIGNAL_RECV");
            snprintf(buf+strlen(buf), 1024, "%d\n", sig);
            break;

        case SIGNAL_SENT:
            snprintf(buf, 1024, "%s ; ","SIGNAL_SENT");
            snprintf(buf+strlen(buf), 1024, "%d : %d\n", sig, getpid());
            break;

        case FILE_MODF:
            snprintf(buf, 1024, "%s ; ","FILE_MODF");
            snprintf(buf + strlen(buf), 1024, "%s : %o : %o\n", path_name, old_permission, current_permission);
            break;

        default:
            break;
    }
    char * buf_cop;
    buf_cop = buf;
    return buf_cop;
}

int writeRecords(event_type event, pid_t cur_pid, int sig){
    //user sets filename: export LOG_FILENAME='filename' (incluir pelicas) no terminal
    filename = getenv("LOG_FILENAME");   //check if variable LOG-FILENAME is set
    if (filename != NULL) {
        printf("Filename: %s\n",filename);
        size_t filedesc = open(filename, O_RDWR | O_CREAT | O_APPEND, 0777); //TODO: truncate can be problematic//(create file if it does not exist, else truncate)
        if(filedesc < 0)      //open returns -1 upon failure
            printf("Error opening file.");

        char buf[1024];

       //instant
        long int ticks = sysconf(_SC_CLK_TCK); 
        clock_t end_time = times(time_struct);
        double time_spent = (double)(end_time - begin_time) / ticks * 60;

        int cur_perm = getChmod(path_name);
        //events
        char * event_info = getEventInfo(event, sig, cur_perm);

        if(event == SIGNAL_SENT){
            cur_pid = pid;
        }

        int count;
        //snprintf formates and stores chars in buf, 2nd arg = max_num_chars
        count = snprintf(buf, 1000, "%4.5f ; ",time_spent);
        write(filedesc, buf, count);  //write instant to filename
        printf("%s ",buf);

        count = snprintf(buf, 1000, "%d ; ",cur_pid); 
        write(filedesc, buf, count);  //write pid to filename       
        printf("%s",buf);

        count = snprintf(buf, 1000, "%s", event_info);
        printf("%s",buf);

        write(filedesc, buf, count);  //write event info to filename

        //count = snprintf(buf, 1000, "%s","\n");
        //write(filedesc, buf, count);

        if(close(filedesc) < 0) 
            printf("Error closing file."); //returns -1 if insuccess

    }
    return 0;
}

void initFile(){
    filename = getenv("LOG_FILENAME");   //check if variable LOG-FILENAME is set
    if (filename != NULL) {
        size_t filedesc = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0777);
    if(close(filedesc) < 0) 
            printf("Error closing file."); //returns -1 if insuccess

    }
}



void sigHandler(int signo) { 
    char input;
    writeRecords(SIGNAL_SENT, getpid(),signo); 
    switch(signo)
    {
        case SIGINT:
            if (getpid() == getpgrp()){ //if group id == process id, it is the parent

                printf("\n%d ; %s ;",getpid(),path_name); 
                printf(" nftot: %d ;",nftot); 
                printf(" nfmod: %d \n", nfmod); 
                printf("Terminate program (y/n)\n");
                scanf("%c", &input);
            
                while (input != 'y' && input != 'n'){
                        printf("Enter 'y' to terminate, enter 'n' to proceed.");
                        scanf("%c", &input);
                } 
                if (input == 'y') {
                    printf("\nProcess terminated.\n");
                    kill(0, SIGTERM);
                    writeRecords(PROC_EXIT, getpid(), SIGTERM);
                    exit(0);
                }
                else if (input == 'n') {
                    printf("\nProcess will continue.\n");
                    kill(0, SIGCONT);     
                    writeRecords(PROC_EXIT, getpid(), SIGCONT);
                    input = '0';               
                }
                signal(SIGINT,sigHandler); 

            }
            else {
                signal(SIGCONT,sigHandler);
                signal(SIGTERM,sigHandler);
                pause();
            }
            break;
        case SIGCONT: 
            writeRecords(SIGNAL_SENT, getpid(),signo);
            writeRecords(SIGNAL_RECV,getpid(),signo);
            break;
        case SIGTERM:
            writeRecords(SIGNAL_SENT, getpid(),signo);
            writeRecords(SIGNAL_RECV,getpid(),signo);
            exit(0);
            break;
        default:
            writeRecords(SIGNAL_RECV,getpid(),signo);
            break;
    }
}

int changePerms(char* option, char *mode, char *buf, int permission){
    
    int new_permission;
    char *endptr;
    
    new_permission = strtol(mode, &endptr, 8);     //Check if a string can be converted to int. Parameters passed by command line are always strings

    if (endptr == mode)        // Not a number - MODE
        new_permission = checkMode(mode, permission);
    else if (mode[0] != '0' && strlen(mode) == 4) { //OCTAL must start with '0'
        printf("\nOctal-mode permission must start with a '0'.\n");
        exit(0);
    }

    if (chmod (buf, new_permission) < 0)
    {
        fprintf(stderr, "%s: error in chmod(%s, %s) - %d (%s)\n",
                "./xmod", buf, mode, errno, strerror(errno));
        exit(1);
    }
    nftot++;

    if(permission != new_permission) nfmod++;
    getOptions(buf, option, permission, new_permission);

    writeRecords(FILE_MODF,getpid(),new_permission);
    
    return 0;
}

int changeDirPerms(int argc, char *argv[]){ //AMGS PODEM ME AJUDAR PARA EU PODER IR DORMIR AS 19 POR FAVOR OBG BJ SBJSBSJS BSJ
    int id;
    DIR *dir_desc;
    struct dirent * dir;
    int count_children;
    
    char option[100];
    strcpy(option,argv[1]);
    //printf("%s, %s, %s, %s\n", argv[0], argv[1], argv[2], argv[3]);

    char mode[100]; 
    strcpy(mode,argv[argc-2]);

    char buf[100]; 

    old_permission = getChmod(argv[argc-1]); // TO-DO, chmod is returning 41777 instead of 1777
    
    changePerms(option, mode, argv[argc-1], old_permission);

    if ((dir_desc = opendir(argv[argc-1])) == NULL) 
        exit(0);   //opendir(path) -> last argument of argv is path

  
    while ((dir = readdir(dir_desc)) != NULL) { //copy to args all the elements in argv     

        char ** args = malloc((argc+1)* sizeof(char *));
        
        for(int i = 0; i < argc; i++){
            size_t size = strlen(argv[i]) + 1; 
            args[i] = malloc(size);
            memcpy(args[i],argv[i],size);
        }

        args[argc] = NULL;                      //by default, last argument is null
        strcat(args[argc -1], "/");             //"/" at the end of path

        if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
            continue;
        
        strcpy(buf,argv[argc-1]);
        strcat(buf, "/");
        strcat(buf, dir->d_name);
        printf(" path %s\n",buf);

        struct stat ret;
        if (stat(buf, &ret) == -1) {
            return -1;
        }

        if (S_ISDIR(ret.st_mode)){
            id = fork();
            sleep(3);
            if(id == 0) { // filho 
                strcat(argv[argc - 1], "/");
                strcat(argv[argc - 1], dir->d_name);
                execvp("./xmod", argv); 
                exit(0);
                writeRecords(PROC_EXIT,getpid(),0);
            }
            if(id < 0){
                printf("Error\n");
            }
            else{               
                count_children++;
            } 
        } else { //it's a file
            changePerms(option, mode, args[argc-1], old_permission); 
        }    
    }
    
    while(count_children > 0){
        wait(NULL);
        count_children--;
    }

    if(closedir(dir_desc) < 0)
        printf("Error closing dir\n");
    return 0;
}

void initSignals(){
    signal(SIGINT, sigHandler);
    signal(SIGQUIT, sigHandler);
    signal(SIGHUP, sigHandler);
    signal(SIGUSR1, sigHandler);
    signal(SIGUSR2, sigHandler);
    signal(SIGPIPE, sigHandler);
    signal(SIGALRM, sigHandler);
    signal(SIGCHLD, sigHandler);
}

int main(int argc, char *argv[])
{   
    begin_time = times(time_struct);
    argToStr(argc, argv, msg);

    if(getpid() == getpgrp())
        initFile();

    writeRecords(PROC_CREAT,getpid(),0);
    pid = getpid();
    initSignals();

    char option[100];
    //strcpy(option,argv[1]);
    char* medium;
    for (int i = 1; i < 4; i++)
    {
        if (argv[i][0] == '-') {
            medium = argv[i];
            strcat(option, medium);
        }
        
    }
    
    char mode[100]; 
    strcpy(mode,argv[argc-2]);
   
    char buf[100];
    strcpy(buf,argv[argc-1]);
    
    strcpy(path_name,buf);

    struct stat ret;
    if (stat(buf, &ret) == -1) {
        return -1;
    }

    old_permission = getChmod(buf); // TO-DO chmod returnin 41777 instead of 1777
    if(strcmp(option, "R") == 0 || strcmp(option, "-cR") == 0 || strcmp(option, "-c-R") == 0 || strcmp(option, "-vR") == 0 ||
        strcmp(option, "-v-R") == 0 || strcmp(option, "-cvR") == 0 || strcmp(option, "-c-v-R") == 0 || strcmp(option, "-vcR") == 0 ||
        strcmp(option, "-v-c-R") == 0 || strcmp(option, "-vRc") == 0 || strcmp(option, "-v-R-c") == 0 || strcmp(option, "-c-R-v") == 0 ||
        strcmp(option, "-cRv") == 0 || strcmp(option, "-Rvc") == 0 || strcmp(option, "-R-v-c") == 0 || strcmp(option, "-R-c-v") == 0 ||
        strcmp(option, "-Rcv") == 0){
       changeDirPerms(argc, argv);
    }
    else{
        changePerms(option, mode, buf, old_permission);
    }

    writeRecords(PROC_EXIT, getpid(), 0);
    return 0;
}
    
//1777 -> -rwxrwxrwt (todas as permissões privado -> utilizador necessita exe permissions)
//7777 -> publico
//1177 -> ---xrwxrwt

//long int strtol(const char *str, char **endptr, int base) 
