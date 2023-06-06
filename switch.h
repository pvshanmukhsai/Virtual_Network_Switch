#ifndef HELPER_H
#define HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

#define SWITCH_PID "/switch_pid"
#define SWITCH_PID_SIZE sizeof(pid_t)

#define EN_DIS_PORTS "/en_dis_ports"
#define EN_DIS_PORTS_SIZE 16

typedef struct con_discon
{
  pid_t pid;
  char mac_address[18];
} con_discon_t;

#define CON_DISCON_PORTS "/con_discon_ports"
#define CON_DISCON_PORTS_SIZE (4 * sizeof(con_discon_t))

#define _FLAGS O_RDWR | O_CREAT


char *send_mq[4] = {"/send_mq_port_1", "/send_mq_port_2", "/send_mq_port_3", "/send_mq_port_4"};
char *recv_mq[4] = {"/recv_mq_port_1", "/recv_mq_port_2", "/recv_mq_port_3", "/recv_mq_port_4"};
char *file_name[4] = {"port1.txt", "port2.txt", "port3.txt", "port4.txt"};

char *sem_recv_names[4] = {"sem_recv_1", "sem_recv_2", "sem_recv_3", "sem_recv_4"};
char *sem_send_names[4] = {"sem_send_1", "sem_send_2", "sem_send_3", "sem_send_4"};

int shm_switch_pid_fd, shm_en_dis_ports_fd, shm_con_discon_ports_fd;
void *shm_switch_pid_ptr, *shm_en_dis_ports_ptr, *shm_con_discon_ports_ptr;

struct mq_attr mq_port_attr;

#endif

