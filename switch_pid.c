/*
 * File        : switch_pid.c
 * Description : Deals with shared memory realted to switch process id
 * */
#include <unistd.h>
extern void *shm_switch_pid_ptr;

/*
 * Function    : get_switch_pid
 * Output      : process id of switch
 * Description : whenever this function is called, it returns process id of the switch
 * */
pid_t get_switch_pid()
{
  pid_t *pid = (pid_t *) shm_switch_pid_ptr;
  return *pid; 
}
