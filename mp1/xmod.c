#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>


int main(int argc, char *argv[])
{   
    char mode[100]; 
    strcpy(mode,argv[1]);
    char buf[100];
    strcpy(buf,argv[2]);
    int i;
    i = strtol(mode, 0, 8);
    if (chmod (buf,i) < 0)
    {
        fprintf(stderr, "%s: error in chmod(%s, %s) - %d (%s)\n",
                argv[0], buf, mode, errno, strerror(errno));
        exit(1);
    }*/
    printf("Permission changed with success.\n");
    return(0);
}

//1777 -> -rwxrwxrwt (todas as permissões privado -> utilizador necessita exe permissions)
//7777 -> publico
//1177 -> ---xrwxrwt

//long int strtol(const char *str, char **endptr, int base) 
