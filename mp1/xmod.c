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
char* filename;

pid_t pid;

int getChmod(const char *path){
    struct stat ret;
    
    if (stat(path, &ret) == -1) {
        return -1;
    }
    //bits is an octal number -> we use int but is octal
    mode_t bits = ret.st_mode%(32768); // 8⁵ -> mode from 101777 to 1777, for example 
    //printf("mode -> %o\n", bits); //print octal, if we use printf("mode -> %d\n", bits); the values are not the same!!

    return bits;
}

char* getPrintedMode(int permission){
    char* final = "";
    final = malloc(10);
    char* medium = "";
    char perm [10];
    sprintf(perm, "%o", permission);
    int i, limit = 0;
    if (strlen(perm) == 4){ //file
        i = 1;
        //medium = "-";
    } else if (strlen(perm) == 5){ //directory
        i = 2;
        //medium = "d";
    } else printf("Error on type of file\n");
    limit = i + 3;
    //strcat(final, medium);
    for (; i < limit; i++){
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
    if (strcmp(option, "-v") == 0){
        if (previous_permission == permission) return 0;
        printf("mode of '%s' retained as %o (%s)\n", path, permission, getPrintedMode(permission));
        return 0;
    } else if (strcmp(option, "-c") == 0){
        if (previous_permission == permission) return 0;
        printf("mode of '%s' changed from %o (%s) to %o (%s)\n", path, previous_permission, getPrintedMode(previous_permission), permission, getPrintedMode(permission));
        return 0;
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
            printf("xmod: invalid mode: '%s'\n", mode);
            return -1;
        }
    } else if (mode[1] == '='){
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
    char* manip = &mode[2];         // acepts mode[2] and following characters
    if (mode[0] == 'u'){
        result = permission ^ (checkPermissions(mode, manip)*64);
    } else if (mode[0] == 'g'){
        result = permission ^ (checkPermissions(mode, manip)*8);
    } else if (mode[0] == 'o'){
        result = permission ^ checkPermissions(mode, manip);
    } else if (mode[0] == 'a'){
        result = permission ^ (checkPermissions(mode, manip) + checkPermissions(mode, manip)*8 + checkPermissions(mode, manip)*64);
    } else {
        printf("xmod: invalid mode: '%s'\n", mode);
        return -1;
    }

    return result;
    
}

char * getEventInfo(event_type events, pid_t pid, int sig, int argc, char *argv[], int current_permission){
    //events
    char buf[10000];
    switch(events){
        case PROC_CREAT:
            snprintf(buf, 1024, "%s ; ","PROC_CREAT");
            for (size_t i; i < argc; i++){
                snprintf(buf + strlen(buf), 1024, "%s\n", argv[i]);
            };
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
            snprintf(buf+strlen(buf), 1024, "%d : %d\n", sig, pid);
            break;

        case FILE_MODF:
            snprintf(buf, 1024, "%s ; ","FILE_MODF");
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
        count = snprintf(buf, 1000, "%f ; ",time_spent);
        write(filedesc, buf, count);  //write instant to filename
        printf("time spent: %s\n",buf);

        count = snprintf(buf, 1000, "%d ; ",pid); 
        write(filedesc, buf, count);  //write pid to filename       
        printf("%s\n",buf);

        count = snprintf(buf, 1000, "%s", event_info);
        write(filedesc, buf, count);  //write pid to filename
        printf("%s\n",buf);

        if(close(filedesc) < 0) 
            printf("Error closing file."); //returns -1 if insuccess

    }
    return 0;
}

static void handler(int signo) {
    char input;
    switch(signo)
    {
        case SIGINT:
            if (getpid() == getpgid(getpid())){ //if group id == process id, it is the parent
                printf("%d ; filename ; ", getpid() ); //TO-DO filename - global variable
                printf("nftot: ; "); //TO-DO number of files already found - global variable
                printf("nfmod: \n"); //TO-DO number of files already modified - global variable

                printf("Terminate program (y/n)\n");
                scanf("%c", &input);
                char input;
                do {
                    if (input == 'y') {
                        printf("\nProcess terminated.\n");
                        kill(0, SIGTERM);
                        exit(0);
                    }
                    else if (input == 'n') {
                        printf("\nProcess will continue.\n");
                        kill(0, SIGCONT);
                    }
                    else {
                        printf("Enter 'y' to terminate, enter 'n' to proceed.");
                        scanf("%c", &input);
                    }
                } while (input != 'y' && input != 'n');
            }
            else {
                signal(SIGCONT,handler);
                signal(SIGTERM,handler);
                pause();
            }
            break;
        default:
            //write records e envia este sinal
            break;
    }
}

int changePerms(char* option, char *mode, char *buf, int permission){
    
    int new_permission;
    char *endptr;

    new_permission = strtol(mode, &endptr, 8);     //Check if a string can be converted to int. Parameters passed by command line are always strings
    if (endptr == mode)        // Not a number - MODE
        new_permission = checkMode(mode, permission);
    
    if (chmod (buf, new_permission) < 0)
    {
        fprintf(stderr, "%s: error in chmod(%s, %s) - %d (%s)\n",
                "./xmod", buf, mode, errno, strerror(errno));
        exit(1);
    }

    getOptions(buf, option, permission, new_permission);
    
    return 0;
}

int main(int argc, char *argv[])
{   
    signal(SIGINT, handler);
    signal(SIGCONT, handler);
    signal(SIGTERM, handler);
    signal(SIGQUIT, handler);
    signal(SIGHUP, handler);
    signal(SIGUSR1, handler);
    signal(SIGUSR2, handler);
    signal(SIGPIPE, handler);
    signal(SIGALRM, handler);
    signal(SIGCHLD, handler);
    

    begin_time = times(time_struct);
    char option[100];
    strcpy(option,argv[1]);
    //printf("%s, %s, %s, %s\n", argv[0], argv[1], argv[2], argv[3]);

    char mode[100]; 
    strcpy(mode,argv[2]);
   
    char buf[100];
    filename = argv[3];
    strcpy(buf,argv[3]);
   
    old_permission = getChmod(buf);

    changePerms(option, mode, buf, old_permission);   
    //printf("Permission changed with success.\n");
    writeRecords(FILE_MODF, 1, argc, argv);  
    return 0;
}
    

//1777 -> -rwxrwxrwt (todas as permissões privado -> utilizador necessita exe permissions)
//7777 -> publico
//1177 -> ---xrwxrwt

//long int strtol(const char *str, char **endptr, int base) 
