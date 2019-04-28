/*
 * Student name: Eliran Darshan
 * ID: 311451322
 * Group: 07
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <memory.h>
#include <ctype.h>

#define FAILURE "Error in system call\n"
#define NEW_LINE '\n'
#define END 0
#define TRUE 1
#define MATCH 1
#define ERROR -1
#define FALSE -1
#define FD 2
#define NO_MATCH 2
#define ALIKE 3
#define SIZE_FAILURE 21


/*
 * The function which runs the program, and returns the result value
 */
int run(char **argv);

/*
 * The function gets two chars* and compares between them,
 * if they are the same returns 1, and the difference between them in ascii
 * is 32 they "similar" so returns 3, else they are different and returns 2
 */
int checkIfMatch(char *fchar, char *schar);

/*
 * The function gets a const char* and checks if it's a whitespace or new line
 */
int checkIsWhitespaceOrNewLine(const char *note);

/*
 * The function gets a file descriptor , size and char* buffer
 * and reads to the buffer from the file
 */
int readToBuffer(int fd, char *buffer, int size);

int main(int argc, char **argv) {
    return run(argv);

}

int run(char **argv) {
    int val1 = 0;
    int val2 = 0;
    int flag = 1;
    int status = 0;
    int indicator = 0;
    int indicatorLike = 0;
    char *ch1 = malloc(sizeof(char));
    char *ch2 = malloc(sizeof(char));
    int fd1 = open(argv[1], O_RDONLY);
    if (fd1 < 0) {
        write(FD, FAILURE, SIZE_FAILURE);
        _exit(0);
    }

    int fd2 = open(argv[2], O_RDONLY);
    if (fd2 < 0) {
        write(FD, FAILURE, SIZE_FAILURE);
        _exit(0);
    }
    while (flag) {
        if ((val1 = readToBuffer(fd1, ch1, 1)) < 0) {
            write(FD, FAILURE, SIZE_FAILURE);
            _exit(0);
        }
        if ((val2 = readToBuffer(fd2, ch2, 1)) < 0) {
            write(FD, FAILURE, SIZE_FAILURE);
            _exit(0);
        }
        if (val1 == END && val2 == END) {
            if (indicatorLike)
                return indicatorLike;
            return indicator;
        }
        status = checkIfMatch(ch1, ch2);
        switch (status) {
            case 1:
                indicator = MATCH;
                continue;
            case 2:
                indicator = NO_MATCH;
                break;
            case 3:
                indicatorLike = ALIKE;
                break;
            default:
                fprintf(stderr, FAILURE);
        }
        if (indicator == NO_MATCH) {
            flag = 0;
        }
    }
    free(ch1);
    free(ch2);
    return indicator;
}

int checkIsWhitespaceOrNewLine(const char *note) {
    if (*note == ' ' || *note == NEW_LINE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int checkIfMatch(char *fchar, char *schar) {
    if ((*fchar - *schar) == 0) {
        return TRUE;
    } else if (((*fchar - *schar) == 32) || (*schar - *fchar) == 32) {
        return ALIKE;
    } else {
        return NO_MATCH;
    }
}

int readToBuffer(int fd, char *buffer, int size) {
    int val;
    do {
        val = read(fd, buffer, 1);
        if (val < 0) {
            return ERROR;
        }
    } while (checkIsWhitespaceOrNewLine(buffer) == TRUE && val >= 1);
    return val;
}

