#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_SIZE 5000

int isPalindrome(char *str, int start, int end) {
    while (start < end) {
        if (str[start] != str[end])
            return 0;
        start++;
        end--;
    }
    return 1;
}

void child1(char *input_file, int pipe1[]) {
    FILE *fp = fopen(input_file, "r");
    if (fp == NULL) {
        printf("Error: could not open input file.\n");
        exit(1);
    }
    char str[MAX_SIZE];
    fgets(str, MAX_SIZE, fp);
    close(pipe1[0]);
    write(pipe1[1], str, strlen(str) + 1);
    close(pipe1[1]);
    fclose(fp);
}

void child2(int pipe1[], int pipe2[]) {
    close(pipe1[1]);
    char str[MAX_SIZE];
    read(pipe1[0], str, MAX_SIZE);
    close(pipe1[0]);
    int len = strlen(str);
    char res[MAX_SIZE];
    int res_idx = 0;
    for (int i = 0; i < len; i++) {
        if (isalpha(str[i])) {
            int j = i + 1;
            while (j < len && isalpha(str[j]))
                j++;
            for (int k = i; k < j; k++) {
                for (int l = k + 1; l <= j; l++) {
                    if (isPalindrome(str, k, l - 1)) {
                        for (int m = k; m < l; m++) {
                            res[res_idx++] = str[m];
                        }
                        res[res_idx++] = ' ';
                    }
                }
            }
            i = j - 1;
        }
    }
    res[res_idx] = '\0';
    close(pipe2[0]);
    write(pipe2[1], res, strlen(res) + 1);
    close(pipe2[1]);
}

void child3(char *output_file, int pipe2[]) {
    FILE *fp = fopen(output_file, "w");
    if (fp == NULL) {
        printf("Error: could not open output file.\n");
        exit(1);
    }
    char res[MAX_SIZE];
    char c;
    int i = 0;
    close(pipe2[1]);
    while (read(pipe2[0], &c, 1) > 0) {
        if (c != ' ') {
            res[i++] = c;
        } else {
            res[i++] = ' ';
        }
    }
    close(pipe2[1]);
    fprintf(fp, "%s\n", res);
    close(pipe2[0]);
    fclose(fp);
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s input_file output_file\n", argv[0]);
        return 1;
    }
    int pipe1[2];
    int pipe2[2];
    pid_t pid1, pid2, pid3;
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        printf("Error: could not create pipes.\n");
        return 1;
    }
    pid1 = fork();
    if (pid1 == 0) {
        child1(argv[1], pipe1);
        exit(0);
    }
    pid2 = fork();
    if (pid2 == 0) {
        child2(pipe1, pipe2);
        exit(0);
    }
    pid3 = fork();
    if (pid3 == 0) {
        child3(argv[2], pipe2);
        exit(0);
    }
    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
    waitpid(pid3, NULL, 0);
    return 0;
}
