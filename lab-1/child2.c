#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define BUFFER_SIZE 256

void remove_vowels(char *str) {
    char *dst = str;
    while (*str) {
        if (!strchr("aeiouAEIOU", *str)) {
            *dst++ = *str;
        }
        str++;
    }
    *dst = '\0';
}

int main() {
    char buffer[BUFFER_SIZE];

    while (read(STDIN_FILENO, buffer, BUFFER_SIZE) > 0) {
        buffer[strcspn(buffer, "\0")] = '\0';
        remove_vowels(buffer);
        printf("%s\n", buffer);
        fflush(stdout);
    }

    return 0;
}
