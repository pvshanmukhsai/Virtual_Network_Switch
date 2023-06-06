/*
 * File        : en_dis_ports.c
 * Description : It handles the port's state i.e enabled or disabled. Performs activities related to port's state.
 * */
#include <stdio.h>

extern FILE *fptr[4];

extern void *shm_en_dis_ports_ptr;

/*
 * Function    : is_enabled
 * @params     : port_no -> to check port_no's state
 * Output      : 1 -> if port_no is enabled
 *               0 -> if port_no is disabled 
 * Description : Checks whether port_no is enabled or disabled
 * */
int is_enabled(int port_no)
{
  /* typcasting void pointer to int pointer to access information accurately */
  int *temp = (int *) shm_en_dis_ports_ptr;
  if(temp[port_no])
  {
    return 1;
  }
  return 0;
}

/*
 * Function    : display_en_ports
 * Description : It displays all enabled ports to the console
 * */
void display_en_ports()
{
  int *temp = (int *) shm_en_dis_ports_ptr;
  printf("+------+\n");
  printf("| PORT |\n");
  printf("+------+\n");
  for(int i=0; i<4; i++)
  {
    if(is_enabled(i))
      printf("|  %d   |\n", i+1);
  }
  printf("+------+\n");
}

/*
 * Function    : disable_port
 * @params     : port_no -> to change port_no's state
 * Description : It disables port port_no, by updating its shared memory value to 0
 * */
void disable_port(int port_no)
{
  int *temp = (int *) shm_en_dis_ports_ptr;
  temp[port_no - 1] = 0;
  fprintf(fptr[port_no - 1], "Port - %d is disabled\n", port_no);
}

/*
 * Function    : enable_port
 * @params     : port_no -> to change port_no's state
 * Description : It enables port port_no, by updating its shared memory value to 1
 * */
void enable_port(int port_no)
{
  int *temp = (int *) shm_en_dis_ports_ptr;
  temp[port_no - 1] = 1;
  fprintf(fptr[port_no - 1], "Port - %d is enabled\n", port_no);
}

