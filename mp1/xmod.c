#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>


int getChmod(const char *path){
    struct stat ret;
    printf("path: %s\n", path);
    
    if (stat(path, &ret) == -1) {
        return -1;
    }
    //bits is an octal number -> we use int but is octal
    mode_t bits = ret.st_mode%(32768); // 8⁵ -> mode from 101777 to 1777, for example 
    printf("mode -> %o\n", bits); //print octal, if we use printf("mode -> %d\n", bits); the values are not the same!!

    return bits;
}

int getOptions(char* option){
    if (option[0] == '-' ){
        if (option[1] == 'c'|| option[1] == 'v' ||option[1] == 'R' ){
            return 0;
        }
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



int checkMode(char* mode, int permission)
{   
    printf("MODE -> %s\n", mode);
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


int main(int argc, char *argv[])
{   
    char option[100];
    strcpy(option,argv[1]);
    //printf("%s, %s, %s, %s\n", argv[0], argv[1], argv[2], argv[3]);

    char mode[100]; 
    strcpy(mode,argv[2]);
    char *endptr;
    char buf[100];
    strcpy(buf,argv[3]);
    
    int permission = getChmod(buf);

    int i;
    i = strtol(mode, &endptr, 8);     //Check if a string can be converted to int. Parameters passed by command line are always strings
    if (endptr == mode)        // Not a number - MODE
        i = checkMode(mode, permission);
    
    printf("%o\n", i);
    
    if (chmod (buf, i) < 0)
    {
        fprintf(stderr, "%s: error in chmod(%s, %s) - %d (%s)\n",
                argv[0], buf, mode, errno, strerror(errno));
        exit(1);
    }
    printf("Permission changed with success.\n");

    return 0;
}
    

//1777 -> -rwxrwxrwt (todas as permissões privado -> utilizador necessita exe permissions)
//7777 -> publico
//1177 -> ---xrwxrwt

//long int strtol(const char *str, char **endptr, int base) 
