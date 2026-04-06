/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0>.
 */

/** @file newgrf_act0_tunnels.cpp NewGRF Action 0x00 handler for tunnels. */

#include "../stdafx.h"
#include "../debug.h"
#include "../tunnel.h"
#include "newgrf_bytereader.h"
#include "newgrf_internal.h"
#include "newgrf_stringmapping.h"

#include "../safeguards.h"

/**
 * Define properties for tunnels
 * @param first Local ID of the first tunnel.
 * @param last Local ID of the last tunnel.
 * @param prop The property to change.
 * @param buf The property value.
 * @return ChangeInfoResult.
 */
static ChangeInfoResult TunnelChangeInfo(uint first, uint last, int prop, ByteReader &buf)
{
	ChangeInfoResult ret = CIR_SUCCESS;

	if (last > MAX_TUNNELS) {
		GrfMsg(1, "TunnelChangeInfo: Tunnel {} is invalid, max {}, ignoring", last, MAX_TUNNELS);
		return CIR_INVALID_ID;
	}

	for (uint id = first; id < last; ++id) {
		TunnelSpec *tunnel = &_tunnel[id];

		switch (prop) {
			case 0x09: // Minimum length
				tunnel->min_length = buf.ReadByte();
				break;

			case 0x0A: // Maximum length
				tunnel->max_length = buf.ReadByte();
				if (tunnel->max_length > 16) tunnel->max_length = UINT16_MAX;
				break;
				
			case 0x0C: // Maximum speed
				tunnel->speed = buf.ReadWord();
				if (tunnel->speed == 0) tunnel->speed = UINT16_MAX;
				break;

			case 0x0F: // Long format year of availability (year since year 0)
				tunnel->avail_year = Clamp(TimerGameCalendar::Year(buf.ReadDWord()), CalendarTime::MIN_YEAR, CalendarTime::MAX_YEAR);
				break;

			case 0x10: { // purchase string
				AddStringForMapping(GRFStringID{buf.ReadWord()}, &tunnel->description);
				break;
			}

			case 0x11: // description of tunnel with rails
				AddStringForMapping(GRFStringID{buf.ReadWord()}, &tunnel->transport_name[0]);
				break;

			case 0x12: // description of tunnel with roads
				AddStringForMapping(GRFStringID{buf.ReadWord()}, &tunnel->transport_name[1]);
				break;

			case 0x13: // 16 bits cost multiplier
				tunnel->price = buf.ReadWord();
				break;

			default:
				ret = CIR_UNKNOWN;
				break;
		}
	}

	return ret;
}

template <> ChangeInfoResult GrfChangeInfoHandler<GSF_TUNNELS>::Reserve(uint, uint, int, ByteReader &) { return CIR_UNHANDLED; }
template <> ChangeInfoResult GrfChangeInfoHandler<GSF_TUNNELS>::Activation(uint first, uint last, int prop, ByteReader &buf) { return TunnelChangeInfo(first, last, prop, buf); }
