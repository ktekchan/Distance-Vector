/**
 * @ktekchan_assignment3
 * @author  Khushboo Tekchandani <ktekchan@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. This file also contains also 
 * contains all the necessary supporting functions.
 */
#include <stdio.h>
#include <stdlib.h>

#include "../include/global.h"
#include "../include/logger.h"
#include "PA3_headers.h"


/*The Bellman Ford algorithm*/
void bellman_ford(){
        int i, j;
        uint16_t dist, min_dist;
        int row, column;

        dist = INF;
        row = current_router.ID-1;

        /*find the least cost to the router from current router*/
        for (i = 0; i < num_servers; i++){
                if (all_routers[i].ID == current_router.ID)
                        continue;

                        column = all_routers[i].ID-1;
                        min_dist = routing_table[row][column];

                        for (j = 0; j < num_neighbors; j++){
                                if (neighbors[j].flag_is_alive == 0) /*the router is not alive yet*/
                                        continue;

                                row = neighbors[j].ID-1;
                                if (routing_table[row][column] == INF)
                                        continue;

                                if (routing_table[row][column] != INF && all_routers[neighbors[j].ID-1].dist != INF){
                                        dist = routing_table[row][column] + all_routers[neighbors[j].ID-1].dist;
                                }

                                if ((dist < min_dist) || min_dist == INF){
                                        min_dist = dist;

                                        all_routers[i].first_hop = neighbors[j].ID;
                                }

                        }
                        row = current_router.ID-1;
                        routing_table[row][column] = min_dist;
        }

        return;
}

/*Function to send an update packet*/
int send_update_pkt(const int sockfd){
        int i, count;
        count = num_neighbors;
        size_t pkt_size;
        struct sockaddr_in neighbor;
        int neighbor_ID;
        int ret;

        pkt_size = sizeof(*update_packet) + sizeof(*update_packet->router_list) * num_servers;

        for (i = 0; i < num_servers; i++){
                update_packet->router_list[i].dist = routing_table[current_router.ID-1][i];
        }

        /*Send updates to neighboring routers*/
        for (i = 0; i < num_servers; i++){
                if (all_routers[i].flag_is_neighbor == 1){
                        neighbor_ID = all_routers[i].ID;

                        memset(&neighbor, 0, sizeof(struct sockaddr_in));
                        neighbor.sin_family = AF_INET;
                        neighbor.sin_addr.s_addr = all_routers[neighbor_ID - 1].IP_addr;

                        neighbor.sin_port = all_routers[neighbor_ID - 1].port_num;
                        printf("Sending packet to %u\n", neighbor_ID);

                        if ((sendto(sockfd, (void *)update_packet, pkt_size, 0, (struct sockaddr*)&neighbor, sizeof(struct sockaddr_in))) < 0){
                                perror ("sendto: ");
                                return -1;
                        }
                }
        }

        return 0;
}

/*Function to receive an update packet*/
int receive_update_packet(const int sockfd){

        int i,j;
        int ret;
        uint16_t neighbor_ID;
        size_t pkt_size;
        struct update_pkt *recv_update_packet;
        struct sockaddr_in neighbor;
        socklen_t len;
        int dummy;

        len = sizeof(struct sockaddr_in);
        memset(&neighbor, 0, sizeof(struct sockaddr_in));

        recv_update_packet = (struct update_pkt*)malloc(sizeof(struct update_pkt) + sizeof(struct dist_vector) * num_servers);
        pkt_size = sizeof(*recv_update_packet) + sizeof(*update_packet->router_list) * num_servers;

        if ((recvfrom(sockfd, (void *)recv_update_packet, pkt_size, 0, (struct sockaddr *)&neighbor, &len)) < 0){
                ret = -1;
        }

        /*Find the router ID of the router that sent the update*/
        for (i = 0; i < num_servers; i++){
                if(recv_update_packet->router_list[i].dist == 0){
                        neighbor_ID = recv_update_packet->router_list[i].ID;

                        for (j = 0; j < num_neighbors; j++){
                                if (neighbor_ID == neighbors[j].ID && neighbors[j].flag_is_alive == 1){
                                        printf ("Update received from ID: %u\n", neighbor_ID);
                                        num_packets++;
                                        break;
                                }
                        }


                        //changed here!
                        /*To check if this is the first update received from a neighbor*/
                        for (j = 0; j < num_neighbors; j++){
                                if (neighbor_ID == neighbors[j].ID &&  neighbors[j].flag_is_alive == 0 && routing_table[current_router.ID-1][neighbor_ID-1] != INF){
                                        neighbors[j].flag_is_alive = 1;
                                        printf ("Update received from ID: %u\n", neighbor_ID);
                                        num_packets++;
                                        break;
                                }
                        }

                break;
                }
        }

        /*Update routing table entries*/
        for (i = 0; i < num_servers; i++){
                routing_table[neighbor_ID-1][recv_update_packet->router_list[i].ID-1] = recv_update_packet->router_list[i].dist;
                routing_table[recv_update_packet->router_list[i].ID-1][neighbor_ID-1] = recv_update_packet->router_list[i].dist;
        }


        bellman_ford();

        for (i = 0; i < num_servers; i++){
                if (neighbors[i].ID == neighbor_ID && neighbors[i].flag_is_alive == 1){
                        neighbors[i].update_skip_count = 0;
                        break;
                }
                ret = 0;
        }

        return ret;
}


/*Function to handle update command*/
void update(uint16_t ID1, uint16_t ID2, char *dist, int flag_disable){
        uint16_t update_ID;
        int i;
        int k = strlen(dist);

        if (flag_disable == 0)dist[k-1] = 0;

        /*To find the ID of the neighbor to change the link distance*/
        if (ID1 == current_router.ID){
                update_ID = ID2;
        }

        else if (ID2 == current_router.ID){
                update_ID = ID1;
        }

        else{
                printf ("Command doesn't have current router ID\n");
                return;
        }

        if (all_routers[update_ID-1].flag_is_neighbor == 0){
                printf ("Cannot update distance for non-neighbor router\n");
                return;
        }

        /*In case the update distance is infinity*/
        if (strcasecmp(dist, "inf") == 0){
                all_routers[update_ID-1].dist = INF;
                all_routers[update_ID-1].first_hop = -1;
                routing_table[update_ID-1][current_router.ID-1] = INF;
                routing_table[current_router.ID-1][update_ID-1] = INF;

                if (flag_disable == 1){
                        all_routers[update_ID-1].flag_is_neighbor = 0;
                        for (i = 0; i < num_neighbors; i++){
                                if (neighbors[i].ID == update_ID){
                                        neighbors[i].flag_is_alive = 0;
                                        break;
                                }
                        }
                }
        }

        /*In case the update distance is finite*/
        else{
                if (all_routers[update_ID - 1].dist == INF){
                        for (i = 0; i < num_neighbors; i++){
                                if(neighbors[i].ID == update_ID){
                                        neighbors[i].update_skip_count = 0;
                                }
                        }
                }

                all_routers[update_ID - 1].dist = atoi(dist);
                routing_table[update_ID-1][current_router.ID-1] = atoi(dist);
                routing_table[current_router.ID-1][update_ID-1] = atoi(dist);

                for (i = 0; i < num_neighbors; i++){
                        if(neighbors[i].ID == update_ID){
                                neighbors[i].flag_is_alive = 1;
                                break;
                        }
                }


        }

        bellman_ford();
}

/*Function to handle input commands from the user*/
void process_input (const int sockfd, fd_set *master_list){

        char *command;
        char buffer[256];
        char *input;
        int i, j, n, k;
        char **tok_argv;
        int argc;
        int flag_disable;

        memset(buffer, 0, 256);

        if (n = read(STDIN, buffer, 256) < 0){
                perror("read");
                return;
        }

        buffer[n-1] = 0;

        input = (char *)malloc(strlen(buffer));
        strcpy(input, buffer);

        k = strlen(input);
        if (k == 1){
                command = "dummy";
        }
        else{
                command = strtok (buffer, " \n");
        }

        while(1){

                if (strcasecmp(command, "academic_integrity") == 0){
                        printf ("Academic Integrity\n");
			cse4589_print_and_log(“I have read and understood the course academic integrity \
			policy located at http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity”);
                        break;
                }

                if (strcasecmp(command, "update") == 0){
                        uint16_t ID1, ID2;
                        char *dist;
                        char *p;
                        flag_disable = 0;

                        p = strtok(NULL, " ");
                        ID1 = (uint16_t)atoi(p);
                        p = strtok(NULL, " ");
                        ID2 = (uint16_t)atoi(p);
                        p = strtok(NULL, " ");
                        dist = p;
                        update (ID1, ID2, dist, flag_disable);
                        break;
                }

                if (strcasecmp(command, "step") == 0){
                        if (send_update_pkt(sockfd) < 0){
				cse4589_print_and_log("%s:%s\n", "step", "Falied to send!")
			}
			else
				cse4589_print_and_log("%s:SUCCESS\n", "step");
                        break;
                 }

                if (strcasecmp(command, "packets") == 0){
                        printf ("The number of update packets received since last invocation: %d\n", num_packets);
                        num_packets = 0;
                        break;
                }

                if (strcasecmp(command, "display") == 0){
                        for (j = 0; j < num_servers; j++){
                                printf("%-15d%-15d%-15d\n", all_routers[j].ID, all_routers[j].first_hop, routing_table[current_router.ID-1][j]);
                        }
                        break;
                }

                if (strcasecmp(command, "disable") == 0){
                        uint16_t disable_ID;
                        char *p;
                        char *dist;
                        dist = "inf";
                        p = strtok (NULL, " ");
                        disable_ID = atoi(p);
                        flag_disable = 1;
                        update (current_router.ID, disable_ID, dist, flag_disable);
                        break;
                }

                if (strcasecmp(command, "crash") == 0){
                        while(1){}
                        break;
                }


                if (strcasecmp(command, "dump") == 0){
                      	size_t pkt_size;
			pkt_size = sizeof(*update_packet) + sizeof(*update_packet->router_list) * num_servers;
                        break;
                }

                printf ("Invalid command!\n");
                break;
        }
        return;
}




/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	/*Init. Logger*/
	cse4589_init_log();

	/*Clear LOGFILE and DUMPFILE*/
	fclose(fopen(LOGFILE, "w"));
	fclose(fopen(DUMPFILE, "wb"));

	/*Start Here*/
        if (argc !=5){

                printf ("Usage: ./assignment3 -t <pathtotopologyfile>\
                        -i <routingupdateinterval>\n");
                return -1;
        }

        char *filepath;
        char buffer[1024];
        int i, j;
        char *p;
        uint16_t read_ID;
        uint16_t read_dist;
        uint16_t ID_temp;
        uint16_t dist_temp;
        int recv;

        filepath = argv[2];

        /*Read the topology file*/
        FILE *file = fopen(filepath, "r");
        fgets (buffer, 1024, file);
        num_servers = atoi(buffer);
        all_routers = (struct routers *)malloc(sizeof(struct routers) * num_servers);

        fgets (buffer, 1024, file);
        num_neighbors = atoi(buffer);
        neighbors = (struct router_neighbor *)malloc(sizeof(struct router_neighbor) * num_neighbors);

        /*Store the information of all routers*/
        for (i = 0; i < num_servers; i++){
                fgets(buffer, 1024, file);
                strtok (buffer, " ");
                read_ID = atoi(buffer);
                all_routers[read_ID-1].ID = read_ID;
                printf ("ID: %u\n", all_routers[read_ID-1].ID);
                p = (char *)strtok (NULL, " ");
                all_routers[read_ID-1].IP_addr = inet_addr(p);
                printf ("IP: %u\n", all_routers[read_ID-1].IP_addr);
                p = (char *)strtok(NULL, " ");
                all_routers[read_ID-1].port_num = htons(atoi(p));
                printf ("Port: %u\n", all_routers[read_ID-1].port_num);
                all_routers[read_ID-1].first_hop = -1;
                all_routers[read_ID-1].dist = INF;
                all_routers[read_ID-1].flag_is_neighbor = 0;
        }
        /*Find current router ID and store the information for it*/
        fgets(buffer, 1024, file);
        strtok (buffer, " ");
        read_ID = atoi(buffer);

        current_router.IP_addr = all_routers[read_ID-1].IP_addr;
        printf ("Current Ip: %u\n", current_router.IP_addr);
        current_router.ID = read_ID;
        printf ("Current ID: %u\n", current_router.ID);
        current_router.port_num = all_routers[read_ID-1].port_num;
        printf ("Current port %u\n", current_router.port_num);
        current_router.dist = 0;
        current_router.first_hop = current_router.ID;
        current_router.flag_is_neighbor = 0;
        all_routers[current_router.ID -1].dist = 0;

        /*Store information about first neighbor*/
        p = (char *)strtok(NULL, " ");
        read_ID = atoi(p);
        p = (char *)strtok(NULL, " ");
        read_dist = atoi(p);
        all_routers[read_ID-1].flag_is_neighbor = 1;
        all_routers[read_ID-1].dist = read_dist;
        all_routers[read_ID-1].first_hop = read_ID;

        neighbors[0].ID = read_ID;
        neighbors[0].update_skip_count = 0;
        neighbors[0].flag_is_alive = 1;

        /*Store information about the neighbors*/
        for (i = 1; i < num_neighbors; i++){
                fgets(buffer, 1024, file);
                strtok (buffer, " ");
                p = (char *)strtok(NULL, " ");
                read_ID = atoi(p);
                p = (char *)strtok(NULL, " ");
                read_dist = atoi(p);
                all_routers[read_ID-1].flag_is_neighbor = 1;
                all_routers[read_ID-1].dist = read_dist;
                all_routers[read_ID-1].first_hop = read_ID;

                neighbors[i].ID = read_ID;
                neighbors[i].update_skip_count = 0;
                neighbors[i].flag_is_alive = 0; //changed here
        }

       /*Initialize the routing table*/
        routing_table = (uint16_t **)malloc(sizeof(uint16_t *) * (num_servers));

        for (i = 0; i < num_servers; i++){
                routing_table[i] = (uint16_t *)malloc(sizeof(uint16_t) * num_servers);
        }

        /*Setting distance to self as 0 and all other distance to INF*/
        for (i = 0; i < num_servers; i++){
                for (j = 0; j < num_servers; j++){
                        if (j == i){
                                routing_table[i][j] = 0;
                        }
                        else
                                routing_table[i][j] = INF;
                }
        }


        /*Setting the remaining distances in the routing table*/
        for (i = 0; i < num_neighbors; i++){
                ID_temp = neighbors[i].ID;
                dist_temp = all_routers[ID_temp-1].dist;
                routing_table[current_router.ID-1][ID_temp-1] = dist_temp;
                routing_table[ID_temp-1][current_router.ID-1] = dist_temp;
        }

        for (i = 0; i < num_servers; i++){
                for (j = 0; j < num_servers; j++){
                        printf ("%u     ", routing_table[i][j]);
                }
                printf ("\n");
        }

        /*Initialize the update_packet*/
        update_packet = (struct update_pkt *)malloc(sizeof(struct update_pkt) + sizeof(struct dist_vector) * num_servers);
        update_packet->num_fields = num_servers;
        update_packet->port_num = current_router.port_num;
        update_packet->IP_addr = current_router.IP_addr;
        for(i = 0; i < num_servers; i++){
                update_packet->router_list[i].IP = all_routers[i].IP_addr;
                printf ("update_packet->router_list[i].IP: %u\n", update_packet->router_list[i].IP);
                update_packet->router_list[i].port = all_routers[i].port_num;
                printf ("update_packet->router_list[i].port: %u\n", update_packet->router_list[i].port);
                update_packet->router_list[i].padding = 0;
                update_packet->router_list[i].ID = all_routers[i].ID;
                printf ("update_packet->router_list[i].ID: %u\n", update_packet->router_list[i].ID);
                update_packet->router_list[i].dist = all_routers[i].dist;
                printf ("update_packet->router_list[i].dist: %u\n", all_routers[i].dist);
        }


        /*Create UDP socket for the router*/
        int sockfd, head_sock, n;
        socklen_t len;

        struct sockaddr_in server_addr;

        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
                perror("socket");
                return -1;
        }

        head_sock = sockfd;

        memset(&server_addr, 0 , sizeof(server_addr));

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = current_router.port_num;
        printf ("Port! %d\n", server_addr.sin_port);

        int optval = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

        if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
                perror("bind");
                return -1;
        }


        fclose(file);

        fd_set master_list, watch_list;
        FD_ZERO (&master_list);
        FD_ZERO (&watch_list);
       FD_SET (sockfd, &master_list);
        FD_SET (STDIN, &master_list);

        struct timeval timeout;
        timeout.tv_sec = atoi(argv[4]);
        timeout.tv_usec = 0;

        int select_return;

        /*Select loop*/
        while (1){

                watch_list = master_list;

                if (select(head_sock + 1, &watch_list, NULL, NULL, &timeout) < 0){
                        perror ("select:");
                        return -1;
                }

                if (FD_ISSET(STDIN, &watch_list)){
                        process_input(sockfd, &watch_list);
                }

                else if (FD_ISSET(sockfd, &watch_list)){
                        recv = receive_update_packet(sockfd);
                }

                else{
                        printf ("Sending update packet!\n");
                        send_update_pkt(sockfd);

                        for (i = 0; i < num_neighbors; i++){
                                neighbors[i].update_skip_count++;
                        }

                        for (i = 0; i < num_neighbors; i++){
                                if (neighbors[i].update_skip_count == 3){
                                        neighbors[i].flag_is_alive = 0;
                                        all_routers[neighbors[i].ID-1].dist = INF;
                                        routing_table[current_router.ID-1][neighbors[i].ID-1] = INF;
                                        routing_table[neighbors[i].ID-1][current_router.ID-1] = INF;
                                }
                        }
                        timeout.tv_sec = atoi(argv[4]);
                        timeout.tv_usec = 0;
                }

        }


	return 0;
}
