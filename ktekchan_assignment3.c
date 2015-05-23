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
 * This contains the main function. Add further description here....
 */
#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "logger.h"

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
	char buffer[256];
	int num_servers;
	int num_neighbors;
	
	filepath = argv[2];

	FILE *file = fopen(filepath, "r");
	fgets (buffer, 256, file);
	num_servers = atoi(buffer);

	fgets (buffer, 256, file);
	num_neighbors = atoi(buffer);

	fclose(file);
	return 0;
}
