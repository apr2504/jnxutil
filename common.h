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

#ifndef _COMMON_H_ 
#define _COMMON_H_       1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define JPEG_TMP_FILE "/tmp/jnxutil_tmp.jpg"
#define COPYRIGHT "(C)2019 License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n\nWritten by Plasteev Vladislav <apr2504 at yandex dot ru> Yekaterinburg, Russia\nBased on materials from <http://whiter.brinkster.net/en/JNX.shtml>"

#define ZERO_CHAR_L(str, len88) ({ \
				    auto int len77 = len88; \
				    if (len77 == 0) { len77 = ((strlen(str)) + 1); } \
				    memset((str), 0, (len77 * sizeof(char))); \
				})

#define ZERO_CHAR(str) ZERO_CHAR_L(str, 0);
/*
#define PROG_NAME "jnxutil"
#define PROG_VERSION "0.0.1"
*/


#define  VERSION  printf(\
                           "%s: version %s %s\n%s\n",\
                           PROG_NAME,\
			   PROG_VERSION,\
			   __DATE__,\
			   COPYRIGHT\
			);

#ifdef WITH_GDAL
#define  USAGE printf("Usage: %s -itrehV [-d directory] [-n number] [-v value] jnxfilename\n\
               -i    print info\n\
               -t    print tiles table\n\
               -r    print raw values\n\
               -e    export tiles to GeoTiff format\n\
               -d    directory for exporting tiles\n\
               -n    element number to edit\n\
               -v    new value for element\n\
               -h    this message and exit\n\
               -V    print version\n",PROG_NAME);
#else
#define  USAGE printf("Usage: %s -itrhV [-n number] [-v value] jnxfilename\n\
               -i    print info\n\
               -t    print tiles table\n\
               -r    print raw values\n\
               -n    element number to edit\n\
               -v    new value for element\n\
               -h    this message and exit\n\
               -V    print version\n",PROG_NAME);
#endif

enum
{
    CHAR_VALUE,
    SHORT_VALUE,
    UINT_VALUE,
    INT_VALUE
};

struct tile
{
    uint top;
    uint right;
    uint bottom;
    uint left;
    short int width;
    short int height;
    uint size;
    uint offset;
};

struct element
{
    uint  number;              // element serial number
    uint  offset;              // element offset from the beginning of the jnx file
    uint  length;              // length of the element in bytes
    uint  type;                // char, short, uint, int
    uint  layer;               // 0 - Header, 1 - Level 1 etc
    char  name[20];            // name of the element
    char  char_value[1024];    // if element is string
    short short_value;         // if element is short int
    uint  uint_value;          // if element is unsigned int
    int   int_value;           // if element is int
};

struct level
{
    struct tile *tiles;
};

enum
{ 
    INFO,
    EDIT,
    TILES
};

int work_mode;
uint print_tiles_table, elements_counter, print_raw_values, new_value_number;
char tiles_dir[1024];
struct element *elements;
struct level *levels;

extern char *jnx_file_name;
extern char *new_value;

extern FILE *open_file(char *, char *);
extern char *jnx_grad2grad(char*, int, int);
extern uint el_name2number(char *, uint);
extern uint el_name2uint_value(char *, uint);
extern char *el_name2char_value(char *, uint);
extern void make_config(int, char**);
extern void make_dir(char *);
extern void change_dir(char *);
extern void open_jnx_file();
extern void close_jnx_file();
extern void close_file(FILE **);
extern void write_block(void *, size_t, size_t, FILE *);
extern void write_one_block(void *, size_t, FILE *);
extern void read_block(void *, size_t, size_t);
extern void read_uint_block(char *, uint);
extern void read_int_block(char *, uint);
extern void read_short_block(char *, uint);
extern void read_str(char *, uint);
extern void seek_block(long, int);
extern void printout(void);
extern void free_struct_mem(void);
extern void elements_print(void);

#endif /* _COMMON_H_ */
