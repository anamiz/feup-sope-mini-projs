#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>


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
            printf("xmod: invalid mode: '%s'\n", mode);
            return -1;
        }
    } else if (mode[1] == '='){
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
    
    if (chmod (buf, i) < 0)
    {
        fprintf(stderr, "%s: error in chmod(%s, %s) - %d (%s)\n",
                argv[0], buf, mode, errno, strerror(errno));
        exit(1);
    }
    //printf("Permission changed with success.\n");
    getOptions(buf, option, permission, i);
    return 0;
}
    

//1777 -> -rwxrwxrwt (todas as permissões privado -> utilizador necessita exe permissions)
//7777 -> publico
//1177 -> ---xrwxrwt

//long int strtol(const char *str, char **endptr, int base) 
