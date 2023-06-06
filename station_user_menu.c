/*
 * File        : switch_user_menu
 * Description : displays switch's user menu
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

/* function declarations */
int send_frame();
pid_t get_switch_pid();
void close_station();
int is_enabled(int);

extern char send_buffer[100], src_mac_address[18], dest_mac_address[19];
extern int port_no;

/*
 * Function    : station_user_menu
 * Description : Displays station_user_menu and handles relative functions 
 * */
int station_user_menu()
{
  int ret, choice;
  char input_buffer[50];
  char *end_ptr;
  while(1)
  {
    memset(send_buffer, '\0', sizeof(send_buffer));

    printf("\n+-----------------------------+\n");
    printf("|       STATION MENU          |\n");
    printf("+-----------------------------+\n");
    printf("  [1] Send Frame\n");
    printf("  [2] Exit\n");
    printf("-------------------------------\n");

    printf("Enter your option:");
    fgets(input_buffer, 50, stdin);
    input_buffer[strlen(input_buffer) - 1] = '\0';
    choice = strtol(input_buffer, &end_ptr, 10);
    if(*end_ptr != '\0')
    {
      printf("Invalid input\n\n");
      continue;
    }

    switch(choice)
    {
      case 1:
        if(!is_enabled(port_no - 1))
        {
          printf("Error: Port is disabled.. Cannont send frames\n\n");
          continue;
        }
        printf("Enter destination MAC address:");
        fgets(dest_mac_address, 19, stdin);
        dest_mac_address[strlen(dest_mac_address) - 1] = '\0';

        /* send frame to dest_mac_address */
        if( send_frame() == EXIT_FAILURE )
        {
          return EXIT_FAILURE;
        }
        continue;

      case 2:
        /* send signal to switch and close the station */
        kill(get_switch_pid(), SIGUSR1);
        close_station();
        //return 0;

      default:
        printf("Enter valid choice\n\n");
    }
  }
}
