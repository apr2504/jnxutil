/******************************************************************************
                         jnxutil - Garmin jnx file manipulation utility.
                             -------------------
         begin                : 0:04 october,15 2019
         copyright            : (C)2019 by Plasteev Vladislav
         email                : apr2504@yandex.ru
*******************************************************************************/
/******************************************************************************
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*******************************************************************************/
#include "common.h"

char *jnx_file_name;
char *new_value = "";

extern void check_jnx_file(void);
extern void read_jnx_file(void);
extern void read_tiles_table(void);
#ifdef WITH_GDAL
extern void export_tiles(void);
#endif
extern void edit_jnx_file(void);

void make_config(int argc, char **argv)
{
    int option;
    work_mode = -1;
    print_tiles_table = 0;
    print_raw_values = 0;

    if (argc < 2) 
    {
	USAGE;
	exit(-1);
    }
    while ((option = getopt(argc, argv, ":Vh?ritev:n:d:")) != -1)
    {
	switch (option)
	{
	    case 'V':
		VERSION;
		exit(1);
	    break;

	    case ':':
		fprintf(stderr, "%s: option '-%c' needs argument\n", PROG_NAME, optopt);
		exit(1);
	    break;

	    case 'v':
		new_value = strdup(optarg);
	    break;

	    case 'i':
		work_mode = INFO;
	    break;

	    case 'n':
		work_mode = EDIT;
		new_value_number = atoi(optarg);
	    break;

	    case 't':
		print_tiles_table = 1;
	    break;

	    case 'r':
		print_raw_values = 1;
	    break;

#ifdef WITH_GDAL
	    case 'e':
		work_mode = TILES;
	    break;

	    case 'd':
		ZERO_CHAR_L(tiles_dir, 512);
		strcpy(tiles_dir, optarg);
	    break;
#endif
	    case 'h':
	    case '?':
	    default :
		USAGE;	  // macros defined in common.h
		exit(1);
	    break;
	} // switch 
    } // while
    if (optind < argc) jnx_file_name = argv[optind];
    if (jnx_file_name == NULL) jnx_file_name = "";
    if ((strlen(jnx_file_name)) == 0)
    {
	fprintf(stderr, "%s: proper jnx filename should be specified\n", PROG_NAME);
	exit(1);
    }
#ifdef WITH_GDAL
    if ((work_mode == TILES) & ((strlen(tiles_dir)) == 0))
    {
	fprintf(stderr, "%s: option '-e' needs output tiles dir ('-d' option)\n", PROG_NAME);
	exit(1);
    }
#endif
    if ((work_mode == EDIT) & ((strlen(new_value)) == 0))
    {
	fprintf(stderr, "%s: option '-n' needs new value ('-v' option)\n", PROG_NAME);
	exit(1);
    }
    if ((work_mode == -1) & (print_tiles_table == 0) & (print_raw_values == 0)) work_mode = INFO;
}

int main(int argc, char **argv)
{
    make_config(argc, argv);
    open_jnx_file();
    check_jnx_file();
    read_jnx_file();
    read_tiles_table();
    if (print_raw_values) elements_print();
    switch (work_mode)
    {
	case INFO:
	    printout();
	break;
#ifdef WITH_GDAL
	case TILES:
	    export_tiles();
	break;
#endif
	case EDIT:
	    edit_jnx_file();
	break;
    }
    free_struct_mem();
    close_jnx_file();
    exit(0);
}
