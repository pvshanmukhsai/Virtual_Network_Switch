/*
 * File        : hash_mac_table.c
 * Description : Deals with the mac_table. Performs mac learning, adding (port_no, mac_address) pair into the hash table
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 10

/* mac_table entires is of type mac_table_t */
typedef struct mac_table
{
  int port_no;
  char mac_address[18];
  struct mac_table *next;
} mac_table_t;

extern mac_table_t *mac_table[TABLE_SIZE];

/*
 * Function    : hash
 * @params     : mac_address -> to compute the hash value
 * Output      : returns hash value of mac_address
 * Description : Calculates hash value of mac_address
 * */
unsigned int hash(char *mac_address)
{
  int len = strlen(mac_address);
  unsigned int hash_value = 0;
  for(int i=0; i<len; i++)
  {
    hash_value += mac_address[i];
    hash_value *= mac_address[i];
  }
  return (hash_value%TABLE_SIZE);
}

/*
 * Function    : init_mac_table
 * Description : intitialize mac_table entries to NULL
 * */
void init_mac_table()
{
  for(int i=0; i<TABLE_SIZE; i++)
  {
    mac_table[i] = NULL;
  }
}

/*
 * Function    : add_to_mac_table
 * @params     : port_no     -> port_no of the station to which it is connected
 *               mac_address -> mac_address of the station
 * Description : Inserts (port_no, mac_address) pair into the hash table (i.e. mac_table)
 * */
void add_to_mac_table(int port_no, char *mac_address)
{
  mac_table_t *newnode = (mac_table_t *) malloc(sizeof(mac_table_t));
  if(newnode == NULL)
  {
    perror("Error in malloc()");
    return;
  }
  newnode->port_no = port_no;
  strcpy(newnode->mac_address, mac_address);
  newnode->next = NULL;

  /* get index based on mac_address */
  int index = hash(mac_address);
  /* if some entry is already there, make head points to current entry and current entry's next to already  */
  newnode->next = mac_table[index];
  mac_table[index] = newnode;
}

/*
 * Function    : get_port_no_from_mac_table
 * @params     : mac_address -> to return port_no related to mac_address
 * Output      : returns port_no
 * Description : Returns port_no related to mac_address from mac_table
 * */
int get_port_no_from_mac_table(char *mac_address)
{
  int index = hash(mac_address);
  mac_table_t *temp = mac_table[index];
  while(temp)
  {
    if( strcmp(temp->mac_address, mac_address) == 0 )
    {
      return temp->port_no;
    }
    temp = temp->next;
  }
  return -1;
}

/*
 * Function    : is_available
 * @params     : mac_address -> checks this mac_address is available or not
 * Output      : 0 -> if mac_address is not available in mac_table
 *               1 -> if mac_address is available in mac_table
 * Description : Checks whether the given mac_address is available or not in mac_table
 * */
int is_available(char *mac_address)
{
  int index = hash(mac_address);
  mac_table_t *temp = mac_table[index];

  while(temp)
  {
    if( strcmp(temp->mac_address, mac_address) == 0 )
    {
      return 1;
    }
    temp = temp->next;
  }
  return 0;
}

/*
 * Function    : delete_entry_from_mac_table
 * @params     : mac_address -> deletes mac_address from table
 * Description : deleted mac_address and its port_no pair from the mac_table
 * */
void delete_entry_from_mac_table(char *mac_address)
{
  int index = hash(mac_address);
  mac_table_t *temp = mac_table[index];
  mac_table_t *prev = NULL;

  while(temp != NULL && strcmp(temp->mac_address, mac_address) != 0)
  {
    prev = temp;
    temp = temp->next;
  }
  if(temp == NULL)
  {
    return;
  }
  if(prev == NULL)
  {
    mac_table[index] = temp->next;
    free(temp);
  }
  else
  {
    prev->next = temp->next;
    free(temp);
  }
}

/*
 * Function    : display_mac_table
 * Description : Displays the elements of the mac_table
 * */
void display_mac_table()
{

  printf("\n\n+--------+----------------------+\n");
  printf("|  PORT  |      MAC ADDRESS     |\n");
  printf("+--------+----------------------+\n");
  for(int i=0; i<TABLE_SIZE; i++)
  {
    if(mac_table[i] == NULL)
    {
      continue;
    }
    else
    {
      mac_table_t *temp = mac_table[i];

      while(temp)
      {
        printf("|    %d   |   %s  |\n", temp->port_no, temp->mac_address);
        temp=temp->next;
      }
    }
  }  
  
  /*
  if(temp == NULL)
  {
    printf("\n**********************************\n");
    printf("******* CAM TABLE IS EMPTY *******\n");
    printf("**********************************\n");
    return;
  }
  */
  printf("+--------+----------------------+\n");

}

