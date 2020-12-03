#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
int check_validity_useraddr(const void *);

void sys_exit(int);
int sys_fibonacci(int);
int sys_max_of_four(int, int, int, int);

struct lock *get_filesys_lock(void);

#endif /* userprog/syscall.h */
