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
#ifdef WITH_GDAL
#include <gdal/cpl_string.h>
#include <gdal/gdal_utils.h>
#endif

void check_jnx_file()
{
    char buf[9] = {0};

    seek_block(-8, SEEK_END);
    read_block(buf, sizeof(char), 8);
    seek_block(0, SEEK_SET);
    if ((strncmp(buf, "BirdsEye", 8)) != 0)
    {
	fprintf(stderr, "%s: %s - not jnx file or damaged?\n", PROG_NAME, jnx_file_name);
	close_jnx_file();
	exit(-1);
    }
}

void read_jnx_file()
{
    uint i;

    elements_counter = 0;
    //memory for all element (20 (header) + 9 * 5 (max 5 levels in jnx file)
    elements = malloc((sizeof(struct element)) * 65);

    read_uint_block("version", 0);
    read_uint_block("devid", 0);
    read_int_block("lat1", 0);
    read_int_block("lon1", 0);
    read_int_block("lat2", 0);
    read_int_block("lon2", 0);
    read_uint_block("details", 0);
    read_uint_block("expire", 0);
    read_int_block("productId", 0);
    read_uint_block("crc", 0);
    read_uint_block("signature", 0);
    read_uint_block("signature_offset", 0);
    if ((el_name2uint_value("version", 0)) == 4) read_int_block("zorder", 0);

    for (i = 0; i < el_name2uint_value("details", 0); i++)
    {
	read_uint_block("num_tiles", (i + 1));
	read_uint_block("offset", (i + 1));
	read_uint_block("scale", (i + 1));

	if ((el_name2uint_value("version", 0)) == 4) 
	{
	    read_uint_block("unknown", (i + 1));
	    read_str("copyright", (i + 1));
	}
    } //for

    read_uint_block("block_version", 0);
    if ((el_name2uint_value("block_version", 0)) != 9) 
    {
	free_struct_mem();
	close_jnx_file();
	exit(0);
    }

    read_str("group_id", 0);
    read_str("group_name", 0);
    read_str("unknown_str", 0);
    read_short_block("product_id", 0);
    read_str("map_name", 0);
    read_uint_block("details2", 0);

    for (i = 0; i < el_name2uint_value("details", 0); i++)
    {
	read_str("level_name", (i + 1));
	read_str("level_desc", (i + 1));
	read_str("level_copyright", (i + 1));
	read_uint_block("level_zoom", (i + 1));
    }
}

void read_tiles_table()
{
    uint i, j;
    char buf[30] = {0};

    levels = malloc((sizeof(struct level)) * (el_name2uint_value("details", 0)));
    for (i = 0; i < el_name2uint_value("details", 0); i++)
    {
	seek_block((el_name2uint_value("offset", (i+1))), SEEK_SET);
	levels[i].tiles = malloc(1024);
	levels[i].tiles = malloc(sizeof(struct tile) * (el_name2uint_value("num_tiles", (i+1))));

	for (j = 0; j < (el_name2uint_value("num_tiles", (i+1))); j++)
	{
	    read_block(&levels[i].tiles[j], sizeof(struct tile), 1);
	    if (print_tiles_table)
	    {
		printf("Level %d Tile %d\n", (i + 1), (j + 1));
		printf("      top:        %s\n", jnx_grad2grad(buf, levels[i].tiles[j].top, 1));
		printf("      right:      %s\n", jnx_grad2grad(buf, levels[i].tiles[j].right, 0));
		printf("      bottom:     %s\n", jnx_grad2grad(buf, levels[i].tiles[j].bottom, 1));
		printf("      left:       %s\n", jnx_grad2grad(buf, levels[i].tiles[j].left, 0));
		printf("      WxH:        %dx%d\n", levels[i].tiles[j].width, levels[i].tiles[j].height);
		printf("      jpeg size:  %d Kb\n", (levels[i].tiles[j].size + 2) / 1024);
		printf("      offset:     %#x\n", levels[i].tiles[j].offset);
	    }
	}//for
    }//for
}

#ifdef WITH_GDAL
void export_tiles()
{
    FILE *fp_tmpfile = NULL;
    uint i, j, c; 
    int gdal_error;
    short jpeg_start = 0xd8ff;
    char tilename[20] = {0};
    char coordinates[20] = {0};
    char *dirname_buf, *jpeg_buf;
    char *map_name = el_name2char_value("map_name", 0);
    if ((map_name == NULL) | (map_name[0] == 0))
	map_name = el_name2char_value("group_id", 0);

    tiles_dir[(strlen(tiles_dir))] = '/';
    make_dir(strcat(tiles_dir, map_name));

    GDALAllRegister();
    for (i = 0; i < el_name2uint_value("details", 0); i++)
    {
	dirname_buf = calloc(((strlen(tiles_dir)) + 10), sizeof(char));
	sprintf(dirname_buf, "%s/%c%d", tiles_dir, 'z', (el_name2uint_value("level_zoom", (i+1))));
	make_dir(dirname_buf);
	change_dir(dirname_buf);
	printf("Level %d:\n      exporting tiles to directory %s\n", (i + 1), dirname_buf);

	c = 1;
	for (j = 0; j < el_name2uint_value("num_tiles", (i+1)); j++)
	{
	    if (levels[i].tiles[j].size == 0) continue; // buggy jnx files

	    jpeg_buf = calloc((levels[i].tiles[j].size + 1), sizeof(char));
	    ZERO_CHAR_L(tilename, 20);
	    sprintf(tilename, "%05d.geo.tif", (j + 1));
	    fp_tmpfile = open_file(JPEG_TMP_FILE, "w+");

	    write_one_block(&jpeg_start, 2, fp_tmpfile);  // writing 2 first jpeg bytes (0xFF 0xD8)
	    seek_block(levels[i].tiles[j].offset, SEEK_SET);
	    read_block(jpeg_buf, levels[i].tiles[j].size, 1);
	    write_one_block(jpeg_buf, levels[i].tiles[j].size, fp_tmpfile);
	    c++;
	    free(jpeg_buf);
	    close_file(&fp_tmpfile);
	    
	    char **argv = NULL;
	    argv = CSLAddString(argv, "-q");
	    argv = CSLAddString(argv, "-a_ullr");
	    ZERO_CHAR_L(coordinates, 20);
	    sprintf(coordinates, "%.6f", levels[i].tiles[j].left * 180.0 / 0x7FFFFFFF);
	    argv = CSLAddString(argv, coordinates);   //left    (lon1)
	    ZERO_CHAR_L(coordinates, 20);
	    sprintf(coordinates, "%.6f", levels[i].tiles[j].top * 180.0 / 0x7FFFFFFF);
	    argv = CSLAddString(argv, coordinates);    //top     (lat1)
	    ZERO_CHAR_L(coordinates, 20);
	    sprintf(coordinates, "%.6f", levels[i].tiles[j].right * 180.0 / 0x7FFFFFFF);
	    argv = CSLAddString(argv, coordinates);  //right   (lon2)
	    ZERO_CHAR_L(coordinates, 20);
	    sprintf(coordinates, "%.6f", levels[i].tiles[j].bottom * 180.0 / 0x7FFFFFFF);
	    argv = CSLAddString(argv, coordinates); //bottom  (lat2)
	    argv = CSLAddString(argv, "-co");
	    argv = CSLAddString(argv, "COMPRESS=JPEG");
	    argv = CSLAddString(argv, "-co");
	    argv = CSLAddString(argv, "JPEG_QUALITY=80");
	    argv = CSLAddString(argv, "-co");
	    argv = CSLAddString(argv, "PHOTOMETRIC=YCBCR");
	    argv = CSLAddString(argv, "-a_srs");
	    argv = CSLAddString(argv, "EPSG:4326");
	    gdal_error = 0;
	    GDALDatasetH inDs = GDALOpen(JPEG_TMP_FILE, GA_ReadOnly);
	    GDALTranslateOptions *opt =  GDALTranslateOptionsNew(argv, NULL);
	    GDALDatasetH *outDs = (GDALDatasetH *)GDALTranslate(tilename, inDs, opt, &gdal_error);
	    GDALTranslateOptionsFree(opt);
	    CSLDestroy(argv);
	    GDALClose(inDs);
	    GDALClose(outDs);
	    if (gdal_error != 0) printf("GDAL: error exporting %s/%s\n", dirname_buf, tilename);
	}//for
	printf("      %d GeoTIFF file(s) created\n", (c - 1));
	change_dir("../../../");
	free(dirname_buf);
    }//for
}
#endif //WITH_GDAL

void edit_jnx_file()
{
    FILE *tmp_ptr;
    char *zero_buf;
    short short_value;
    uint i, uint_value; 
    int int_value, jnx_file_free_space, str_diff = 0;

    switch (elements[new_value_number].type)
    {
	case CHAR_VALUE:
	    jnx_file_free_space = 
		elements[el_name2number("offset", 1)].uint_value - 
		elements[el_name2number("level_zoom", el_name2uint_value("details", 0))].offset - 4;
	    if (jnx_file_free_space < 0) jnx_file_free_space = 0;

	    str_diff = elements[new_value_number].length - strlen(new_value);
	    if ((str_diff + jnx_file_free_space) < 0)
	    {
		printf("%s: jnx file hasn't enough free room\n", PROG_NAME);
		return;
	    }
	    printf("# %d, old value = \"%s\", new value = \"%s\"\n",
		    new_value_number, 
		    elements[new_value_number].char_value,
		    new_value);
	    
	    ZERO_CHAR(elements[new_value_number].char_value);
	    strcpy(elements[new_value_number].char_value, new_value);
	break;
	
	case SHORT_VALUE:
	    short_value = (short)atoi(new_value);
	    printf("# %d, old value = %#x(%hi), new value = %#x(%hi)\n", 
		    new_value_number, 
		    elements[new_value_number].short_value, 
		    elements[new_value_number].short_value, 
		    short_value, 
		    short_value);
	    elements[new_value_number].short_value = short_value;
	break;
	
	case UINT_VALUE:
	    uint_value = (uint)strtoul(new_value, NULL, 10);
	    printf("# %d, old value = %#x(%u), new value = %#x(%u)\n",
		    new_value_number, 
		    elements[new_value_number].uint_value, 
		    elements[new_value_number].uint_value, 
		    uint_value, 
		    uint_value);
	    elements[new_value_number].uint_value = uint_value;
	break;
	
	case INT_VALUE:
	    int_value = atoi(new_value);
	    printf("# %d, old value = %#x(%i), new value = %#x(%i)\n",
		    new_value_number, 
		    elements[new_value_number].int_value, 
		    elements[new_value_number].int_value, 
		    int_value, 
		    int_value);
	    elements[new_value_number].int_value = int_value;
	break;
    }
    close_jnx_file();
    tmp_ptr = open_file(jnx_file_name, "r+");
    for (i = 0; i < elements_counter; i++)
    {
	switch (elements[i].type)
	{
	    case CHAR_VALUE:
		write_one_block(elements[i].char_value, (strlen(elements[i].char_value) + 1), tmp_ptr);
	    break;
	    case SHORT_VALUE:
		write_one_block(&elements[i].short_value, 2, tmp_ptr);
	    break;
	    case UINT_VALUE:
		write_one_block(&elements[i].uint_value, 4, tmp_ptr);
	    break;
	    case INT_VALUE:
		write_one_block(&elements[i].int_value, 4, tmp_ptr);
	    break;
	}//switch
    }//for
    if (str_diff > 0) 
    {
	zero_buf = malloc(str_diff);
	ZERO_CHAR_L(zero_buf, str_diff);
	write_one_block(zero_buf, str_diff, tmp_ptr);
	free(zero_buf);
    }
    free(new_value);
    close_file(&tmp_ptr);
}
