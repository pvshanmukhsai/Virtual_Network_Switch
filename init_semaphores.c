#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_PORTS 4
#define _FLAGS O_RDWR | O_CREAT

extern sem_t *s_recv[4];
extern sem_t *s_send[4];

extern char *sem_recv_names[4];
extern char *sem_send_names[4];

void init_semaphore(int port_no)
{
  /* creating semaphore */
  s_recv[port_no - 1] = sem_open(sem_recv_names[port_no - 1], _FLAGS, 0777, 1);
  if(s_recv[port_no - 1] == SEM_FAILED)
  {
    perror("Error in sem_open()");
    exit(EXIT_FAILURE);
  }

  s_send[port_no - 1] = sem_open(sem_send_names[port_no - 1], _FLAGS, 0777, 1);
  if(s_send[port_no - 1] == SEM_FAILED)
  {
    perror("Error in sem_open()");
    exit(EXIT_FAILURE);
  }
}

void init_semaphores()
{
  for(int i=0; i<MAX_PORTS; i++)
  {
    init_semaphore(i+1);
  }
}
