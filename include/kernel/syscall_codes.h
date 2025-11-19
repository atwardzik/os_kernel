//
// Created by Artur Twardzik on 05/01/2025.
//

#ifndef SYSCALL_CODES_H
#define SYSCALL_CODES_H

#define RESTART_SVC     0
#define EXIT_SVC        1
#define SPAWN_SVC       2
#define SPAWNP_SVC      3
#define READ_SVC        4
#define WRITE_SVC       5
#define OPEN_SVC        6
#define CLOSE_SVC       7
#define WAIT_SVC        8
#define READDIR_SVC     9
#define CHDIR_SVC       10
#define LSEEK_SVC       11
#define FSTAT_SVC       12
#define GETCWD_SVC      13

#define GET_TIME_SVC    20
#define GET_PID_SVC     21
#define GET_PPID_SVC    22
#define YIELD_SVC       33
#define KILL_SVC        34
#define SIGNAL_SVC      35
#define SIGRETURN_SVC   36

#define OS_INIT_SVC     255

#endif //SYSCALL_CODES_H
