/*
 * File        : switch_user_menu
 * Description : displays switch's user menu
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* function declarations */
int is_enabled(int);
void enable_port(int);
void disable_port(int);
void display_en_ports();
void display_mac_table();
void switch_off();

/*
 * Function    : switch_user_menu
 * Description : Displays switch's user menu and handles relative functions
 * */
int switch_user_menu()
{
  char input_buffer[50];
  char *end_ptr;
  int choice, port_num;
  while(1)
  {
    printf("\n+-----------------------------+\n");
    printf("|        SWITCH MENU          |\n");
    printf("+-----------------------------+\n");
    printf("  [1] Enable Port\n");
    printf("  [2] Disable Port\n");
    printf("  [3] Show All Enabled Ports\n");
    printf("  [4] Display MAC Table\n");
    printf("  [5] Exit\n");
    printf("-------------------------------\n");
    printf("Enter your choice : ");
    fgets(input_buffer, 50, stdin);
    input_buffer[strlen(input_buffer) - 1] = '\0';
    
    choice = strtol(input_buffer, &end_ptr, 10);
    if(*end_ptr != '\0')
    {
      printf("Invalid input.. Input value between 1 and 5 are allowed\n\n");
      continue;
    }

    switch(choice)
    {
      case 1:
        printf("Enter port number to enable : ");
        fgets(input_buffer, 50, stdin);
        input_buffer[strlen(input_buffer) - 1] = '\0';
        port_num = strtol(input_buffer, &end_ptr, 10);
        if(*end_ptr != '\0')
        {
          printf("Invalid input.. Input value between 1 and 4 are allowed\n\n");
          continue;
        }

        if(port_num < 1 || port_num > 4)
        {
          printf("Invalid port number\n\n");
          continue;
        }

        if(is_enabled(port_num - 1))
        {
          printf("Port - %d is already enabled\n\n", port_num);
        }
        else
        {
          /* enable port */
          enable_port(port_num);
          printf("Port - %d is enabled\n\n", port_num);
        }
        continue;
      case 2:
        printf("Enter port number to disable : ");
        fgets(input_buffer, 50, stdin);
        input_buffer[strlen(input_buffer) - 1] = '\0';
        port_num = strtol(input_buffer, &end_ptr, 10);
        if(*end_ptr != '\0')
        {
          printf("Invalid input.. Input value between 1 and 4 are allowed\n\n");
          continue;
        }

        if(port_num < 1 || port_num > 4)
        {
          printf("Invalid port number\n\n");
          continue;
        }
        if(!is_enabled(port_num - 1))
        {
          printf("Port - %d is already disabled\n\n", port_num);
        }
        else
        {
          /* disable port */
          disable_port(port_num);
          printf("Port - %d is disabled\n\n", port_num);
        }
        continue;
      case 3:
        /* display all enabled ports */
        display_en_ports();
        continue;
      case 4:
        /* display mac_table */
        display_mac_table();
        continue;
      case 5:
        /* turn off the switch */
        switch_off();
        return EXIT_SUCCESS;
      default:
        printf("Enter valid choice\n\n");
    }
  }
}
