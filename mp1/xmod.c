#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>


int main(int argc, char **argv)
{ 
    char mode[] = "1177"; //todas as permissoes 
    char buf[100] = "/home/aluis/sopenotes";
    int i;
    i = strtol(mode, 0, 8);
    if (chmod (buf,i) < 0)
    {
        fprintf(stderr, "%s: error in chmod(%s, %s) - %d (%s)\n",
                argv[0], buf, mode, errno, strerror(errno));
        exit(1);
    }
    return(0);
}

//1777 -> -rwxrwxrwt 
//1177 -> ---xrwxrwt