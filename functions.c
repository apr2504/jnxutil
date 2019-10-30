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
#include <limits.h>


struct zoom
{
    uint jnx;
    char *eTrex;
    uint z;
};

static struct zoom zooms[] =
{  //jnx       eTrex    z
    {UINT_MAX,"800km",  0},
    {2446184, "800km",  1},
    {2446184, "800km",  2},
    {2446184, "800km",  3},
    {1799999, "500km",  4},
    {1223072, "300km",  4},
    {611526,  "200km",  5},
    {458642,  "120km",  6},
    {305758,  "80km",   7},
    {152877,  "50km",   8},
    {114657,  "30km",   8},
    {76437,   "20km",   9},
    {38218,   "12km",   10},
    {28664,   "8km",    10},
    {19109,   "5km",    11},
    {9554,    "3km",    12},
    {7166,    "2km",    12},
    {4777,    "1.2km",  13},
    {2388,    "800m",   14},
    {1791,    "500m",   15},
    {1194,    "300m",   15},
    {597,     "200m",   15},
    {448,     "120m",   15},
    {298,     "80m",    16},
    {149,     "50m",    17},
    {112,     "30m",    17},
    {75,      "20m",    18},
    {37,      "12m",    19},
    {28,      "8m",     19},
    {19,      "5m",     20},
    {19,      "5m",     20},
    {19,      "5m",     21},
    {19,      "5m",     22},
    {19,      "5m",     23},
    {19,      "5m",     24}
};

static char *product_ids[] = 
{
    "BirdsEye",
    "--",
    "BirdsEye Select EIRE",
    "BirdsEye Select Deutschland",
    "BirdsEye Select Great Britain",
    "BirdsEye Select France",
    "BirdsEye Select Kompass - Switzerland",
    "BirdsEye Select Kompass - Austria + East Alps",
    "USGS Quads (BirdsEye TOPO, U.S. and Canada)",
    "NRC TopoRama (BirdsEye TOPO, U.S. and Canada)",
    "--"
};

char *jnx2eTrex_zoom(uint jnx_zoom)
{
    uint i = 0;
    while (zooms[i].jnx >= jnx_zoom) i++;
    return(zooms[i-1].eTrex);
}

char *productId2str(int productId)
{
    int i = 0;
    while ((i < productId) & (i <= 11)) i++;
    return(product_ids[i]);
}

char *jnx_grad2grad(char *buf, int in_grad, int lat)
{
    char ch;
    int grad;
    double min, sec;
    double dec_grad = in_grad * 180.0 / 0x7FFFFFFF;
    
    if ((dec_grad > 0) & (lat == 1)) ch = 'N';
    if ((dec_grad > 0) & (lat == 0)) ch = 'E';
    if ((dec_grad < 0) & (lat == 1)) ch = 'S';
    if ((dec_grad < 0) & (lat == 0)) ch = 'W';
    grad = (int)dec_grad;
    min = (dec_grad - grad) * 60.0;
    sec = (min - (int)min) * 60.0;

    ZERO_CHAR_L(buf, 30);
    sprintf(buf, "%c%.4f° (%c%d°%02d'%04.1f\")", ch, (dec_grad < 0 ? -dec_grad : dec_grad), ch, grad, (int)min, sec);
    return(buf);
}

uint el_name2uint_value(char *el_name, uint layer)
{
    for (uint i = 0; i < 65; i++)
    {
	if (((strcmp(el_name, elements[i].name)) == 0) & (elements[i].layer == layer))
	    return elements[i].uint_value;
    }
    return 0;
}

int el_name2int_value(char *el_name, uint layer)
{
    for (uint i = 0; i < 65; i++)
    {
	if (((strcmp(el_name, elements[i].name)) == 0) & (elements[i].layer == layer))
	    return elements[i].int_value;
    }
    return 0;
}

short el_name2short_value(char *el_name, uint layer)
{
    for (uint i = 0; i < 65; i++)
    {
	if (((strcmp(el_name, elements[i].name)) == 0) & (elements[i].layer == layer))
	    return elements[i].short_value;
    }
    return 0;
}

char *el_name2char_value(char *el_name, uint layer)
{
    for (uint i = 0; i < 65; i++)
    {
	if (((strcmp(el_name, elements[i].name)) == 0) & (elements[i].layer == layer))
	    return elements[i].char_value;
    }
    return 0;
}

uint el_name2number(char *el_name, uint layer)
{
    for (uint i = 0; i < 65; i++)
    {
	if (((strcmp(el_name, elements[i].name)) == 0) & (elements[i].layer == layer))
	    return elements[i].number;
    }
    return 0;
}

void elements_print()
{
    FILE *fptr;
    int r;
    uint g;
    short int s;
    char *buf;

    fptr = open_file(jnx_file_name, "r");
    printf(" \n# | offset | len |        value\n--------------------------------------------\n");
    for (uint i = 0; i < elements_counter; i++)
    {
	switch (elements[i].type)
	{
	    case INT_VALUE:
		fseek(fptr, elements[i].offset, SEEK_SET);
		fread(&r, 4, 1, fptr);
		printf("%2d | 0x%04x | %2d  | %i(%#x)\n", i, elements[i].offset, elements[i].length, r, r);
	    break;
	    case UINT_VALUE:
		fseek(fptr, elements[i].offset, SEEK_SET);
		fread(&g, 4, 1, fptr);
		printf("%2d | 0x%04x | %2d  | %u(%#x)\n", i, elements[i].offset, elements[i].length, g, g);
	    break;
	    case SHORT_VALUE:
		fseek(fptr, elements[i].offset, SEEK_SET);
		fread(&s, 2, 1, fptr);
		printf("%2d | 0x%04x | %2d  | %hi(%#x)\n", i, elements[i].offset, elements[i].length, s, s);
	    break;
	    case CHAR_VALUE:
		fseek(fptr, elements[i].offset, SEEK_SET);
		buf = calloc((elements[i].length + 1), sizeof(char));
		fread(buf, sizeof(char), elements[i].length, fptr);
		printf("%2d | 0x%04x | %2d  | \"%s\"\n", i, elements[i].offset, elements[i].length, buf);
		free(buf);
	    break;
	}//switch
    }//for
    fclose(fptr);
    printf("elements total: %d\n\n", elements_counter);
}

void printout()
{
    uint i;
    char buf[30] = {0};
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("\n#  |       parameter name          |            value\n");
    printf("-------------------------------------------------------------------------\n");
    printf("0  | version:                      | %u\n", el_name2uint_value("version", 0));
    printf("1  | device ID:                    | %u\n", el_name2uint_value("devid", 0));
    printf("2  | top:                          | %s\n", jnx_grad2grad(buf, el_name2int_value("lat1", 0), 1));
    printf("3  | right:                        | %s\n", jnx_grad2grad(buf, el_name2int_value("lon1", 0), 0));
    printf("4  | bottom:                       | %s\n", jnx_grad2grad(buf, el_name2int_value("lat2", 0), 1));
    printf("5  | left:                         | %s\n", jnx_grad2grad(buf, el_name2int_value("lon2", 0), 0));
    printf("6  | number of levels of detail:   | %u\n", el_name2uint_value("details", 0));
    printf("7  | subscription expiration date: | %u\n", el_name2uint_value("expire", 0));
    printf("8  | subscription product ID:      | %i(%s)\n", el_name2int_value("productId", 0), productId2str(el_name2int_value("productId", 0)));
    printf("9  | crc32 of tile coordinates:    | %u\n", el_name2uint_value("crc", 0));
    printf("10 | signature version:            | %u\n", el_name2uint_value("signature", 0));
    printf("11 | signature offset:             | %u\n", el_name2uint_value("signature_offset", 0));
    if ((el_name2uint_value("version", 0)) == 4) 
    {
	printf("%d | z-order:                      | %i\n", el_name2number("zorder", 0), el_name2int_value("zorder", 0));
    }
    printf("%d | block version:                | %u\n", el_name2number("block_version", 0), el_name2uint_value("block_version", 0));
    printf("%d | group ID:                     | %s\n", el_name2number("group_id", 0), el_name2char_value("group_id", 0));
    printf("%d | group name:                   | %s\n", el_name2number("group_name", 0), el_name2char_value("group_name", 0));
    printf("%d | unknown string:               | \"%s\"\n", el_name2number("unknown_str", 0), el_name2char_value("unknown_str", 0));
    printf("%d | subscription product ID:      | %hi\n", el_name2number("product_id", 0), el_name2short_value("product_id", 0));
    printf("%d | map name:                     | %s\n", el_name2number("map_name", 0), el_name2char_value("map_name", 0));
    printf("%d | number of levels:             | %u\n", el_name2number("details2", 0), el_name2uint_value("details2", 0));

    for (i = 0; i < el_name2uint_value("details", 0); i++)
    {
	printf("                       Level %d:\n", (i + 1));
	printf("%d |      number of tiles:         | %u\n", el_name2number("num_tiles", (i+1)), el_name2uint_value("num_tiles", (i+1)));
	printf("%d |      tiles table offset:      | %#x\n", el_name2number("offset", (i+1)), el_name2uint_value("offset", (i+1)));
	printf("%d |      jnx(eTrex) scale:        | %u(%s)\n", el_name2number("scale", (i+1)), el_name2uint_value("scale", (i+1)), jnx2eTrex_zoom(el_name2uint_value("scale", (i+1))));
	if ((el_name2uint_value("version", 0)) == 4) 
	{
	    printf("%d |      unknown attribute:       | %u\n", el_name2number("unknown", (i+1)), el_name2uint_value("unknown", (i+1)));
	    printf("%d |      copyright:               | %s\n", el_name2number("copyright", (i+1)), el_name2char_value("copyright", (i+1)));
	}
	    printf("%d |      level name:              | %s\n", el_name2number("level_name", (i+1)), el_name2char_value("level_name", (i+1)));
	    printf("%d |      level description:       | %s\n", el_name2number("level_desc", (i+1)), el_name2char_value("level_desc", (i+1)));
	    printf("%d |      level copyright:         | %s\n", el_name2number("level_copyright", (i+1)), el_name2char_value("level_copyright", (i+1)));
	    printf("%d |      level zoom:              | z%d\n", el_name2number("level_zoom", (i+1)), el_name2uint_value("level_zoom", (i+1)));
    }//for
}

void free_struct_mem()
{
    for (uint i = 0; i < el_name2uint_value("details", 0); i++) free(levels[i].tiles);
    free(levels);
    free(elements);
}
