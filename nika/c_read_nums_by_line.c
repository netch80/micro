#include <stdio.h>

int main() {
    int lnum = 1;
    for(;;) {
        int c = getchar();
        int value;
        printf("Character %d\n", c);
        if (c == EOF) {
            break;
        }
        if (c == '\n') {
            ++lnum;
            continue;
        }
        if (c == ' ' || c == '\t' || c == '\r') {
            continue;
        }
        ungetc(c, stdin);
        int r = scanf("%d", &value);
        if (r == 1) {
            printf("Line %d <- number %d\n", lnum, value);
        } else {
            printf("Number parsing failed, exiting\n");
            break;
        }
    }
    return 0;
}
