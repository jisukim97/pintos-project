#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "lib/user/syscall.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include <list.h>

/* Process Control Block. */
struct process
{
    const char * file_name;       /* Filename to execute. */
    pid_t pid;
    struct thread *parent;        /* Parent Process. */
    struct list_elem child_elem; /* List elements for children. */
    bool is_loaded;               /* Flag = -1 if process creation is failed. */
    bool is_exited;               /* True when the proces is terminated. */
    struct semaphore sema_exit;  /* exit semaphore. */
    struct semaphore sema_load;  /* load semaphore. */
    int exit_status;              /* termination status when exit() called. */
};

tid_t process_execute(const char *);
int process_wait(tid_t);
void process_exit(void);
void process_activate(void);

struct process *process_get_child(pid_t);
void process_remove_child(struct process *);

#endif /* userprog/process.h */
