#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>


int convertDecimalToOctal(int decimalNumber)
{
    int octalNumber = 0, i = 1;

    while (decimalNumber != 0)
    {
        octalNumber += (decimalNumber % 8) * i;
        decimalNumber /= 8;
        i *= 10;
    }

    return octalNumber;
}

int getChmod(const char *path){
    struct stat ret;
    int octal_mode;
    printf("path: %s\n", path);
    
    if (stat(path, &ret) == -1) {
        return -1;
    }
    
    octal_mode = convertDecimalToOctal(ret.st_mode)%10000;
    printf("%d\n", octal_mode);
    
    return octal_mode;
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
    int result = 0;
    switch (mode[1]){
        case '-':
        {
            if (strcmp(manip, "rwx") == 0) {
                result = 00;
            } else if (strcmp(manip, "rw") == 0) {
                result = 01;
            } else if (strcmp(manip, "rx") == 0) {
                result = 02;
            } else if (strcmp(manip, "r") == 0) {
                result = 03;
            } else if (strcmp(manip, "wx") == 0) {
                result = 04;
            } else if (strcmp(manip, "w") == 0) {
                result = 05;
            } else if (strcmp(manip, "x") == 0) {
                result = 06;
            } else {
                 printf("Error in MODE -> +\n");
            }
            
            break;
        }
        case '+':
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
                printf("Error in MODE -> +\n");
            }
            break;
        }

       
        case '=':
         {
            if (strcmp(manip, "rwx") == 0) {
                result = 7;
            } else if (strcmp(manip, "rw") == 0) {
                result = 6;
            } else if (strcmp(manip, "rx") == 0) {
                result = 5;
            } else if (strcmp(manip, "r") == 0) {
                result = 4;
            } else if (strcmp(manip, "wx") == 0) {
                result = 3;
            } else if (strcmp(manip, "w") == 0) {
                result = 2;
            } else if (strcmp(manip, "x") == 0) {
                result = 1;
            } else {
                printf("Error in MODE -> =\n");
            }
            break;
        }
        default:
            break;
    }

    return result;
}



int checkMode(char* mode, int permission)
{   
    printf("MODE -> %s\n", mode);
    int result = 00;
    char* manip = &mode[2];         // acepts following characters

    printf("MODE -> %s; length -> %ld\n", manip, strlen(manip));
    printf("MODE -> %s; mode[0] -> %c; mode[1] -> %c; manip -> %s\n", mode, mode[0], mode[1], manip);
    if (mode[0] == 'u'){ //1 7 7 7
        if (mode[1] == '='){
            result = checkPermissions(mode, manip)*100;
        } else {

        }
        //result *= 100;
    } else if (mode[0] == 'g'){
        if (mode[1] == '='){
            printf("HERE\n");
            result = checkPermissions(mode, manip)*10;
            printf("%d\n", result);
        } else {

        }
        //result *= 10;
    } else if (mode[0] == 'o'){
        if (mode[1] == '='){
            result = checkPermissions(mode, manip);
        } else {

        }
        //do nothing
    } else if (mode[0] == 'a'){
        if (mode[1] == '='){
            result += checkPermissions(mode, manip) + checkPermissions(mode, manip)*100 + checkPermissions(mode, manip)*10;
        } else {

        }
        //result += result*100 + result*10;
    } else {
        //TODO: error
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
    printf("%d\n", i);
    
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
