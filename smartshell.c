#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h> 

char *lsh_read_line(void);
char **lsh_split_line(char *line);
int lsh_execute(char **args);
void lsh_loop(void);

int main(int argc, char**argv) {
    lsh_loop(); // main function
    return EXIT_SUCCESS;
}

void lsh_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);

        free(line);
        free(args);
    } while (status);
}

/*
    lsh_read_line function basically reads the current args within the shell itself and processes that args for errors wtihtin the liine.
*/

#define LSH_RL_BUFSIZE 1024

char *lsh_read_line(void) {
    char *line = NULL;
    ssize_t bufsize = 0;

    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS); // Exit on EOF
        } else {
            perror("lsh: getline error");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

/*
    lsh_split_line function splits the input line into tokens based on delimiters.
    It dynamically allocates memory for the tokens and handles reallocation if the buffer size is exceeded.
*/

#define LSH_TOK_DELIM " \t\r\n\a"
#define LSH_TOK_BUFSIZE 64

char **lsh_split_line(char *line) {
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0; // Might cause issues with buffer allocation
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += LSH_RL_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

    /*
        lsh_launch launches the shell 
    */

int lsh_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("lsh");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1; 
}

/* 
    Built-in commands for the shell
    These functions handle built-in commands like cd, help, and exit.
*/

int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_mkdir(char **args);
int lsh_ls(char **args);
int lsh_pwd(char **args);
int lsh_touch(char **args);
int lsh_rmdir(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "mkdir",
    "ls",
    "pwd",
    "touch",
    "rmdir"
};

int (*builtin_func[]) (char **) = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit,
    &lsh_mkdir,
    &lsh_ls,
    &lsh_pwd,
    &lsh_touch,
    &lsh_rmdir
};

int lsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int lsh_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: cd: missing argument\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help(char **args) {
    int i;
    printf("SmartShell\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built-in commands:\n");

    for (i = 0; i < lsh_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int lsh_mkdir(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: mkdir: missing argument\n");
    } else {
        if (mkdir(args[1], 0755) != 0) { // we are making a directory with 0755 permissions so that the user can read, write, and execute within this new directory
            perror("lsh");
        }
    }
    return 1;
}

int lsh_ls(char **args) {
    if (args[1] != NULL) {
        fprintf(stderr, "lsh: ls: no arguments expected\n");
        return 1;
    }
    // Use the system command to list files in the current directory
    if (system("ls") == -1) {
        perror("lsh");
    }
    return 1;
}

int lsh_pwd(char **args) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("lsh");
    }
    return 1;
}

int lsh_touch(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: touch: missing argument\n");
    } else {
        // Create an empty file or update the timestamp of the file
        FILE *file = fopen(args[1], "a");
        if (file == NULL) {
            perror("lsh");
        } else {
            fclose(file);
        }
    }
    return 1;
}

int lsh_rmdir(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: rmdir: missing argument\n");
    } else {
        // Remove the directory
        if (rmdir(args[1]) != 0) {
            perror("lsh");
        }
    }
    return 1;
}

int lsh_exit(char **args) {
    return 0;
}

int lsh_execute(char **args) {
    int i;

    if (args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < lsh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i]) (args);
        }
    }
    return lsh_launch(args);
}

