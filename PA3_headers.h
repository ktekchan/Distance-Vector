/*-------------------------------------------------

Name: Khushboo Tekchandani
UBIT name: ktekchan

This file contains the necessary headers and 
function declarations.

--------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "logger.h"

/*To store information about the current router*/
struct router_info{
	uint32_t IP_addr,
	uint16_t ID,
	uint16_t port_num,
};

/*To keep a record of the neighbors of the server.*/
struct router_neighbor{
	uint16_t ID,
   /*to keep a count of the number of times update has been missed*/
	int update_skip_count,  
   /*to keep a track of the first time update was received*/
   int flag_is_alive 
};

/*To store the distance vector information*/
struct dist_vector{
	uint32_t server_IP,
	uint16_t server_port,
	uint16_t server_ID,
	uint16_t padding,
	uint16_t cost,
};

/*To store the update packet*/
struct update_pkt{
	uint16_t num_fields,
	uint16_t port_num,
	uint32_t IP_addr,
	struct *dist_vector router_list,
};

/*Academic integrity*/
void academic_integrity();
