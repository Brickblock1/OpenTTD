/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file tunnel.h Header file for tunnels */

#ifndef TUNNEL_H
#define TUNNEL_H

#include "gfx_type.h"
#include "tile_cmd.h"
#include "timer/timer_game_calendar.h"

static const uint MAX_TUNNELS = 2; ///< Maximal number of available tunnel specs.

typedef uint TunnelType; ///< Tunnel spec number.

/**
 * Struct containing information about a single tunnel type
 */
struct TunnelSpec {
	TimerGameCalendar::Year avail_year; ///< the year where it becomes available
	byte min_length;                    ///< the minimum length (not counting start and end tile)
	uint16_t max_length;                ///< the maximum length (not counting start and end tile)
	uint16_t price;                     ///< the price multiplier
	uint16_t speed;                     ///< maximum travel speed (1 unit = 1/1.6 mph = 1 km-ish/h)
	SpriteID sprite_rail;               ///< the sprite which is used in the rail GUI
	SpriteID sprite_road;               ///< the sprite which is used in the road GUI
	PaletteID pal;                      ///< the palette which is used in the GUI
	StringID material;                  ///< the string that contains the tunnel description
	StringID transport_name[2];         ///< description of the tunnel, when built for road or rail
	PalSpriteID **sprite_table;         ///< table of sprites for drawing the tunnel
};

extern TunnelSpec _tunnel[MAX_TUNNELS];

/**
 * Get the specification of a tunnel type.
 * @param i The type of tunnel to get the specification for.
 * @return The specification.
 */
static inline const TunnelSpec *GetTunnelSpec(TunnelType i)
{
	assert(i < lengthof(_tunnel));
	return &_tunnel[i];
}

CommandCost CheckTunnelAvailability(TunnelType tunnel_type, uint tunnel_len, DoCommandFlag flags = DC_NONE);
int CalcTunnelLenCostFactor(int x);

void ResetTunnels();

#endif /* TUNNEL_H */
