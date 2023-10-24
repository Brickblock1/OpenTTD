/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file tunnel_land.h This file contains all the sprites for tunnels
 * It consists of a number of arrays.
 * <ul><li>_tunnel_sprite_table_n_m. Defines all the sprites of a tunnel besides the pylons.
 * n defines the number of the tunnel type, m the number of the section. the highest m for
 * each tunnel set defines the heads.<br>
 * Sprites for middle sections are arranged in groups of four, the elements are:
 * <ol><li>Element containing the track. This element is logically behind the vehicle.</li>
 * <li>Element containing the structure that is logically between the vehicle and the camera</li>
 * <li>Element containing the pylons.</li></ol>
 * First group is for railway in X direction, second for railway in Y direction, two groups each follow for road, monorail and maglev<p>
 * <br>Elements for heads are arranged in groups of eight:
 * <ol><li>X direction, north end, flat</li>
 * <li>Y direction, north end, flat</li>
 * <li>X direction, south end, flat</li>
 * <li>Y direction, south end, flat</li>
 * <li>X direction, north end, sloped</li>
 * <li>Y direction, north end, sloped</li>
 * <li>X direction, south end, sloped</li>
 * <li>Y direction, south end, sloped</li></ol>
 * This is repeated 4 times, for rail, road, monorail, maglev</li>
 * </ul>
 */

# define MN(a) {a, PAL_NONE}
# define MR(a) {a, PALETTE_TO_STRUCT_RED}
# define MW(a) {a, PALETTE_TO_STRUCT_WHITE}
# define MC(a) {a, PALETTE_TO_STRUCT_CONCRETE}

/**
 * Describes the data that defines each tunnel in the game
 * @param y       year of availability
 * @param mnl     minimum length (not counting tunnel heads)
 * @param mxl  	  maximum length (not counting tunnel heads)
 * @param p       price multiplier
 * @param mxs     maximum speed allowed (1 unit = 1/1.6 mph = 1 km-ish/h)
 * @param spr_rl  sprite to use in purchase GUI rail
 * @param spr_rd  sprite to use in purchase GUI road
 * @param plt     palette for the sprite in purchase GUI
 * @param dsc     description of the tunnel in purchase GUI
 * @param nrl     description of the rail tunnel in query tool
 * @param nrd     description of the road tunnel in query tool
 */
#define MBR(y, mnl, mxl, p, mxs, spr_rl, spr_rd, plt, dsc, nrl, nrd) \
	{y, mnl, mxl, p, mxs, spr_rl, spr_rd, plt, dsc, { nrl, nrd }, nullptr,}

const TunnelSpec _orig_tunnel[] = {
/*
	       year of availability
	       |  minimum length
	       |  |   maximum length
	       |  |   |        price multiplier
	       |  |   |        |    maximum speed
	       |  |   |        |    |  			sprite to use in GUI rail
	       |  |   |        |    |  			|      sprite to use in GUI road
		   |  |   |        |    |           |      | 	  palette in GUI
	   string with description               name on rail                                         name on road
	   |                              		 |                                                    | */
	MBR(   0, 0, 0xFFFF,  80,   UINT16_MAX, 0xFF,  0xFF,  PAL_NONE,
	   STR_TUNNEL_NAME_DEFAULT,              STR_LAI_TUNNEL_DESCRIPTION_RAILROAD, 	              STR_LAI_TUNNEL_DESCRIPTION_ROAD),
	MBR(   0, 1, 0,       80,   20,         0xFF,  0xDD,  PAL_NONE,
	   STR_TUNNEL_NAME_DEFAULT,              STR_LAI_TUNNEL_DESCRIPTION_RAILROAD, 	              STR_LAI_TUNNEL_DESCRIPTION_ROAD),
};

#undef MBR
#undef MN
#undef MR
#undef MW
#undef MC
