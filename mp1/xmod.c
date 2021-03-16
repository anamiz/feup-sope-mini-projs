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
    printf("path: %s\n", path);
    
    if (stat(path, &ret) == -1) {
        return -1;
    }
    
   printf("%d\n", convertDecimalToOctal(ret.st_mode)%10000);

   int octal_mode = 00;
   octal_mode = convertDecimalToOctal(ret.st_mode)%10000;

   return octal_mode;
}

int getOptions(char* option[]){
    if (option[0] == '-' ){
        if (option[1] == 'c'|| option[1] == 'v' ||option[1] == 'R' ){
            return 0;
        }
    } 
    printf("Error in option.");
    return 1;
}

int checkMode(char* mode, int permission)
{
    int result = 00;
    char* manip;

    for (int i = 2; i <= strlen(mode); i++)
    {
        manip += mode[i];
    }

    printf("MODE -> %s", manip);

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
                //TODO: error
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
                //TODO: error
            }
            break;
        }

       
        case '=':
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
                //TODO: error
            }
            break;
        }
        default:
            break;
    }


    if (mode[0] == 'u'){ //1 7 7 7
   
        
       
    } else if (mode[0] == 'g'){
        result *= 10;
    } else if (mode[0] == 'o'){
        //do nothing
        result *= 10;
    } else if (mode[0] == 'a'){
        result += result*100 + result*10;
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
    int i;
    i = strtol(mode, &endptr, 8);     //Check if a string can be converted to int. Parameters passed by command line are always strings
    int permission = getChmod(buf);
    if (endptr == mode)        // Not a number - MODE
        i = checkMode(mode, permission);
    if (chmod (buf, i) < 0)
    {
        fprintf(stderr, "%s: error in chmod(%s, %s) - %d (%s)\n",
                argv[0], buf, mode, errno, strerror(errno));
        exit(1);
    }
    printf("Permission changed with success.\n");
    

    return 0;
}
    
    /*char mode[100]; 
    strcpy(mode,argv[2]);

    if (isdigit(mode[0])) // mode is a number - MODE-OCTAL
    {
        char path[100];
        strcpy(path,argv[3]);
        int i;
        i = strtol(mode, 0, 8);     //Check if a string can be converted to int. Parameters passed by command line are always strings
        if (chmod (path, i) < 0)
        {
            fprintf(stderr, "%s: error in chmod(%s, %s) - %d (%s)\n",
                    argv[0], path, mode, errno, strerror(errno));
            exit(1);
        }
        printf("Permission changed with success.\n");
        
    }

    else if (isalpha(mode[0])) // mode is a string - MODE 
    {

        char path[100];
        strcpy(path,argv[3]);

    
        if (chmod (path, mode) < 0)
        {
            fprintf(stderr, "%s: error in chmod(%s, %s) - %d (%s)\n",
                    argv[0], path, mode, errno, strerror(errno));
            exit(1);
        }
        printf("Permission changed with success.\n");

    } else {
       
    }

    return 0;*/


//1777 -> -rwxrwxrwt (todas as permissões privado -> utilizador necessita exe permissions)
//7777 -> publico
//1177 -> ---xrwxrwt

//long int strtol(const char *str, char **endptr, int base) 
