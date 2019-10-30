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
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

FILE *fp_jnxfile;

void make_dir(char *dir_name)
{
    struct stat sb;

    if (stat(dir_name, &sb) == 0 && S_ISDIR(sb.st_mode)) 
    {
	fprintf(stderr, "mkdir: dir %s already exists\n", dir_name);
	exit(-1);
    } 
    
    if ((mkdir(dir_name, 0777)) == -1) 
    {
	fprintf(stderr, "mkdir: create dir %s %s\n", dir_name, strerror(errno));
	exit(-1);
    }
}

void change_dir(char *dirname)
{
    if ((chdir(dirname)) != 0)
    {
	fprintf(stderr, "chdir: change dir %s %s\n", dirname, strerror(errno));
	exit(-1);
    }
}

FILE *open_file(char *filename, char *mode)
{
    FILE *fptr;
    if ((fptr = fopen(filename, mode)) == NULL)
    {
	fprintf(stderr, "fopen: open file %s %s\n", filename, strerror(errno));
	exit(-1);
    }
    return fptr;
}

void open_jnx_file()
{
    fp_jnxfile = open_file(jnx_file_name, "r");
}

void close_file(FILE *fptr)
{
    fclose(fptr);
}

void close_jnx_file()
{
    fclose(fp_jnxfile);
}

void write_block(void *buf, size_t size, size_t num, FILE *fptr)
{
    if ((fwrite(buf, size, num, fptr)) != num)
	fprintf(stderr, "fwrite: couldn't write into file %s\n", strerror(errno));
}

void write_one_block(void *buf, size_t size, FILE *fptr)
{
    write_block(buf, size, 1, fptr);
}

void read_block(void *buf, size_t size, size_t num)
{
    if ((fread(buf, size, num, fp_jnxfile)) != num)
    {
	fprintf(stderr, "fread: couldn't read file %s %s\n", jnx_file_name, strerror(errno));
	close_jnx_file();
	exit(-1);
    }
}

void read_uint_block(char * value_name, uint layer)
{
    elements[elements_counter].number = elements_counter;
    elements[elements_counter].offset = ftell(fp_jnxfile);
    elements[elements_counter].length = 4;
    elements[elements_counter].type = UINT_VALUE;
    elements[elements_counter].layer = layer;
    strcpy(elements[elements_counter].name, value_name);
    read_block(&elements[elements_counter].uint_value, 4, 1);
    elements_counter++;
}

void read_int_block(char * value_name, uint layer)
{
    elements[elements_counter].number = elements_counter;
    elements[elements_counter].offset = ftell(fp_jnxfile);
    elements[elements_counter].length = 4;
    elements[elements_counter].type = INT_VALUE;
    elements[elements_counter].layer = layer;
    strcpy(elements[elements_counter].name, value_name);
    read_block(&elements[elements_counter].int_value, 4, 1);
    elements_counter++;
}

void read_short_block(char * value_name, uint layer)
{
    elements[elements_counter].number = elements_counter;
    elements[elements_counter].offset = ftell(fp_jnxfile);
    elements[elements_counter].length = 2;
    elements[elements_counter].type = SHORT_VALUE;
    elements[elements_counter].layer = layer;
    strcpy(elements[elements_counter].name, value_name);
    read_block(&elements[elements_counter].short_value, 2, 1);
    elements_counter++;
}

void read_str(char *value_name, uint layer)
{
    uint j = 0;
    char ch;

    elements[elements_counter].number = elements_counter;
    elements[elements_counter].offset = ftell(fp_jnxfile);
    elements[elements_counter].type = CHAR_VALUE;
    elements[elements_counter].layer = layer;
    strcpy(elements[elements_counter].name, value_name);
    do
    {
	read_block(&ch, sizeof(char), 1);
	elements[elements_counter].char_value[j++] = ch;
    } while (ch != 0);
    elements[elements_counter].length = (j - 1);
    elements_counter++;
}

void seek_block(long offset, int whence)
{
    if ((fseek(fp_jnxfile, offset, whence)) != 0)
    {
        fprintf(stderr, "fseek: error set position in file %s %s\n", jnx_file_name, strerror(errno));
	close_jnx_file();
	exit(-2);
    }
}
