/*
 * File        : switch.c
 * Description : To simulate the functionality of network switch
 * */

#include "switch.h"

#define MAX_PORTS 4
#define TABLE_SIZE 10

/* function declarations */
int is_enabled(int);
void disable_port(int);
void enable_port(int);
void display_en_ports();
pid_t is_connected(int);
void connect_port(int);
void disconnect_port(int);
void init_mac_table();
void add_to_mac_table(int, char *);
void delete_entry_from_mac_table(char *);
void display_mac_table();
int get_port_no_from_mac_table(char *);
int is_available(char *);
int get_station_port_num(pid_t);
char *get_station_mac_addr(pid_t);
int init_shared_memories();
int switch_user_menu();
void init_semaphores();

/* frame structure */
typedef struct frame
{
  char src_mac_address[18];
  char dest_mac_address[18];
  char data[28];
} frame_t;

/* mac_table entry structure */
typedef struct mac_table
{
  int port_no;
  char mac_address[18];
  struct mac_table *next;
} mac_table_t;

/* mac_table to store (port,mac_address) (used hash_map) */
mac_table_t *mac_table[TABLE_SIZE];

/* to store thread ids for each port */
pthread_t id[4];

/* a buffer size of 100 bytes for each port to store data receiving from mqueue */
char buffer[4][100];

/* to store port no */
int port[4];

/* to store file descriptors returned by mq_open for each port */
mqd_t mq_fd[4];
mqd_t mq_send_fd[4];

/* semaphores */
sem_t *s_recv[4];
sem_t *s_send[4];

/* to store file descriptors of files opened by each port to log data */
FILE *fptr[4];

/*
 * Function    : broadcast
 * @params     : port_no -> to access port_no related buffer to send data
 * Description : This function forwards the frame to all active and connected ports. It also takes care of traffic filtering.
 * */
void broadcast(int port_no)
{
  int ret, count = 0;
  for(int i=0; i<MAX_PORTS; i++)
  {
    // send frame only if the destination port is enabled and connected to a station and dont send frame on which port it is received
    if(is_enabled(i) && is_connected(i) && (i != (port_no - 1)) )
    {
      /* restoring the original buffer */
      buffer[i][17] = ' ';
      buffer[i][35] = ' ';

      sem_wait(s_send[i+1]);
      /* sending the frame to destination port */
      ret = mq_send(mq_send_fd[i], buffer[port_no - 1], 100, 0);
      sem_post(s_send[i+1]);
      if(ret == -1)
      {
        perror("Error in mq_send()");
        return;
      }
      count++;
      /* logging data to file */
      fprintf(fptr[port_no - 1], "Frame is forwarded to port - %d\n", i+1);
    }
  }
  /* if no port is enabled or connected, log that information to file */
  if(count == 0)
  {
    fprintf(fptr[port_no - 1], "No port is enabled or connected to a station. So frame is not forwarded to any port.\n");
  }
}

/*
 * Function    : unicast
 * @params     : dest_port -> destination port number to forward the frame
 *               src_port  -> port number to access related buffer
 * Description : This function forwards the frame to dest_port
 * */
void unicast(int dest_port, int src_port)
{
  int ret;
  /* forwards the frame to destination port mqueue */
  sem_wait(s_send[dest_port]);
  ret = mq_send(mq_send_fd[dest_port - 1], buffer[src_port - 1], 100, 0);
  sem_post(s_send[dest_port]);
  if(ret == -1)
  {
    perror("Error in mq_send()");
    return;
  }
  /* log data to file */
  fprintf(fptr[src_port - 1], "Frame is forwarded to port - %d\n", dest_port);
}

/*
 * Function    : listen_port
 * @params     : port_no -> pointer to this thread's port_no
 * Description : This function is invoked by init_port function for every port. It will take care of listening traffic on port, mac learning and sends the received frame to its
 *               destination port.
 * */
void *listen_port(void *port_no)
{
  int *temp_port_no = (int *) port_no;

  int ret;
  while(1)
  {
    //sem_wait(s_recv[*temp_port_no]);
    /* Receiving frame from port and stores it in port's related buffer */
    ret = mq_receive(mq_fd[*temp_port_no - 1], buffer[*temp_port_no - 1], 100, NULL);
    //sem_post(s_recv[*temp_port_no]);
    if(ret == -1)
    {
      perror("Error in mq_receive()");
      return NULL;
    }

    void *temp = buffer[*temp_port_no - 1];
    /* typecasting buffer to easily access destination mac address and source mac address */
    frame_t *f = (frame_t *) temp;
    /* modifying space with null character for easy access and will restore back to default while sending frame to its destination port */
    buffer[*temp_port_no - 1][17]='\0';
    buffer[*temp_port_no - 1][35]='\0';

    /* logging data to file */
    fprintf(fptr[*temp_port_no - 1], "\nFrame received on port - %d. Frame's Destination mac address is %s, Frame's Source mac address is %s\n", *temp_port_no, f->dest_mac_address, f->src_mac_address);

    /* mac learning -> if src_mac_address is not available in mac_table, add it to mac_table */
    if(!is_available(f->src_mac_address))
    {
      add_to_mac_table(*temp_port_no, f->src_mac_address);
    }

    /* If the frame is a broadcast frame, broadcast it */
    if(strcmp(f->dest_mac_address, "FF:FF:FF:FF:FF:FF") == 0)
    {
      fprintf(fptr[*temp_port_no - 1], "Broadcasting the frame:\n");
      /* broadcasting the frame */
      broadcast(*temp_port_no);
      fprintf(fptr[*temp_port_no - 1], "\n");

    }
    /* if the destination mac_address is in mac_table, then extract dest_port_no and unicast the frame (forwards the frame to dest_port_no) */
    else if( is_available(f->dest_mac_address) )
    {
      /* extract dest_port_no from mac_table by passing dest_mac_address */
      int dest_port_no = get_port_no_from_mac_table(f->dest_mac_address);
      /* unicast the frame only if the dest_port is enabled and src_port is not equal to dest_port */
      if( is_enabled(dest_port_no - 1) && is_connected(dest_port_no - 1) && *temp_port_no != dest_port_no)
      {
        /* restoring buffer */
        buffer[*temp_port_no - 1][17] = ' ';
        buffer[*temp_port_no - 1][35] = ' ';
        fprintf(fptr[*temp_port_no - 1], "Unicast the frame to port - %d\n", dest_port_no);
        /* unicast the frame */
        unicast(dest_port_no, *temp_port_no);
        fprintf(fptr[*temp_port_no - 1], "\n");
      }
      else
      {
        /* log data to file if the frame is dropped */
        fprintf(fptr[*temp_port_no - 1], "Frame with Dest - %s, Src - %s is dropped\n\n", f->dest_mac_address, f->src_mac_address);
      }
    }
    else
    {
      fprintf(fptr[*temp_port_no - 1], "Unknown Unicast the frame\n");
      /* Unknown unicast the frame */
      broadcast(*temp_port_no);
      fprintf(fptr[*temp_port_no - 1], "\n");
    }
  }
  /* when thread is killed release all memory */
  pthread_detach(pthread_self());
}

/*
 * Function    : init_port
 * @params     : port_no -> port_no to open its related message queues
 * Description : This function openes two message queues related to port port_no and created seperate thread for each port so that each port will listen for incoming frames
 * */
void init_port(int port_no)
{
  /* open message queue and store its descriptor */
  mq_fd[port_no - 1] = mq_open(recv_mq[port_no - 1], _FLAGS, 0777, &mq_port_attr);
  if(mq_fd[port_no - 1] == -1)
  {
    perror("error in mq_open()");
    return;
  }
  mq_send_fd[port_no - 1] = mq_open(send_mq[port_no - 1], _FLAGS, 0777, &mq_port_attr);
  if(mq_send_fd[port_no - 1] == -1)
  {
    perror("error in mq_open()");
    return;
  }

  /* create thread for each port to listen for incoming frames */
  pthread_create(&id[port_no - 1], NULL, listen_port, &port[port_no - 1]);
}

/*
 * Function    : open_file
 * @params     : port_no -> to open files related to port_no
 * Description : This function simply openes the file related to each port and logs welcoming message to the file
 * */
void open_file(int port_no)
{

  time_t t;
  time(&t);

  if( (fptr[port_no - 1] = fopen(file_name[port_no - 1],"a")) == NULL )
  {
    perror("Error in fopen()");
    return;
  }
  fprintf(fptr[port_no - 1], "\n+----------------------------------------------------------------+\n");
  fprintf(fptr[port_no - 1], "   Opened the file for logging data on %s", ctime(&t));
  fprintf(fptr[port_no - 1], "+----------------------------------------------------------------+\n");
}

/*
 * Function    : init_ports
 * Descripton  : This function will initialise each port like enabling the port, make sure every port is disconnected when switch is on and calls init_port function.
 * */
void init_ports()
{
  mq_port_attr.mq_maxmsg = 5; // max msgs in queue is 5
  mq_port_attr.mq_msgsize = 100; // max size of msg in queue is 100
                                 
  init_semaphores();

  init_mac_table();

  for(int i = 0; i < MAX_PORTS; i++)
  {
    port[i]=i+1;
    /* opening file related to each port*/
    open_file(i+1);
    /* enable port */
    enable_port(i+1);
    disconnect_port(i+1);
    /* initialise each port */
    init_port(i+1);
  }
}

/*
 * Function    : switch_off
 * Description : This function will close all opened file descriptors, unlinks all opened mqueues and shared memories
 * */
void switch_off()
{
  /* freeing entries from hash table */
  for(int i=0; i<TABLE_SIZE; i++)
  {
    mac_table_t *temp = mac_table[i];

    while(temp)
    {
      mac_table[i] = temp->next;
      free(temp);
      temp = mac_table[i];
    }
  }

  for(int i=0; i<MAX_PORTS; i++)
  {
    if(is_connected(i))
    {
      /* inform all connected stations that switch has been closed */
      kill(is_connected(i), SIGUSR1);
    }
    mq_close(mq_fd[i]);
    mq_close(mq_send_fd[i]);
    fclose(fptr[i]);
  }

  shm_switch_pid_ptr = NULL;
  shm_en_dis_ports_ptr = NULL;
  shm_con_discon_ports_ptr = NULL;

  /* unlink all message queus */
  for(int i=0; i<MAX_PORTS; i++)
  {
    mq_unlink(send_mq[i]);
    mq_unlink(recv_mq[i]);
  }

  for(int i=0; i<MAX_PORTS; i++)
  {
    sem_close(s_recv[i+1]);
    sem_close(s_send[i+1]);
    sem_unlink(sem_recv_names[i]);
    sem_unlink(sem_send_names[i]);
  }
  
  /* unmap the shared memory */
  munmap(NULL, SWITCH_PID_SIZE);
  munmap(NULL, CON_DISCON_PORTS_SIZE);
  munmap(NULL, EN_DIS_PORTS_SIZE);
  
  /* close shared memory fds */
  close(shm_switch_pid_fd);
  close(shm_en_dis_ports_fd);
  close(shm_con_discon_ports_fd);

  /* unlink shared memories */
  shm_unlink(SWITCH_PID);
  shm_unlink(EN_DIS_PORTS);
  shm_unlink(CON_DISCON_PORTS);

}

/*
 * Function    : sigaction_terminator
 * @params     : sig      -> signal number
 *               info     -> extra information about the signal being handled
 *               ucontext -> user context of the process
 * Description : When ctrl+c is pressed, this function will handle that SIGINT signal and terminates switch by calling switch_off function
 * */
void switch_sigaction_terminator(int sig, siginfo_t *info, void *ucontext)
{
  printf("\nTERMINATING SWITCH.....\n");
  switch_off();
  exit(0);
}

/*
 * Function    : sigaction_handler
 * @params     : sig      -> signal number
 *               info     -> extra information about the signal being handled
 *               ucontext -> user context of the process
 * Description : It handles SIGUSR1 signal generated by station. It removes respective station's entry from mac_table and disconnect that station from the port.
 * */
void switch_sigaction_handler(int sig, siginfo_t *info, void *ucontext)
{
  pid_t station_pid = info->si_pid;
  /* gets mac_address of respective station using station's process id (used shared memory to store pid,mac_address pair)*/
  char *mac_address = get_station_mac_addr(station_pid);
  if(strlen(mac_address) != 0)
  {
    delete_entry_from_mac_table(mac_address);
  }
  int station_port_num = get_station_port_num(station_pid);
  //delete_entry_from_mac_table((con_discon_t *) shm_con_discon_ports_ptr[station_port_num - 1]->mac_address);
  disconnect_port(station_port_num);
}

/*
 * Function    : main
 * Description : It initialises shared memories, ports and displays switch_user_menu
 * */
int main()
{
  int ret;


  /* initialising sigaction structs */
  struct sigaction sa1, sa2;
  sa1.sa_sigaction = switch_sigaction_handler;
  sa1.sa_flags = SA_SIGINFO;
  sa1.sa_restorer = NULL;

  sa2.sa_sigaction = switch_sigaction_terminator;
  sa2.sa_flags = SA_SIGINFO;
  sa2.sa_restorer = NULL;

  /* initialising shared_memories */
  if(init_shared_memories() == EXIT_FAILURE)
  {
    return EXIT_FAILURE;
  }

  /* updating switch_pid shared memory with switch process id */
  pid_t *temp_pid_ptr = (pid_t *) shm_switch_pid_ptr;
  *temp_pid_ptr = getpid();
  temp_pid_ptr = NULL;


  /* intialise ports */
  init_ports();

  /* handle SIGUSR1 and SIGINT signals */
  sigaction(SIGUSR1, &sa1, NULL);
  sigaction(SIGINT, &sa2, NULL);

  /* display switch_user_menu */
  if( switch_user_menu() == EXIT_SUCCESS)
  {
    return EXIT_SUCCESS;
  }

  return 0;
}

