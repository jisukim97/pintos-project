#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "userprog/process.h"
#include "threads/interrupt.h"
#include "threads/palloc.h"
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
static bool sys_create(const char *, unsigned);
static bool sys_remove(const char *);
static int sys_open(const char *);
static int sys_filesize(int);
static void sys_seek(int, unsigned);
static unsigned sys_tell(int);
void sys_close(int);

static struct lock filesys_lock; /* Lock for file system call. */

void syscall_init(void)
{
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&filesys_lock);
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
  case SYS_CREATE:
  {
    if (!check_validity_useraddr(esp + sizeof(uintptr_t)) || !check_validity_useraddr(esp + 3 * sizeof(uintptr_t) - 1))
      sys_exit(-1);
    char *file = *(char **)(esp + sizeof(uintptr_t));
    unsigned initial_size = *(unsigned *)(esp + 2 * sizeof(uintptr_t));
    f->eax = (uint32_t)sys_create(file, initial_size);
    break;
  }
  case SYS_REMOVE:
  {
    if (!check_validity_useraddr(esp + sizeof(uintptr_t)) || !check_validity_useraddr(esp + 2 * sizeof(uintptr_t) - 1))
      sys_exit(-1);
    char *file = *(char **)(esp + sizeof(uintptr_t));
    f->eax = (uint32_t)sys_remove(file);
    break;
  }
  case SYS_OPEN:
  {
    if (!check_validity_useraddr(esp + sizeof(uintptr_t)) || !check_validity_useraddr(esp + 2 * sizeof(uintptr_t) - 1))
      sys_exit(-1);
    char *file = *(char **)(esp + sizeof(uintptr_t));
    f->eax = (uint32_t)sys_open(file);
    break;
  }
  case SYS_FILESIZE:
  {
    if (!check_validity_useraddr(esp + sizeof(uintptr_t)) || !check_validity_useraddr(esp + 2 * sizeof(uintptr_t) - 1))
      sys_exit(-1);
    int fd = *(int *)(esp + sizeof(uintptr_t));
    f->eax = (uint32_t)sys_filesize(fd);
    break;
  }
  case SYS_SEEK:
  {
    if (!check_validity_useraddr(esp + sizeof(uintptr_t)) || !check_validity_useraddr(esp + 3 * sizeof(uintptr_t) - 1))
      sys_exit(-1);
    int fd = *(int *)(esp + sizeof(uintptr_t));
    unsigned position = *(unsigned *)(esp + 2 * sizeof(uintptr_t));
    sys_seek(fd, position);
    break;
  }
  case SYS_TELL:
  {
    if (!check_validity_useraddr(esp + sizeof(uintptr_t)) || !check_validity_useraddr(esp + 2 * sizeof(uintptr_t) - 1))
      sys_exit(-1);
    int fd = *(int *)(esp + sizeof(uintptr_t));
    f->eax = (uint32_t)sys_tell(fd);
    break;
  }
  case SYS_CLOSE:
  {
    if (!check_validity_useraddr(esp + sizeof(uintptr_t)) || !check_validity_useraddr(esp + 2 * sizeof(uintptr_t) - 1))
      sys_exit(-1);
    int fd = *(int *)(esp + sizeof(uintptr_t));
    sys_close(fd);
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
  struct file_descriptor_entry *fde;
  int bytes_read, i;

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

  fde = process_get_fd(fd);
  if (!fde)
    return -1;
  lock_acquire(&filesys_lock);
  bytes_read = (int)file_read(fde->file, buffer, (off_t)size);
  lock_release(&filesys_lock);
  return bytes_read;
}

static int sys_write(int fd, void *buffer, unsigned size)
{
  struct file_descriptor_entry *fde;
  int bytes_written, i;

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

  fde = process_get_fd(fd);
  if (!fde)
    return -1;

  lock_acquire(&filesys_lock);
  bytes_written = (int)file_write(fde->file, buffer, (off_t)size);
  lock_release(&filesys_lock);
  return bytes_written;
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

static bool sys_create(const char *file, unsigned initial_size)
{
  bool ret;
  int i;
  if (!check_validity_useraddr(file))
    sys_exit(-1);

  for (i = 0; *(file + i) != NULL; i++)
    if (!check_validity_useraddr(file + i + 1))
      sys_exit(-1);

  lock_acquire(&filesys_lock);
  ret = filesys_create(file, (off_t)initial_size);
  lock_release(&filesys_lock);
  return ret;
}

static bool sys_remove(const char *file)
{
  bool ret;
  int i;
  if (!check_validity_useraddr(file))
    sys_exit(-1);

  for (i = 0; *(file + i) != NULL; i++)
    if (!check_validity_useraddr(file + i + 1))
      sys_exit(-1);

  lock_acquire(&filesys_lock);
  ret = filesys_remove(file);
  lock_release(&filesys_lock);
  return ret;
}

static int sys_open(const char *file)
{
  struct file_descriptor_entry *fde;
  struct file *new_file;
  int i;

  if (!check_validity_useraddr(file))
    sys_exit(-1);

  for (i = 0; *(file + i) != NULL; i++)
    if (!check_validity_useraddr(file + i + 1))
      sys_exit(-1);

  fde = palloc_get_page(0);
  if (!fde)
    return -1;

  lock_acquire(&filesys_lock);

  new_file = filesys_open(file);
  if (!new_file)
  {
    palloc_free_page(fde);
    lock_release(&filesys_lock);
    return -1;
  }
  
  fde->fd = thread_get_next_fd();
  fde->file = new_file;
  list_push_back(thread_get_fdt(), &fde->fdt_elem);

  lock_release(&filesys_lock);
  return fde->fd;
}

static int sys_filesize(int fd)
{
  struct file_descriptor_entry *fde = process_get_fd(fd);
  int filesize;
  if (!fde)
    return -1;
  lock_acquire(&filesys_lock);
  filesize = file_length(fde->file);
  lock_release(&filesys_lock);
  return filesize;
}

static void sys_seek(int fd, unsigned position)
{
  struct file_descriptor_entry *fde = process_get_fd(fd);
  if (!fde)
    return;
  lock_acquire(&filesys_lock);
  file_seek(fde->file, (off_t)position);
  lock_release(&filesys_lock);
}

static unsigned sys_tell(int fd)
{
  struct file_descriptor_entry *fde = process_get_fd(fd);
  unsigned pos;
  if (!fde)
    return -1;
  lock_acquire(&filesys_lock);
  pos = (unsigned)file_tell(fde->file);
  lock_release(&filesys_lock);
  return pos;
}

void sys_close(int fd)
{
  struct file_descriptor_entry *fde = process_get_fd(fd);
  if (!fde)
    return;
  lock_acquire(&filesys_lock);
  file_close(fde->file);
  list_remove(&fde->fdt_elem);
  palloc_free_page(fde);
  lock_release(&filesys_lock);
}

struct lock *get_filesys_lock(void)
{
  return &filesys_lock;
}
