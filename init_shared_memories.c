/*
 * File        : init_shared_memories.c
 * Description : Initializes three shared memories
 *               1. to store information about enabled/disabled ports
 *               2. to store information about connected station's process id and its mac address
 *               3. to store process id of switch
 * */

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */

/* shared memory of sizeof(pid_t) bytes to store switch process id */
#define SWITCH_PID "/switch_pid"
#define SWITCH_PID_SIZE sizeof(pid_t)

/* shared memory of (4 * sizeof(int)) bytes to store information of port's state i.e enabled or disabled */
#define EN_DIS_PORTS "/en_dis_ports"
#define EN_DIS_PORTS_SIZE (4 * sizeof(int))

/* struct to store in shared memory */
typedef struct con_discon
{
  pid_t pid;
  char mac_address[18];
} con_discon_t;

/* shared memory of (4 * sizeof(con_discon_t)) bytes to store information about station's process id and mac address */
#define CON_DISCON_PORTS "/con_discon_ports"
#define CON_DISCON_PORTS_SIZE (4 * sizeof(con_discon_t))

#define _FLAGS O_RDWR | O_CREAT

/* shared memory descriptors and pointers */
extern int shm_switch_pid_fd, shm_en_dis_ports_fd, shm_con_discon_ports_fd;
extern void *shm_switch_pid_ptr, *shm_en_dis_ports_ptr, *shm_con_discon_ports_ptr;

/*
 * Function    : init_shared_memories
 * Description : initializes all three shared memories
 * */
int init_shared_memories()
{
  int ret;
  /* opening shared memory to store switch process id */
  shm_switch_pid_fd = shm_open(SWITCH_PID, _FLAGS, 0777);
  if(shm_switch_pid_fd == -1)
  {
    perror("Error in shm_open()");
    return EXIT_FAILURE;
  }

  /* allocating size to shared memory */
  ret = ftruncate(shm_switch_pid_fd, SWITCH_PID_SIZE);
  if(ret == -1)
  {
    perror("Error in ftruncate()");
    return EXIT_FAILURE;
  }

  /* mapping virual address space to memory pages */
  shm_switch_pid_ptr = mmap(NULL, SWITCH_PID_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_switch_pid_fd, 0);
  if(shm_switch_pid_ptr == MAP_FAILED)
  {
    perror("Error in mmap()");
    return EXIT_FAILURE;
  }

  /* opening shared memory to enable/disable port status */
  shm_en_dis_ports_fd = shm_open(EN_DIS_PORTS, _FLAGS, 0777);
  if(shm_en_dis_ports_fd == -1)
  {
    perror("Error in shm_open()");
    return EXIT_FAILURE;
  }

  /* allocating size to shared memory */
  ret = ftruncate(shm_en_dis_ports_fd, EN_DIS_PORTS_SIZE);
  if(ret == -1)
  {
    perror("Error in ftruncate()");
    return EXIT_FAILURE;
  }

  /* mapping virual address space to memory pages */
  shm_en_dis_ports_ptr = mmap(NULL, EN_DIS_PORTS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_en_dis_ports_fd, 0);
  if(shm_en_dis_ports_ptr == MAP_FAILED)
  {
    perror("Error in mmap()");
    return EXIT_FAILURE;
  }


  /* opening shared memory to track whether ports are connected to stations or not, by storing port number and its respective station process id */
  shm_con_discon_ports_fd = shm_open(CON_DISCON_PORTS, _FLAGS, 0777);
  if(shm_con_discon_ports_fd == -1)
  {
    perror("Error in shm_open()");
    return EXIT_FAILURE;
  }

  /* allocating size to shared memory */
  ret = ftruncate(shm_con_discon_ports_fd, CON_DISCON_PORTS_SIZE);
  if(ret == -1)
  {
    perror("Error in ftruncate()");
    return EXIT_FAILURE;
  }

  /* mapping virual address space to memory pages */
  shm_con_discon_ports_ptr = mmap(NULL, CON_DISCON_PORTS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_con_discon_ports_fd, 0);
  if(shm_con_discon_ports_ptr == MAP_FAILED)
  {
    perror("Error in mmap()");
    return EXIT_FAILURE;
  }
}
