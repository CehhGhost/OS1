#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZE 5000

int main() {
    char str[BUFSIZE];
    char newStr[BUFSIZE];
    int len, i, j;

    fgets(str, BUFSIZE, stdin);

    len = strlen(str);

    j = 0;
    for (i = 0; i < len; i++) {
        if (isupper(str[i])) {
            int k = i;
            while (isalpha(str[k])) {
                newStr[j] = str[k];
                j++;
                k++;
            }
            newStr[j] = ' ';
            j++;
            i = k;
        }
    }

    newStr[j] = '\0';

    printf("%s", newStr);
    return 0;
}
