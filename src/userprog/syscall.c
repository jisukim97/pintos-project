#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/process.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "threads/vaddr.h"

#define max(a, b) ((a) > (b)) ? (a) : (b)

static void syscall_handler(struct intr_frame *);

static void sys_halt();
static pid_t sys_exec(const char *);
static int sys_wait(pid_t);
static int sys_read(int, void *, unsigned);
static int sys_write(int, void *, unsigned);

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler(struct intr_frame *f)
{
  void *esp = f->esp;
  if (!check_validity_useraddr(esp) || !check_validity_useraddr(esp + sizeof(uintptr_t) - 1))
    sys_exit(-1);
  int sys_num = *(int *)esp;

  switch (sys_num)
  {
  case SYS_HALT:
  {
    sys_halt();
  }
  case SYS_EXIT:
  {
    if (!check_validity_useraddr(esp + sizeof(uintptr_t)) || !check_validity_useraddr(esp + 2 * sizeof(uintptr_t) - 1))
      sys_exit(-1);
    int status = *(int *)(esp + sizeof(uintptr_t));
    sys_exit(status);
  }
  case SYS_EXEC:
  {
    if (!check_validity_useraddr(esp + sizeof(uintptr_t)) || !check_validity_useraddr(esp + 2 * sizeof(uintptr_t) - 1))
      sys_exit(-1);
    char *cmd_line = *(char **)(esp + sizeof(uintptr_t));
    f->eax = (uint32_t)sys_exec(cmd_line);
    break;
  }
  case SYS_WAIT:
  {
    if (!check_validity_useraddr(esp + sizeof(uintptr_t)) || !check_validity_useraddr(esp + 2 * sizeof(uintptr_t) - 1))
      sys_exit(-1);
    pid_t pid = *(pid_t *)(esp + sizeof(uintptr_t));
    f->eax = (uint32_t)sys_wait(pid);
    break;
  }
  case SYS_READ:
  {
    if (!check_validity_useraddr(esp + sizeof(uintptr_t)) || !check_validity_useraddr(esp + 4 * sizeof(uintptr_t) - 1))
      sys_exit(-1);
    int fd = *(int *)(esp + sizeof(uintptr_t));
    char *buffer = *(char **)(esp + 2 * sizeof(uintptr_t));
    unsigned size = *(unsigned *)(esp + 3 * sizeof(uintptr_t));
    f->eax = (uint32_t)sys_read(fd, buffer, size);
    break;
  }
  case SYS_WRITE:
  {
    if (!check_validity_useraddr(esp + sizeof(uintptr_t)) || !check_validity_useraddr(esp + 4 * sizeof(uintptr_t) - 1))
      sys_exit(-1);
    int fd = *(int *)(esp + sizeof(uintptr_t));
    char *buffer = *(char **)(esp + 2 * sizeof(uintptr_t));
    unsigned size = *(unsigned *)(esp + 3 * sizeof(uintptr_t));
    f->eax = (uint32_t)sys_write(fd, buffer, size);
    break;
  }
  case SYS_FIBONACCI:
  {
    if (!check_validity_useraddr(esp) || !check_validity_useraddr(esp + 2 * sizeof(uintptr_t) - 1))
      sys_exit(-1);
    int n = *(int *)(esp + sizeof(uintptr_t));
    f->eax = (uint32_t)sys_fibonacci(n);
    break;
  }
  case SYS_MAXFOUR:
  {
    if (!check_validity_useraddr(esp + sizeof(uintptr_t)) || !check_validity_useraddr(esp + 5 * sizeof(uintptr_t) - 1))
      sys_exit(-1);
    int a = *(int *)(esp + sizeof(uintptr_t));
    int b = *(int *)(esp + 2 * sizeof(uintptr_t));
    int c = *(int *)(esp + 3 * sizeof(uintptr_t));
    int d = *(int *)(esp + 4 * sizeof(uintptr_t));
    f->eax = (uint32_t)sys_max_of_four(a, b, c, d);
    break;
  }

  default:
    sys_exit(-1);
  }
}

int check_validity_useraddr(const void *uaddr)
{
  struct thread *cur_thread = thread_current();
  if (uaddr == NULL || is_kernel_vaddr(uaddr) || !pagedir_get_page(thread_get_pagedir(cur_thread), uaddr))
    return false;

  return true;
}

static void sys_halt()
{
  shutdown_power_off();
}

void sys_exit(int status)
{
  struct process *pcb = thread_get_pcb();

  pcb->exit_status = status;
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_exit();
}

static pid_t sys_exec(const char *cmd_line)
{
  pid_t pid;
  struct process *child;
  int i;

  if (!check_validity_useraddr(cmd_line))
    sys_exit(-1);

  for (i = 0; *(cmd_line + i) != NULL; i++)
    if (!check_validity_useraddr(cmd_line + i + 1))
      sys_exit(-1);

  pid = process_execute(cmd_line);
  child = process_get_child(pid);

  if (!child || !child->is_loaded)
    return PID_ERROR;

  return pid;
}

static int sys_wait(pid_t pid)
{
  return process_wait(pid);
}

static int sys_read(int fd, void *buffer, unsigned size)
{
  int i;

  if (!check_validity_useraddr(buffer))
    sys_exit(-1);

  for (i = 0; *(char *)(buffer + i) != NULL; i++)
    if (!check_validity_useraddr(buffer + i + 1))
      sys_exit(-1);

  if (fd == 0)
  {
    unsigned i;

    for (i = 0; i < size; i++)
      *(uint8_t *)(buffer + i) = input_getc();

    return size;
  }
}

static int sys_write(int fd, void *buffer, unsigned size)
{
  int i;

  if (!check_validity_useraddr(buffer))
    sys_exit(-1);

  for (i = 0; *(char *)(buffer + i) != NULL; i++)
    if (!check_validity_useraddr(buffer + i + 1))
      sys_exit(-1);

  if (fd == 1)
  {
    putbuf((const char *)buffer, (size_t)size);

    return size;
  }
}

int sys_fibonacci(int n)
{
  if (n <= 0)
    return 0;
  else if (n == 1)
    return 1;
  return sys_fibonacci(n - 2) + sys_fibonacci(n - 1);
}

int sys_max_of_four(int a, int b, int c, int d)
{
  return max(max(a, b), max(c, d));
}