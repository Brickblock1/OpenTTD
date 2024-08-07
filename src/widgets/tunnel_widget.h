/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file tunnel_widget.h Types related to the tunnel widgets. */

#ifndef WIDGETS_TUNNEL_WIDGET_H
#define WIDGETS_TUNNEL_WIDGET_H

/** Widgets of the #BuildTunnelWindow class. */
enum BuildTunnelSelectionWidgets {
	WID_BTS_CAPTION,           ///< Caption of the window.
	WID_BTS_DROPDOWN_ORDER,    ///< Direction of sort dropdown.
	WID_BTS_DROPDOWN_CRITERIA, ///< Criteria of sort dropdown.
	WID_BTS_TUNNEL_LIST,       ///< List of tunnels.
	WID_BTS_SCROLLBAR,         ///< Scrollbar of the list.
};

#endif /* WIDGETS_TUNNEL_WIDGET_H */
