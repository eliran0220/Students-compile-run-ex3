/*
 * Student name: Eliran Darshan
 * ID: 311451322
 * Group: 07
 */

#include <stdio.h>
#include <malloc.h>
#include <fcntl.h>
#include <memory.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <wait.h>
#include <stdlib.h>

#define LENGTH 150
#define RESULTS "results.csv"
#define OUTPUT "output.txt"
#define COMP_EXE "./comp.out"
#define A_OUT "a.out"
#define A_OUT_EXE "./a.out"
#define FIVE_SECONDS 5
#define ONE_SECOND 1
#define GCC "gcc"
#define NEW_LINE "\n"
#define FAILURE "Error in system call\n"
#define NO_C_FILE_TEXT ",0,N0_C_FILE"
#define COMP_ERROR_TEXT ",20,COMPILATION ERROR"
#define TIMEOUT_TEXT ",40,TIMEOUT"
#define BAD_OUTPUT_TEXT ",60,BAD OUTPUT"
#define SIMILAR_TEXT ",80,SIMILAR"
#define GREAT_JOB_TEXT ",100,GREAT JOB"
#define BACKSLASH_ZERO '\0'
#define SLASH '/'
#define ERROR -1
#define NO_C_FILE 0
#define DOT '.'
#define C_LETTER 'c'
#define NOT_EOF 1
#define GREAT_JOB 1
#define COMP_ERROR 1
#define BAD_OUTPUT 2
#define FD 2
#define SIMILAR 3
#define COMP_SUCCESS 4
#define TIMEOUT 5
#define COMPILATION_ERROR 6
#define NINE 9
#define SIZE_FAILURE 21

typedef struct {
    char generalPath[LENGTH];
    char inputPath[LENGTH];
    char correctOutput[LENGTH];

} Information;

/*
 * The function gets 2 paths for files in the parameters, uses the
 * comp program to compare between the two. returns 1 if they are identical
 * 2 if they don't match at all, and 3 if they are the "same"
 */
int compareFiles(char *correctOutPath, char *givenOutPath);

/*
 * The function gets the file descriptor , the directory name and the result
 * as parameters and writes to the result file.
 */
void writeToResult(int fd, char *dirName, int result, int *firstWrite);

/*
 * The function gets the file path and another char* and compiles it, later on
 * if the compilation worked it runs the file. if the compilation worked
 * the function returns 6, if the compilation worked but the ./a.out
 * had a timeout, returns 5, else everything succeeded returns 4
 */
int compileCFiles(char *path, char *aOut);

/*
 * The function check if the path of the file given to it is a c file
 * returns 1 if so, else returns 0
 */
int checkIsCFile(char *name);

/*
 * The function reads a line of a file into a path, used for the
 * information struct
 */
int readLine(int file, char *path);

/*
 * The function is a recursive function which searches for a c file inside the
 * folders. it gets the path as a parameter, checks at the start if it's
 * a directory, if so continues inside to search for the file, if not
 * checks if it's a c file, if so, continues to compile it.
 */
int searchCFile(char *path, char *aOut);

/*
 * The function gets the path of the file as a parameter, and checks if the
 * file is a directory
 */
int checkIsDirFile(char *name);

/*
 * The function which runs the program. gets the argv as a parameter to start
 * building the paths to the information struct.
 */
int run(char **argv);

/*
 * The function creates a path and puts to the given char*, from the argv
 */
void createPath(char **argv, char given[LENGTH]);

/*
 * The function reads the 3 lines from the configure file to the
 * information struct, using the readLine function inside it
 */
void readLinesToInformation(Information *info, char *path);

/*
 * The function copies to the givenString, the correct path from
 * the givenPath and givenName parameters, between them adds the "/" char
 */
void copyPath(char *givenString, char *givenPath, char *givenName);

/*
 * The main function, start of the program
 */
int main(int argc, char **argv) {
    return run(argv);
}

int run(char **argv) {
    Information information;
    int output;
    int firstWrite = 0;
    char save[LENGTH];
    getcwd(save, LENGTH);
    int checkFlag = 0;
    char path[LENGTH];
    char tempPath[LENGTH];
    char aOut[LENGTH];
    int value = 0;
    createPath(argv, path);
    readLinesToInformation(&information, path);
    DIR *pDir;
    struct dirent *pDirent;
    int input = open(information.inputPath, O_RDONLY);
    if (input < 0) {
        write(FD, FAILURE, SIZE_FAILURE);
    }
    int result = open(RESULTS, O_RDWR | O_TRUNC | O_CREAT,
                      S_IRUSR | S_IWUSR);
    if (result < 0) {
        write(FD, FAILURE, SIZE_FAILURE);
    }
    if (dup2(input, STDIN_FILENO) < 0) {
        write(FD, FAILURE, SIZE_FAILURE);
    }

    if ((pDir = opendir(information.generalPath)) == NULL) {
        write(FD, FAILURE, SIZE_FAILURE);
        return ERROR;
    }
    while ((pDirent = readdir(pDir)) != NULL) {
        if (strcmp(pDirent->d_name, ".") == 0 ||
            strcmp(pDirent->d_name, "..") == 0) {
            continue;
        }
        output = open(OUTPUT, O_RDWR | O_TRUNC | O_CREAT,
                      S_IRUSR | S_IWUSR);
        if (output < 0) {
            write(FD, FAILURE, SIZE_FAILURE);
        }
        copyPath(tempPath, information.generalPath, pDirent->d_name);
        if (dup2(output, STDOUT_FILENO) < 0) {
            write(FD, FAILURE, SIZE_FAILURE);
        }
        //if it's a folder, we start checking for a c file inside
        if (checkIsDirFile(tempPath)) {
            //recursively call the searchCFile and get return value
            checkFlag = searchCFile(tempPath, aOut);
            if (lseek(input, 0, SEEK_SET) < 0) {
                write(FD, FAILURE, SIZE_FAILURE);
            }
            //if compiled , compare and write to file
            if (checkFlag == COMP_SUCCESS) {
                value = compareFiles(information.correctOutput, OUTPUT);
                writeToResult(result, pDirent->d_name, value, &firstWrite);
                if (close(output) < 0) {
                    write(FD, FAILURE, SIZE_FAILURE);
                }

                // not compiled, write to file and the reason
            } else {
                writeToResult(result, pDirent->d_name, checkFlag, &firstWrite);
                if (close(output) < 0) {
                    write(FD, FAILURE, SIZE_FAILURE);
                }
                /*
                if (unlink(aOut) < 0) {
                    write(FD, FAILURE, SIZE_FAILURE);
                }
                 */

            }
        }
    }
    //closing all the remaining opened files and folders
    if (unlink(aOut) < 0) {
        write(FD, FAILURE, SIZE_FAILURE);
    }
    if (close(result) < 0) {
        write(FD, FAILURE, SIZE_FAILURE);

    }
    if (unlink(OUTPUT) < 0) {
        write(FD, FAILURE, SIZE_FAILURE);
    }
    if (closedir(pDir) < 0) {
        write(FD, FAILURE, SIZE_FAILURE);
    }
    return 0;
}

int compareFiles(char *correctOutPath, char *givenOutPath) {
    pid_t pid;
    char *argv[4] = {COMP_EXE, correctOutPath, givenOutPath, NULL};
    int status;
    if ((pid = fork()) < 0) {
        write(FD, FAILURE, SIZE_FAILURE);
    }
    if (pid == 0) {
        if (execvp(argv[0], argv) == -1) {
            write(FD, FAILURE, SIZE_FAILURE);
        }
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }

}

void writeToResult(int fd, char *dirName, int result, int *firstWrite) {
    ssize_t n;
    if (*firstWrite >= 1) {
        n = write(fd, NEW_LINE, strlen(NEW_LINE));
    }
    n = write(fd, dirName, strlen(dirName));
    switch (result) {
        case NO_C_FILE:
            n = write(fd, NO_C_FILE_TEXT, strlen(NO_C_FILE_TEXT));
            if (n < 0) {
                write(FD, FAILURE, SIZE_FAILURE);
            }
            (*firstWrite)++;
            break;
        case COMPILATION_ERROR:
            n = write(fd, COMP_ERROR_TEXT, strlen(COMP_ERROR_TEXT));
            if (n < 0) {
                write(FD, FAILURE, SIZE_FAILURE);
            }
            (*firstWrite)++;
            break;
        case TIMEOUT:
            n = write(fd, TIMEOUT_TEXT, strlen(TIMEOUT_TEXT));
            if (n < 0) {
                write(FD, FAILURE, SIZE_FAILURE);
            }
            (*firstWrite)++;
            break;
        case BAD_OUTPUT:
            n = write(fd, BAD_OUTPUT_TEXT, strlen(BAD_OUTPUT_TEXT));
            if (n < 0) {
                write(FD, FAILURE, SIZE_FAILURE);
            }
            (*firstWrite)++;
            break;
        case SIMILAR:
            n = write(fd, SIMILAR_TEXT, strlen(SIMILAR_TEXT));
            if (n < 0) {
                write(FD, FAILURE, SIZE_FAILURE);
            }
            (*firstWrite)++;
            break;
        case GREAT_JOB:
            n = write(fd, GREAT_JOB_TEXT, strlen(GREAT_JOB_TEXT));
            if (n < 0) {
                write(FD, FAILURE, SIZE_FAILURE);
            }
            (*firstWrite)++;
            break;
        default:
            break;

    }
}

int compileCFiles(char *path, char *aOut) {
    pid_t pid;
    char save[LENGTH];
    getcwd(save, LENGTH);
    char directoryPath[LENGTH];
    int status;
    strcpy(aOut, path);
    int counter = 0;
    while (path[counter] != BACKSLASH_ZERO)
        counter++;
    while (path[counter] != SLASH)
        counter--;
    counter++;
    aOut[counter] = BACKSLASH_ZERO;
    strcpy(directoryPath, aOut);
    strcat(aOut, A_OUT);
    char *argv[3] = {GCC, path, NULL};
    chdir(directoryPath);
    pid = fork();
    if (pid < 0) {
        write(FD, FAILURE, SIZE_FAILURE);
    }
    //we are in the son, compile the program
    if (pid == 0) {
        if (execvp(argv[0], argv) == -1) {
            write(FD, FAILURE, SIZE_FAILURE);
            chdir(save);
            return COMPILATION_ERROR;
        }

    } else if (pid > 0) {
        //we are in the fater, wait for status
        waitpid(pid, &status, 0);
        if (WEXITSTATUS(status) == COMP_ERROR) {
            chdir(save);
            return COMPILATION_ERROR;
        }
    }
    char *args[2] = {A_OUT_EXE, NULL};

    if ((pid = fork()) < 0) {
        write(FD, FAILURE, SIZE_FAILURE);
    }
    // we are in the son, run the file
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            write(FD, FAILURE, SIZE_FAILURE);
        }

    } else if (pid > 0) {
        // we are in the file, checking if there was a timeout
        int indicator = 0, second = 0;
        while (indicator == 0) {
            second += 1;
            sleep(ONE_SECOND);
            if (second > FIVE_SECONDS) {
                break;
            }
            indicator = waitpid(pid, &status, WNOHANG);
        }
        if (second > FIVE_SECONDS) {
            chdir(save);
            kill(pid, NINE);
            wait(NULL);
            return TIMEOUT;
        }

    }
    chdir(save);
    return COMP_SUCCESS;

}

int searchCFile(char *path, char *aOut) {
    if (!checkIsDirFile(path)) {
        if (checkIsCFile(path)) {
            return compileCFiles(path, aOut);
        }
        return NO_C_FILE;
    }

    int status = NO_C_FILE;
    struct dirent *pDirent;
    char tempPath[LENGTH];
    DIR *folder;
    if ((folder = opendir(path)) == NULL) {
        write(FD, FAILURE, SIZE_FAILURE);
        return ERROR;
    }
    // while the c file wasn't found and the folder isn't null
    while ((pDirent = readdir(folder)) != NULL && status == NO_C_FILE) {
        if (strcmp(pDirent->d_name, ".") == 0 ||
            strcmp(pDirent->d_name, "..") == 0)
            continue;
        copyPath(tempPath, path, pDirent->d_name);
        status = searchCFile(tempPath, aOut);
        if (status == ERROR) {
            if (closedir(folder) < 0) {
                write(FD, FAILURE, SIZE_FAILURE);
            }
            return ERROR;
        }

    }

    if (closedir(folder) < 0) {
        write(FD, FAILURE, SIZE_FAILURE);
        exit(1);
    }
    return status;
}

void copyPath(char *givenString, char *givenPath, char *givenName) {
    strcpy(givenString, givenPath);
    strcat(givenString, "/");
    strcat(givenString, givenName);
}


void readLinesToInformation(Information *info, char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        write(FD, FAILURE, SIZE_FAILURE);
        exit(1);
    }
    readLine(fd, info->generalPath);
    readLine(fd, info->inputPath);
    readLine(fd, info->correctOutput);
}

int readLine(int file, char *path) {
    char buffer[1];
    int i = 0;
    while (read(file, buffer, 1) == 1) {
        if (buffer[0] == '\n') {
            path[i] = '\0';
            return NOT_EOF;
        }
        path[i] = buffer[0];
        i++;
    }
    return EOF;
}

void createPath(char **argv, char given[LENGTH]) {
    strcpy(given, argv[1]);
}

int checkIsCFile(char *name) {
    size_t length = strlen(name);
    char ending[2];
    int index = 0;
    size_t i;
    //copying the two last letters
    for (i = length - 2; i < length; i++) {
        ending[index] = name[i];
        index++;
    }
    // if indeed a c file, the ending must match
    if (ending[0] == DOT && ending[1] == C_LETTER) {
        return 1;
    } else {
        return 0;
    }

}

int checkIsDirFile(char *name) {
    struct stat stat_p;
    stat(name, &stat_p);
    return S_ISDIR(stat_p.st_mode);
}
