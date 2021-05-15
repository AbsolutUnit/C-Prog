/*
 * Imprimer: Command-line interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "imprimer.h"
#include "conversions.h"
#include "sf_readline.h"

typedef struct {
    int ID;
    char* name;
    char* type;
    PRINTER_STATUS status;
}  printer;

typedef struct {
    char* type;
    char* creation;
    char* status_update;
    JOB_STATUS status;
    unsigned int eligible;
    char* file;
    int jobID;
} job;

printer PRINTER_LIST[MAX_PRINTERS];
int print_list_index = 0;
job JOB_LIST[MAX_JOBS];
int job_list_index = 0;

int find_print(int dum, FILE *out){
    job holder = JOB_LIST[dum];
//    printer dumbo;
    int index = -1;
    for (int i = 0; i < MAX_PRINTERS; i++){
        if (PRINTER_LIST[i].name == NULL && i == MAX_PRINTERS - 1){
            fprintf(out, "No printers available!\n");
            return -1;
        } else if (PRINTER_LIST[i].status == PRINTER_IDLE){
            if (find_conversion_path(holder.type, PRINTER_LIST[i].type)[0] != NULL){
                if (holder.eligible & (0b1 << PRINTER_LIST[i].ID)) {
                    index = i;
                    break;
                }
            } else if (strcmp(holder.type, PRINTER_LIST[i].type) == 0){
                if (holder.eligible & (0b1 << PRINTER_LIST[i].ID)) {
                    index = i;
                    break;
                }
            } else {
                continue;
            }
        }
    }
    return index;
}

int conv_len(int a, int b){
    job holder = JOB_LIST[a];
    printer dumbo = PRINTER_LIST[b];
    CONVERSION **path = find_conversion_path(holder.type, dumbo.type);
    int len = 0;
    CONVERSION *johnny = path[0];
    while (johnny != NULL){
        len++;
        johnny = path[len];
    }
    return len;
}

int seek_p(int dum, int dum2, int len, FILE *out){
    job holder = JOB_LIST[dum];
    printer dumbo = PRINTER_LIST[dum2];
    CONVERSION **path = find_conversion_path(holder.type, dumbo.type);
    int pipes[len][2];
    int print_disc = imp_connect_to_printer(dumbo.name, dumbo.type, PRINTER_NORMAL);
    int f = open(holder.file, O_RDONLY);
    for (int i = 0; i < len; i++) {
        pid_t chil2;
        pipe(pipes[i]);
        if ((chil2 = fork()) < 0) {
            fprintf(out, "Fork error.\n");
            sf_cmd_error("conversion");
            return -1;
        } else if (chil2 == 0) {
            if (i == 0) {
                dup2(f, 0);
                close(f);
                if (len == 1){
                    dup2(print_disc, 1);
                    close(print_disc);
                    execvp(path[i]->cmd_and_args[0], path[i]->cmd_and_args);
                } else {
                    dup2(pipes[i][1], 1);
                    close(pipes[i][1]);
                    execvp(path[i]->cmd_and_args[0], path[i]->cmd_and_args);
                }
            } else if (i == len - 1) {
                dup2(pipes[i - 1][1], 0);
                close(pipes[i - 1][1]);
                dup2(print_disc, 1);
                close(print_disc);
                execvp(path[i]->cmd_and_args[0], path[i]->cmd_and_args);
            } else {
                dup2(pipes[i - 1][1], 0);
                close(pipes[i - 1][1]);
                dup2(pipes[i][1], 1);
                close(pipes[i][1]);
                execvp(path[i]->cmd_and_args[0], path[i]->cmd_and_args);
            }
        } else {
            continue;
        }
    }
    return 0;
}

int single_fork(int a, int b, int len, FILE *out){
    job holder = JOB_LIST[a];
    printer dumbo = PRINTER_LIST[b];
    pid_t child;
    if ((child = fork()) < 0){
        fprintf(out, "Fork error.\n");
        sf_cmd_error("conversion");
        return -1;
    } else if (child == 0){
        time_t currtime;
        dumbo.status = PRINTER_BUSY;
        holder.status = JOB_RUNNING;
        holder.status_update = ctime(&currtime);
        int print_disc = imp_connect_to_printer(dumbo.name, dumbo.type, PRINTER_NORMAL);
        int f = open(holder.file, O_RDONLY);
        dup2(f, 0);
        close(f);
        dup2(print_disc, 1);
        close(print_disc);
        char *argv[] = {"/bin/cat", NULL};
        execvp("/bin/cat", argv);
    } else {
        int *abortion = NULL;
        waitpid(child, abortion, 0);
        time_t currtime;
        holder.status_update = ctime(&currtime);
        holder.status = JOB_FINISHED;
        return 0;
    }
    return 0;
}

int name_def(char* name){
    for (int i = 0; i < MAX_PRINTERS; i++){
        if (PRINTER_LIST[i].name == NULL){
            break;
        } else if (strcmp(PRINTER_LIST[i].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

int count_affirm(char* come, FILE *out){
    char *delimi = " ";
    int corrno = 0;
    int errno = 0;
    int reqno = 0;
    char *com = strtok(come, delimi);
    if (strcmp(com, "help") == 0) {
        corrno = 0;
        errno = 0;
        reqno = 0;
    } else if (strcmp(com, "quit") == 0) {
        corrno = 0;
        errno = 0;
        reqno = 0;
    } else if (strcmp(com, "type") == 0) {
        corrno = 1;
        errno = 0;
        reqno = 1;
    } else if (strcmp(com, "printer") == 0) {
        corrno = 2;
        errno = 0;
        reqno = 2;
    } else if (strcmp(com, "conversion") == 0) {
        corrno = 50;
        errno = 0;
        reqno = 3;
    } else if (strcmp(com, "printers") == 0) {
        corrno = 0;
        errno = 0;
        reqno = 0;
    } else if (strcmp(com, "jobs") == 0) {
        corrno = 0;
        errno = 0;
        reqno = 0;
    } else if (strcmp(com, "print") == 0) {
        corrno = 33;
        errno = 0;
        reqno = 1;
    } else if (strcmp(com, "cancel") == 0) {
        corrno = 1;
        errno = 0;
        reqno = 1;
    } else if (strcmp(com, "pause") == 0) {
        corrno = 1;
        errno = 0;
        reqno = 1;
    } else if (strcmp(com, "resume") == 0) {
        corrno = 1;
        errno = 0;
        reqno = 1;
    } else if (strcmp(com, "disable") == 0) {
        corrno = 1;
        errno = 0;
        reqno = 1;
    } else if (strcmp(com, "enable") == 0) {
        corrno = 1;
        errno = 0;
        reqno = 1;
    }
    char *cockazoid = strtok(NULL, delimi);
    while (cockazoid != NULL && strcmp(cockazoid, "") != 0){
        errno++;
        cockazoid = strtok(NULL, delimi);
    }
    if (errno > corrno) {
        fprintf(out, "Wrong number of args (given: %d, required %d) for CLI command '%s'\n", errno, corrno, com);
        sf_cmd_error("arg count");
        return 0;
    } else if (errno < reqno){
        fprintf(out, "Wrong number of args (given: %d, required %d) for CLI command '%s'\n", errno, reqno, com);
        sf_cmd_error("arg count");
        return 0;
    } else if (errno == 0){
        return 1;
    } else {
        return errno;
    }
}

int run_cli(FILE *in, FILE *out)
{
    char *delim = " ";
    if (in == NULL){
        return -1;
    }
    while(1) {
        char *val;
        if (in != stdin || out != stdout){
            size_t *l = 0;
            if (getline(&val, l, in) == -1){
                return -1;
            }
            if (val[(int)(*l) - 1] == '\n'){
                val[(int)(*l) - 1] = '\0';
            }
            fprintf(stdout, "Here's val %s", val);
        } else {
            val = sf_readline("imp> ");
        }
        if (val == NULL){
            break;
        }
//        fprintf(out, "COCK %s.\n", val);
        char *temp = (char *)(malloc(strlen(val) + 1));
        char *temp2 = (char *)(malloc(strlen(val) + 1));
        strcpy(temp, val);
        strcpy(temp2, val);
        char *command = strtok(val, delim);
        int count = count_affirm(temp, out);
        fprintf(stdout, "Here's the token %s", command);
        if (strcmp(command, "help") == 0) {
            if (count) {
                fprintf(out,
                        "Commands are: help quit type printer conversion printers job print cancel disable enable pause resume \n");
                sf_cmd_ok();
                continue;
            } else {
                continue;
            }
        } else if (strcmp(command, "quit") == 0) {
            if (count) {
                sf_cmd_ok();
                free(temp);
                free(temp2);
                free(val);
//                sleep(10);
                if (out != stdout){
                    fflush(out);
                    fclose(out);
                }
                return -1;
            } else {
                continue;
            }
        } else if (strcmp(command, "type") == 0) {
            if (count) {
                strtok(temp2, delim);
                define_type(strtok(NULL, delim));
                sf_cmd_ok();
                continue;
            } else {
                continue;
            }
        } else if (strcmp(command, "printer") == 0) {
            if (count) {
                strtok(temp2, delim);
                char *print_name = strtok(NULL, delim);
                char *type = strtok(NULL, delim);
                if (find_type(type) != NULL){
                    if (name_def(print_name)){
                        fprintf(out, "Printer name '%s' not unique.\n", print_name);
                        sf_cmd_error("printer");
                        continue;
                    } else {
                        sf_printer_defined(print_name, type);
                        sf_printer_status(print_name, PRINTER_DISABLED);
                        printer NEWPRINT = {print_list_index++, print_name, type, PRINTER_DISABLED};
                        PRINTER_LIST[print_list_index - 1] = NEWPRINT;
                        sf_cmd_ok();
                        continue;
                    }
                } else {
                    fprintf(out, "Unknown file type: %s\n", type);
                    sf_cmd_error("printer");
                    continue;
                }
            } else {
                continue;
            }
        } else if (strcmp(command, "conversion") == 0) {
            if (count) {
                strtok(temp2, delim);
                char *from = strtok(NULL, delim);
                char *to = strtok(NULL, delim);
                char *next = strtok(NULL, delim);
                int max = count - 2;
                char *arr[max + 1];
                int iter = 0;
                while (max != iter){
                    arr[iter] = next;
                    next = strtok(NULL, delim);
                    if (next == NULL){
                        break;
                    }
                    iter++;
                }
                arr[iter + 1] = NULL;
                define_conversion(from, to, arr);
                sf_cmd_ok();
                continue;
            } else {
                continue;
            }
        } else if (strcmp(command, "printers") == 0) {
            if (count) {
                strtok(temp2, delim);
                for (int i = 0; i < MAX_PRINTERS; i++){
                    if (PRINTER_LIST[i].name != NULL){
                        sf_printer_status(PRINTER_LIST[i].name, PRINTER_LIST[i].status);
                        char* stat = "";
                        if (PRINTER_LIST[i].status == PRINTER_DISABLED){
                            stat = "Disabled";
                        } else if (PRINTER_LIST[i].status == PRINTER_BUSY){
                            stat = "Busy";
                        } else if (PRINTER_LIST[i].status == PRINTER_IDLE){
                            stat = "Idle";
                        }
                        fprintf(out, "PRINTER: id = %d, name = %s, type = %s, status = %s\n", PRINTER_LIST[i].ID,
                                PRINTER_LIST[i].name, PRINTER_LIST[i].type, stat);
                    }
                }
                sf_cmd_ok();
                continue;
            } else {
                continue;
            }
        } else if (strcmp(command, "jobs") == 0) {
            if (count) {
                strtok(temp2, delim);
                for (int i = 0; i < MAX_PRINTERS; i++){
                    if (JOB_LIST[i].type != NULL){
                        sf_job_status(JOB_LIST[i].jobID, JOB_LIST[i].status);
                        char* stat = "";
                        if (JOB_LIST[i].status == JOB_CREATED){
                            stat = "Created";
                        } else if (JOB_LIST[i].status == JOB_RUNNING){
                            stat = "Running";
                        } else if (JOB_LIST[i].status == JOB_PAUSED){
                            stat = "Paused";
                        } else if (JOB_LIST[i].status == JOB_FINISHED){
                            stat = "Finished";
                        } else if (JOB_LIST[i].status == JOB_ABORTED){
                            stat = "Aborted";
                        } else if (JOB_LIST[i].status == JOB_DELETED){
                            stat = "Deleted";
                        }
                        fprintf(out, "Job[%d]: type = %s, creation(%s), status(%s) = %s, eligible = %u, file = %s\n", i,
                            JOB_LIST[i].type, JOB_LIST[i].creation, JOB_LIST[i].status_update, stat, JOB_LIST[i].eligible, JOB_LIST[i].file);
                    }
                }
                sf_cmd_ok();
                continue;
            } else {
                continue;
            }
        } else if (strcmp(command, "print") == 0) {
            if (count) {
                strtok(temp2, delim);
                char *file_name = strtok(NULL, delim);
                FILE_TYPE *my_file = infer_file_type(file_name);
                if (my_file == NULL || find_type(my_file->name) == NULL){
                    sf_cmd_error("printer");
                    fprintf(out, "File type not declared or could not be inferred.\n");
                    sf_cmd_error("print (file type)");
                } else {
                    time_t curtime;
                    if (count > 1) {
                        sf_job_created(job_list_index++, file_name, my_file->name);
                        int elig = 0b0;
                        for (int i = 0; i < count - 2; i++){
                            int dummy = 0b1;
                            char *new_print = strtok(NULL, delim);
                            int id = -1;
                            for (int j = 0; j < MAX_PRINTERS; j++){
                                if (PRINTER_LIST[j].name == new_print){
                                    id = PRINTER_LIST[j].ID;
                                    break;
                                }
                            }
                            if (id == -1){
                                goto invalid;
                            }
                            elig += (dummy << id);
                        }
                        job NEWJOB = {my_file->name, (char *)(&curtime), (char *)(&curtime), JOB_CREATED, elig,
                                      file_name, job_list_index};
                        JOB_LIST[job_list_index - 1] = NEWJOB;
                        int pr_ind = find_print(job_list_index - 1, out);
                        if (pr_ind == -1){
                            continue;
                        } else {
                            int len = conv_len(job_list_index - 1, pr_ind);
                            pid_t child;
                            if ((child = fork()) < 0){
                                fprintf(out, "Fork error.\n");
                                sf_cmd_error("conversion");
                                return -1;
                            } else if (child == 0) {
                                if (len == 0){
                                    single_fork(job_list_index - 1, pr_ind, len, out);
                                } else {
                                    seek_p(job_list_index - 1, pr_ind, len, out);
                                }
                                _exit(0);
                            } else {
                                sf_cmd_ok();
                                continue;
                            }
                        }
                    } else {
                        sf_job_created(job_list_index++, file_name, my_file->name);
                        job NEWJOB = {my_file->name, (char *)(&curtime), (char *)(&curtime), JOB_CREATED, 0xFFFFFFFF,
                                      file_name, job_list_index};
                        JOB_LIST[job_list_index - 1] = NEWJOB;
                        int pr_ind = find_print(job_list_index - 1, out);
                        if (pr_ind == -1){
                            continue;
                        } else {
                            int len = conv_len(job_list_index - 1, pr_ind);
                            pid_t child;
                            if ((child = fork()) < 0){
                                fprintf(out, "Fork error.\n");
                                sf_cmd_error("conversion");
                                return -1;
                            } else if (child == 0) {
                                if (len == 0){
                                    single_fork(job_list_index - 1, pr_ind, len, out);
                                } else {
                                    seek_p(job_list_index - 1, pr_ind, len, out);
                                }
                                _exit(0);
                            } else {
                                sf_cmd_ok();
                                continue;
                            }
                        }
                    }
                }
            } else {
                invalid: ;
                continue;
            }
        } else if (strcmp(command, "cancel") == 0) {
            if (count) {
                strtok(temp2, delim);
                int job_id = atoi(strtok(NULL, delim));
                if (JOB_LIST[job_id].status != JOB_RUNNING){
                    fprintf(out, "Job not running.\n");
                    sf_cmd_error("job");
                    continue;
                } else {
                    JOB_LIST[job_id].status = JOB_ABORTED;
                    sf_cmd_ok();
                    continue;
                }
            } else {
                continue;
            }
        } else if (strcmp(command, "pause") == 0) {
            if (count) {
                strtok(temp2, delim);
                int job_id = atoi(strtok(NULL, delim));
                if (JOB_LIST[job_id].status != JOB_RUNNING){
                    fprintf(out, "Job not running.\n");
                    sf_cmd_error("job");
                    continue;
                } else {
                    JOB_LIST[job_id].status = JOB_PAUSED;
                    sf_cmd_ok();
                    continue;
                }
            } else {
                continue;
            }
        } else if (strcmp(command, "resume") == 0) {
            if (count) {
                strtok(temp2, delim);
                int job_id = atoi(strtok(NULL, delim));
                if (JOB_LIST[job_id].status != JOB_PAUSED){
                    fprintf(out, "Job not paused.\n");
                    sf_cmd_error("job");
                    continue;
                } else {
                    JOB_LIST[job_id].status = JOB_RUNNING;
                    sf_cmd_ok();
                    continue;
                }
            } else {
                continue;
            }
        } else if (strcmp(command, "disable") == 0) {
            if (count) {
                strtok(temp2, delim);
                char *print_name = strtok(NULL, delim);
                for (int i = 0; i < MAX_PRINTERS; i++){
                    if (PRINTER_LIST[i].name == NULL && i == MAX_PRINTERS - 1){
                        fprintf(out, "Printer with name '%s' does not exist.\n", print_name);
                        sf_cmd_error("printer");
                    } else if (strcmp(PRINTER_LIST[i].name, print_name) == 0){
                        PRINTER_LIST[i].status = PRINTER_DISABLED;
                        sf_printer_status(PRINTER_LIST[i].name, PRINTER_LIST[i].status);
                        fprintf(out, "Printer '%s' disabled.\n", PRINTER_LIST[i].name);
                        sf_cmd_ok();
                        break;
                    }
                }
                continue;
            } else {
                continue;
            }
        } else if (strcmp(command, "enable") == 0) {
            if (count) {
                strtok(temp2, delim);
                char *print_name = strtok(NULL, delim);
                int print_ind = -1;
                for (int i = 0; i < MAX_PRINTERS; i++){
                    if (PRINTER_LIST[i].name == NULL && i == MAX_PRINTERS - 1){
                        fprintf(out, "Printer with name '%s' does not exist.\n", print_name);
                        sf_cmd_error("printer");
                    } else if (strcmp(PRINTER_LIST[i].name, print_name) == 0){
                        print_ind = i;
                        PRINTER_LIST[i].status = PRINTER_IDLE;
                        sf_printer_status(PRINTER_LIST[i].name, PRINTER_LIST[i].status);
                        fprintf(out, "Printer '%s' enabled.\n", PRINTER_LIST[i].name);
                        sf_cmd_ok();
                        break;
                    }
                }
                int job_ind = -1;
                for (int i = 0; i < MAX_JOBS; i++) {
                    if (JOB_LIST[i].type != NULL) {
                        if (find_conversion_path(JOB_LIST[i].type, PRINTER_LIST[print_ind].type)[0] != NULL) {
                            if (JOB_LIST[i].status == JOB_CREATED) {
                                if (JOB_LIST[i].eligible & (0b1 << PRINTER_LIST[print_ind].ID)) {
                                    job_ind = i;
                                    break;
                                }
                            }
                        }
                    }
                }
                int len = conv_len(job_ind, print_ind);
                pid_t child;
                if ((child = fork()) < 0){
                    fprintf(out, "Fork error.\n");
                    sf_cmd_error("conversion");
                    return -1;
                } else if (child == 0) {
                    if (len == 0){
                        single_fork(job_ind, print_ind, len, out);
                    } else {
                        seek_p(job_ind, print_ind, len, out);
                    }
                    _exit(0);
                } else {
                    sf_cmd_ok();
                    continue;
                }
            } else {
                continue;
            }
        } else {
            fprintf(out, "Unrecognized command: %s \n", command);
            sf_cmd_error("unrecognized command");
        }
    }
    return 0;
}
