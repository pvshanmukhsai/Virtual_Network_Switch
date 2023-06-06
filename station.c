/*
 * File        : station.c
 * Description : To simulate the functionality of network switch
 * */

#include "switch.h"

/* function declarations */
void connect_port(int, char *);
int is_enabled(int);
pid_t is_connected(int);
void disconnect_port(int);
pid_t get_switch_pid();
int init_shared_memories();
int station_user_menu();
void init_semaphore(int);

/* frame structure */
typedef struct frame
{
  char src_mac_address[18];
  char dest_mac_address[18];
  char data[28];
} frame_t;

/* file descriptors to access message queues */
mqd_t mq_recv_fd;
mqd_t mq_send_fd;

sem_t *s_recv[4];
sem_t *s_send[4];

/* log file names */
char *station_log[4] = {"log_station1.txt", "log_station2.txt", "log_station3.txt", "log_station4.txt"};

/* buffers related to send frames and receiving frames */
char recv_buffer[100];
char send_buffer[100];
/* to store src mac address and dest mac address */
char src_mac_address[18];
char dest_mac_address[19];
int port_no;
/* thread id*/
pthread_t id;
frame_t f;
/* to store file descriptor */
FILE *fptr;

/*
 * Function    : receive_frames
 * @params     : arg -> arguments to this thread
 * Description : It listens on port for incoming frames.
 * */
void *receive_frames(void *arg)
{
  int ret;

  while(1)
  {
    //sem_wait(s_send[port_no - 1]);
    /* receive frame from mqueue and stores it in recv_buffer */
    ret = mq_receive(mq_recv_fd, recv_buffer, 100, NULL);
    //sem_post(s_send[port_no - 1]);
    if(ret == -1)
    {
      perror("Error in mq_receive()");
      return NULL;
    }

    void *temp = recv_buffer;
    /* type casting for easy access */
    frame_t *f = (frame_t *) temp;
    /* modifiying the frame for easy access of dest_mac_address and src_mac_address of the frame */
    recv_buffer[17]='\0';
    recv_buffer[35]='\0';

    fprintf(fptr, "\nFrame received on port - %d. Frame's Destination address is %s, Frame's Source Address is %s\n", port_no, f->dest_mac_address, f->src_mac_address);

    /* if the frame is a broacast, accept it, or
     * if the frame's destination mac address and
     * station's mac address are equal, accept the frame.
     * Otherwise discard the frame
     * */
    if( strcmp(f->dest_mac_address, "FF:FF:FF:FF:FF:FF") == 0 || strcmp(f->dest_mac_address, src_mac_address) == 0 )
    {
      fprintf(fptr,"Frame with Dest - %s, Src - %s is accepted\n\n", f->dest_mac_address, f->src_mac_address);
    }
    else
    {
      fprintf(fptr, "Frame is discarded\n\n");
    }
  }
  /* release all threads memory when thread is killed */
  pthread_detach(pthread_self());
}

/*
 * Function    : connect_to_port
 * Description : connect station to port by opening respective message queues and log file.
 * */
void connect_to_port()
{
  mq_port_attr.mq_maxmsg = 5; // max msgs in queue is 5
  mq_port_attr.mq_msgsize = 100; // max size of msg in queue is 100

  mq_send_fd = mq_open(recv_mq[port_no - 1], _FLAGS, 0777, &mq_port_attr);
  if(mq_send_fd == -1)
  {
    perror("error in mq_open()");
    return;
  }
  mq_recv_fd = mq_open(send_mq[port_no - 1], _FLAGS, 0777, &mq_port_attr);
  if(mq_recv_fd == -1)
  {
    perror("error in mq_open()");
    return;
  }

  init_semaphore(port_no);

  time_t t;
  time(&t);

  if( (fptr = fopen(station_log[port_no - 1],"a")) == NULL )
  {
    perror("Error in fopen()");
    return;
  }

  fprintf(fptr, "\n+------------------------------------------------------------------------+\n");
  fprintf(fptr, "      Opened the file for logging data on %s", ctime(&t));
  fprintf(fptr, "  STATION WITH MAC ADDRESS - %s IS CONNECTED ON PORT - %d\n", src_mac_address, port_no);
  fprintf(fptr, "+------------------------------------------------------------------------+\n");

  /* update the shared memory with port_no and station's mac address */
  connect_port(port_no, src_mac_address);

  /* create thread to listen incoming frames */
  pthread_create(&id, NULL, receive_frames, NULL);

}

/*
 * Function    : send_frame
 * Description : Sends the frame from station to switch's port
 * */
int send_frame()
{
  int ret;
  /* add src_mac_address in frame */
  strcpy(send_buffer, src_mac_address);
  send_buffer[17] = ' ';
  /* add dest_mac_address in frame */
  strcat(send_buffer, dest_mac_address);
  send_buffer[35] = ' ';

  /* add data to the frame */
  strcat(send_buffer, "*** THIS IS DATA ***");

  sem_wait(s_recv[port_no - 1]);
  /* send the frame to switch */
  ret = mq_send(mq_send_fd, send_buffer, 100, 0);
  sem_post(s_recv[port_no - 1]);
  if(ret == -1)
  {
    perror("Error in mq_send()");
    return EXIT_FAILURE;
  }
  fprintf(fptr, "Frame with destination mac address - %s, source mac address - %s has been sent from station-%d\n\n", dest_mac_address, src_mac_address, port_no);
}

/*
 * Function     : close_station
 * Description  : Closes and opened file descriptors of message queues, shared memories, files and exit the station program
 * */
void close_station()
{
  //disconnect_port(port_no);
  fprintf(fptr, "***** STATION WITH MAC ADDRESS - %s IS DISCONNECTED FROM PORT - %d *****\n", src_mac_address, port_no);
  /* closes log file descriptor */
  fclose(fptr);
  /* closing message queues */
  mq_close(mq_send_fd);
  mq_close(mq_recv_fd);
  /* unmap all shared memories */
  munmap(NULL, SWITCH_PID_SIZE);
  munmap(NULL, CON_DISCON_PORTS_SIZE);
  munmap(NULL, EN_DIS_PORTS_SIZE);
  /* closes shared memories fds */
  close(shm_switch_pid_fd);
  close(shm_en_dis_ports_fd);
  close(shm_con_discon_ports_fd);
  exit(0);
}
/*
 * Function    : sigaction_handler
 * @params     : sig      -> signal number
 *               info     -> extra information about the signal being handled
 *               ucontext -> user context of the process
 * Description : When ctrl+c is pressed, this function will handle that SIGINT signal and terminates station process and sends signal to switch
 * */

void station_sigaction_handler(int sig, siginfo_t *info, void *ucontext)
{
  kill(get_switch_pid(), SIGUSR1);
  close_station();
  exit(0);
}
/*
 * Function    : sigaction_terminator
 * @params     : sig      -> signal number
 *               info     -> extra information about the signal being handled
 *               ucontext -> user context of the process
 * Description : When switch is closed/terminated, SIGUSR1 signal is sent from switch to station. Now this station will call close_port() to close all fds and exit
 * */

void station_sigaction_terminator(int sig, siginfo_t *info, void *ucontext)
{
  printf("\n\n\nSWITCH IS CLOSED\nEXITING......\n");
  close_station();
  exit(0);
}

/*
 * Function    : main
 * @params     : argc -> count of command line arguments
 *               argv -> string array of all command line arguments
 * Description : Connects station to a port, listen traffic on that port and can send traffic to that port. 
 * */
int main(int argc, char *argv[])
{
  int ret;
  int choice;


  /* intialising sigaction structs */
  struct sigaction sa1, sa2;

  sa1.sa_sigaction = station_sigaction_handler;
  sa1.sa_flags = SA_SIGINFO;
  sa1.sa_restorer = NULL;

  sa2.sa_sigaction = station_sigaction_terminator;
  sa2.sa_flags = SA_SIGINFO;
  sa2.sa_restorer = NULL;

  /* initialize all shared memories */
  if(init_shared_memories() == EXIT_FAILURE)
  {
    return EXIT_FAILURE;
  }

  if(argc != 3)
  {
    printf("Usage: ./station <MAC_ADDRESS> <PORT_NO>\n");
    return EXIT_FAILURE;
  }

  /* stores mac address and port number */
  strcpy(src_mac_address, argv[1]);
  port_no = atoi(argv[2]);

  /* check for valid mac_address */
  if(strlen(src_mac_address) != 17 )
  {
    printf("Error: Invalid MAC address\n");
    return EXIT_FAILURE;
  }

  /* check for valid port number */
  if(port_no == 0 || port_no > 4)
  {
    printf("Error: Invalid port number\n");
    return EXIT_FAILURE;
  }

  /* connect the station to the port only if the port is enabled */
  if(!is_enabled(port_no - 1))
  {
    printf("Error: Port is disabled, cannot connect to port - %d\n", port_no);
    return EXIT_FAILURE;
  }

  /* connect the station to the port only if the port is not connected to anyother station */
  if(is_connected(port_no - 1))
  {
    printf("Error: Port is alread connected to a station\n");
    return EXIT_FAILURE;
  }

  /* connect the station to the port */
  connect_to_port();

  /* handle signals */
  sigaction(SIGINT, &sa1, NULL);
  sigaction(SIGUSR1, &sa2, NULL);

  /* display station_user_menu */
  if(station_user_menu() == EXIT_FAILURE)
  {
    return EXIT_FAILURE;
  }

  return 0;
}
