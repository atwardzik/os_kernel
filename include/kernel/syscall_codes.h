//
// Created by Artur Twardzik on 05/01/2025.
//

#ifndef SYSCALL_CODES_H
#define SYSCALL_CODES_H

#define RESTART_SVC     0
#define EXIT_SVC        1
#define SPAWN_SVC       2
#define READ_SVC        3
#define WRITE_SVC       4
#define OPEN_SVC        5
#define CLOSE_SVC       6
#define WAIT_SVC        7
#define READDIR_SVC     8
#define CHDIR_SVC       9
#define GET_TIME_SVC    10
#define GET_PID_SVC     11
#define GET_PPID_SVC    12
#define YIELD_SVC       13
#define KILL_SVC        14
#define SIGNAL_SVC      15
#define SIGRETURN_SVC   16
#define LSEEK_SVC       17

#define OS_INIT_SVC     255

#endif //SYSCALL_CODES_H
