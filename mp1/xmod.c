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

int getChmod(const char *path){
    struct stat ret;
    //printf("path: %s\n", path);
    
    if (stat(path, &ret) == -1) {
        return -1;
    }
    //bits is an octal number -> we use int but is octal
    mode_t bits = ret.st_mode%(32768); // 8⁵ -> mode from 101777 to 1777, for example 
    //printf("mode -> %o\n", bits); //print octal, if we use printf("mode -> %d\n", bits); the values are not the same!!

    return bits;
}

//TODO - apresentar entre parênteses de cada option o que equivale em cada -> (-rwxrwxrwx), por exemplo
int getOptions(const char *path, char* option, int previous_permission, int permission){
    if (strcmp(option, "-v") == 0){
        printf("mode of '%s' retained as %o ()\n", path, permission);
    } else if (strcmp(option, "-c") == 0){
        printf("mode of '%s' changed from %o () to %o ()\n", path, previous_permission, permission);
    } else if (strcmp(option, "-R") == 0){
        

        //TODO
    }
    printf("Error in option.");
    return 1;
}


int checkPermissions(char*mode, char* manip)
{
    int result = 00;
    if (mode[1] == '-' || mode[1] == '+')
    {
        if (strcmp(manip, "rwx") == 0) {
            result = 07;
        } else if (strcmp(manip, "rw") == 0) {
            result = 06;
        } else if (strcmp(manip, "rx") == 0) {
            result = 05;
        } else if (strcmp(manip, "r") == 0) {
            result = 04;
        } else if (strcmp(manip, "wx") == 0) {
            result = 03;
        } else if (strcmp(manip, "w") == 0) {
            result = 02;
        } else if (strcmp(manip, "x") == 0) {
            result = 01;
        } else {
            printf("Error in MODE: <rwx>\n");
        }
    } else if (mode[1] == '='){
        if (strcmp(manip, "rwx") == 0) {
            result = 07;
        } else if (strcmp(manip, "rw") == 0) {
            result = 06;
        } else if (strcmp(manip, "rx") == 0) {
            result = 05;
        } else if (strcmp(manip, "r") == 0) {
            result = 04;
        } else if (strcmp(manip, "wx") == 0) {
            result = 03;
        } else if (strcmp(manip, "w") == 0) {
            result = 02;
        } else if (strcmp(manip, "x") == 0) {
            result = 01;
        } else {
            printf("Error in MODE: <rwx>\n");
        }
    } else {
        printf("Error in MODE: <-|+|=>\n");
    }

    return result;
}


char * getEventInfo(event_type events, pid_t pid, int sig, int argc, char *argv[], int current_permission){
    //events
    char buf[10000];
    switch(events){
        case PROC_CREAT:
            snprintf(buf, 1024, "%s\n","PROC_CREAT");
            for (size_t i; i < argc; i++){
                snprintf(buf + strlen(buf), 1024, "%s\n", argv[i]);
            };
            break;

        case PROC_EXIT:
            snprintf(buf, 1024, "%s\n","PROC_EXIT");
            snprintf(buf+strlen(buf), 1024, "%d\n", sig);
            break;

        case SIGNAL_RECV:
            snprintf(buf, 1024, "%s\n","SIGNAL_RECV");
            snprintf(buf+strlen(buf), 1024, "%d\n", sig);
            break;

        case SIGNAL_SENT:
            snprintf(buf, 1024, "%s\n","SIGNAL_SENT");
            snprintf(buf+strlen(buf), 1024, "%d : %d\n", sig, pid);
            break;

        case FILE_MODF:
            snprintf(buf, 1024, "%s\n","FILE_MODF");
            char path[1000];
            strcpy(path, argv[argc-1]);
            snprintf(buf+strlen(buf), 1024, "%s : %o : %o\n", path, old_permission, current_permission);
            break;

        default:
            break;
    }
    char * buf_cop;
    buf_cop = buf;
    return buf_cop;
}


int writeRecords(event_type event, int sig, int argc, char *argv[]){
    char *filename;
    //user sets filename: export LOG_FILENAME='filename' (incluir pelicas) no terminal
    filename = getenv("LOG_FILENAME");   //check if variable LOG-FILENAME is set
    if (filename != NULL) {
        printf("Filename: %s\n",filename);
        size_t filedesc = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0777); //TODO: truncate can be problematic//(create file if it does not exist, else truncate)
        if(filedesc < 0)      //open returns -1 upon failure
            printf("Error opening file.");

        char buf[1024];

       //instant
        long int ticks = sysconf(_SC_CLK_TCK); 
        clock_t end_time = times(time_struct);
        double time_spent = (double)(end_time - begin_time) / ticks;
        
        //pid
        pid_t pid = getpid();

        char path[100];
        strcpy(path,argv[3]);
        int cur_perm = getChmod(path);
        //events
        char * event_info = getEventInfo(event, pid, sig, argc, argv, cur_perm);

        int count;
        //snprintf formates and stores chars in buf, 2nd arg = max_num_chars
        count = snprintf(buf, 1000, "%f\n",time_spent);
        write(filedesc, buf, count);  //write instant to filename
        printf("time spent: %s",buf);

        count = snprintf(buf, 1000, "%d\n",pid); 
        write(filedesc, buf, count);  //write pid to filename       
        printf("%s\n",buf);

        count = snprintf(buf, 1000, "%s\n", event_info);
        write(filedesc, buf, count);  //write pid to filename
        printf("%s\n",buf);

        if(close(filedesc) < 0) 
            printf("Error closing file."); //returns -1 if insuccess

    }
    return 0;
}

int checkMode(char* mode, int permission)
{   
    //printf("MODE -> %s\n", mode);
    int result = 00;
    char* manip = &mode[2];         // acepts mode[2] and following characters

    //printf("MODE -> %s; length -> %ld\n", manip, strlen(manip));
    //printf("MODE -> %s; mode[0] -> %c; mode[1] -> %c; manip -> %s\n", mode, mode[0], mode[1], manip);
    if (mode[0] == 'u'){
        if (mode[1] == '='){
            result = checkPermissions(mode, manip)*64;
        } else {
            result = permission ^ (checkPermissions(mode, manip)*64);
        }
    } else if (mode[0] == 'g'){
        if (mode[1] == '='){
            result = checkPermissions(mode, manip)*8;
        } else {
            result = permission ^ (checkPermissions(mode, manip)*8);
        }
    } else if (mode[0] == 'o'){
        if (mode[1] == '='){
            result = checkPermissions(mode, manip);
        } else {
            result = permission ^ checkPermissions(mode, manip);
        }
    } else if (mode[0] == 'a'){
        if (mode[1] == '='){
            result = checkPermissions(mode, manip) + checkPermissions(mode, manip)*8 + checkPermissions(mode, manip)*64;
        } else {
            result = permission ^ (checkPermissions(mode, manip) + checkPermissions(mode, manip)*8 + checkPermissions(mode, manip)*64);
        }
    } else {
        printf("Error in user: <u|g|o|a>\n");
    }

    return result;
    
}

int changePerms(char* option, char *mode, char *buf, int permission){
    int i;
    char *endptr;

    i = strtol(mode, &endptr, 8);     //Check if a string can be converted to int. Parameters passed by command line are always strings
    if (endptr == mode)        // Not a number - MODE
        i = checkMode(mode, permission);

    getOptions(buf, option, permission, i);
    
    //printf("%o\n", i);
    
    if (chmod (buf, i) < 0)
    {
        fprintf(stderr, "%s: error in chmod(%s, %s) - %d (%s)\n",
                "./xmod", buf, mode, errno, strerror(errno));
        exit(1);
    }
    
    return 0;
}

int main(int argc, char *argv[])
{   
    begin_time = times(time_struct);
    char option[100];
    strcpy(option,argv[1]);
    //printf("%s, %s, %s, %s\n", argv[0], argv[1], argv[2], argv[3]);

    char mode[100]; 
    strcpy(mode,argv[2]);
   
    char buf[100];
    strcpy(buf,argv[3]);
   
    old_permission = getChmod(buf);
    int permission = getChmod(buf);

    changePerms(option, mode, buf, permission);   
    //printf("Permission changed with success.\n");
    writeRecords(FILE_MODF, 1, argc, argv);  
    return 0;
}
    

//1777 -> -rwxrwxrwt (todas as permissões privado -> utilizador necessita exe permissions)
//7777 -> publico
//1177 -> ---xrwxrwt

//long int strtol(const char *str, char **endptr, int base) 
