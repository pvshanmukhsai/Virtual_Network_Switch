/*
 * File        : con_discon_ports.c
 * Description : Handles station's status i.e connected to a port or not.
 * */
#include <unistd.h>
#include <string.h>

/* struct to store in the shared memory */
typedef struct con_discon
{
  pid_t pid;
  char mac_address[18];
} con_discon_t;

extern void *shm_con_discon_ports_ptr;

/*
 * Funtion     : is_connected
 * @params     : port_no -> to check whether a station is connected to this port
 * Output      : 0   -> if port_no has no connected station
 *               pid -> process id of the connected station to port_no
 * Description : Checks whether a port is connected to a station, if connected returns its process id
 * */
pid_t is_connected(int port_no)
{
  con_discon_t *temp = (con_discon_t *) shm_con_discon_ports_ptr;
  if(temp[port_no].pid == -1)
  {
    return 0;
  }
  return temp[port_no].pid;
}

/*
 * Function    : get_station_port_num
 * @params     : pid -> process id of the station
 * Output      : returns port number on which pid is connected to.
 * Description : based on process id, it returns the port number.
 * */
int get_station_port_num(pid_t pid)
{
  con_discon_t *temp = (con_discon_t *) shm_con_discon_ports_ptr;
  for(int i=0; i<4; i++)
  {
    if(temp[i].pid == pid)
    {
      return i+1;
    }
  }
}

/*
 * Function    : get_station_mac_addr
 * @params     : pid -> process id of the station
 * Output      : returns mac_address of station with process id - pid
 * Description : based on process id, it returns mac_address.
 * */
char *get_station_mac_addr(pid_t pid)
{
  con_discon_t *temp = (con_discon_t *) shm_con_discon_ports_ptr;
  for(int i=0; i<4; i++)
  {
    if(temp[i].pid == pid)
    {
      return temp[i].mac_address;
    }
  }
  return NULL;
}

/*
 * Function    : disconnect_port
 * @params     : port_no -> to change its status
 * Description : Updates shared memory related to port_no with pid of -1 and mac address to null string
 * */
void disconnect_port(int port_no)
{
  con_discon_t *temp = (con_discon_t *) shm_con_discon_ports_ptr;
  temp[port_no - 1].pid = -1;
  temp[port_no - 1].mac_address[0] = '\0';
}

/*
 * Function    : connect_port
 * @params     : port_no     -> to change its status
 *               mac_address -> to update shared memory with station mac address
 * Description : Updates shared memory related to port_no's pid with pid of the calling process and mac address to its station mac address
 * */
void connect_port(int port_no, char *mac_address)
{
  con_discon_t *temp = (con_discon_t *)shm_con_discon_ports_ptr;
  temp[port_no - 1].pid = getpid();
  strcpy(temp[port_no - 1].mac_address, mac_address);
}

