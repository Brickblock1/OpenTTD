/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file tunnel_gui.cpp Graphical user interface for tunnel construction */

#include "stdafx.h"
#include "error.h"
#include "command_func.h"
#include "rail.h"
#include "road.h"
#include "strings_func.h"
#include "window_func.h"
#include "viewport_func.h"
#include "sound_func.h"
#include "gfx_func.h"
#include "tunnelbridge.h"
#include "tilehighlight_func.h"
#include "sortlist_type.h"
#include "widgets/dropdown_func.h"
#include "core/geometry_func.hpp"
#include "tunnelbridge_map.h"
#include "road_gui.h"
#include "tunnelbridge_cmd.h"

#include "widgets/tunnel_widget.h"

#include "table/strings.h"

#include "safeguards.h"

/** The type of the last built rail tunnel */
static TunnelType _last_railtunnel_type = 0;
/** The type of the last built road tunnel */
static TunnelType _last_roadtunnel_type = 0;

/**
 * Carriage for the data we need if we want to build a tunnel
 */
struct BuildTunnelData {
	TunnelType index;
	const TunnelSpec *spec;
	Money cost;
};

typedef GUIList<BuildTunnelData> GUITunnelList; ///< List of tunnels, used in #BuildTunnelWindow.

/**
 * Callback executed after a build Tunnel CMD has been called
 *
 * @param result Whether the build succeeded
 * @param cmd unused
 * @param tile start tile
 * @param transport_type transport type.
 */
void CcBuildTunnel(Commands cmd, const CommandCost &result, TileIndex tile, TransportType transport_type, TunnelType, byte)
{
	if (result.Succeeded()) {
		if (transport_type = TRANSPORT_RAIL) {
			if (_settings_client.sound.confirm) SndPlayTileFx(SND_20_CONSTRUCTION_RAIL, tile);
			if (!_settings_client.gui.persistent_buildingtools) ResetObjectToPlace();
		} else {
			if (_settings_client.sound.confirm) SndPlayTileFx(SND_1F_CONSTRUCTION_OTHER, tile);
			if (!_settings_client.gui.persistent_buildingtools) ResetObjectToPlace();
			DiagDirection start_direction = ReverseDiagDir(GetTunnelBridgeDirection(tile));
			ConnectRoadToStructure(tile, start_direction);
			TileIndex end_tile = GetOtherTunnelBridgeEnd(tile);
			DiagDirection end_direction = ReverseDiagDir(GetTunnelBridgeDirection(end_tile));
			ConnectRoadToStructure(end_tile, end_direction);
		}
	} else {
		SetRedErrorSquare(_build_tunnel_endtile);
	}
}

/** Window class for handling the tunnel-build GUI. */
class BuildTunnelWindow : public Window {
private:
	/* Runtime saved values */
	static Listing last_sorting; ///< Last setting of the sort.

	/* Constants for sorting the tunnels */
	static const StringID sorter_names[];
	static GUITunnelList::SortFunction * const sorter_funcs[];

	/* Internal variables */
	TileIndex tile;
	TransportType transport_type;
	byte road_rail_type;
	GUITunnelList tunnels;
	int tunneltext_offset; ///< Horizontal offset of the text describing the tunnel properties in #WID_BTS_TUNNEL_LIST relative to the left edge.
	Scrollbar *vscroll;

	/** Sort the tunnels by their index */
	static bool TunnelIndexSorter(const BuildTunnelData &a, const BuildTunnelData &b)
	{
		return a.index < b.index;
	}

	/** Sort the tunnels by their price */
	static bool TunnelPriceSorter(const BuildTunnelData &a, const BuildTunnelData &b)
	{
		return a.cost < b.cost;
	}

	/** Sort the tunnels by their maximum speed */
	static bool TunnelSpeedSorter(const BuildTunnelData &a, const BuildTunnelData &b)
	{
		return a.spec->speed < b.spec->speed;
	}

	void BuildTunnel(TunnelType type)
	{
		switch (this->transport_type) {
			case TRANSPORT_RAIL: _last_railtunnel_type = type; break;
			case TRANSPORT_ROAD: _last_roadtunnel_type = type; break;
			default: break;
		}
		Command<CMD_BUILD_TUNNEL>::Post(STR_ERROR_CAN_T_BUILD_TUNNEL_HERE, CcBuildTunnel,
					this->tile, this->transport_type, type, this->road_rail_type);
	}

	/** Sort the builable tunnels */
	void SortTunnelsList()
	{
		this->tunnels.Sort();

		/* Display the current sort variant */
		this->GetWidget<NWidgetCore>(WID_BTS_DROPDOWN_CRITERIA)->widget_data = this->sorter_names[this->tunnels.SortType()];

		/* Set the modified widgets dirty */
		this->SetWidgetDirty(WID_BTS_DROPDOWN_CRITERIA);
		this->SetWidgetDirty(WID_BTS_TUNNEL_LIST);
	}

	/**
	 * Get the StringID to draw in the selection list and set the appropriate DParams.
	 * @param tunnel_data the tunnel to get the StringID of.
	 * @return the StringID.
	 */
	StringID GetTunnelSelectString(const BuildTunnelData &tunnel_data) const
	{
		SetDParam(0, tunnel_data.spec->material);
		SetDParam(1, PackVelocity(tunnel_data.spec->speed, static_cast<VehicleType>(this->transport_type)));
		SetDParam(2, tunnel_data.cost);
		/* If the tunnel has no meaningful speed limit, don't display it. */
		if (tunnel_data.spec->speed == UINT16_MAX) {
			return _game_mode == GM_EDITOR ? STR_SELECT_TUNNEL_INFO_NAME : STR_SELECT_TUNNEL_INFO_NAME_COST;
		}
		return _game_mode == GM_EDITOR ? STR_SELECT_TUNNEL_INFO_NAME_MAX_SPEED : STR_SELECT_TUNNEL_INFO_NAME_MAX_SPEED_COST;
	}

public:
	BuildTunnelWindow(WindowDesc *desc, TileIndex tile, TransportType transport_type, byte road_rail_type, GUITunnelList &&bl) : Window(desc),
		tile(tile),
		transport_type(transport_type),
		road_rail_type(road_rail_type),
		tunnels(std::move(bl))
	{
		this->CreateNestedTree();
		this->vscroll = this->GetScrollbar(WID_BTS_SCROLLBAR);
		/* Change the data, or the caption of the gui. Set it to road or rail, accordingly. */
		this->GetWidget<NWidgetCore>(WID_BTS_CAPTION)->widget_data = (transport_type == TRANSPORT_ROAD) ? STR_SELECT_ROAD_TUNNEL_CAPTION : STR_SELECT_RAIL_TUNNEL_CAPTION;
		this->FinishInitNested(transport_type); // Initializes 'this->tunneltext_offset'.

		this->parent = FindWindowById(WC_BUILD_TOOLBAR, transport_type);
		this->tunnels.SetListing(this->last_sorting);
		this->tunnels.SetSortFuncs(this->sorter_funcs);
		this->tunnels.NeedResort();
		this->SortTunnelsList();

		this->vscroll->SetCount(this->tunnels.size());
	}

	~BuildTunnelWindow()
	{
		this->last_sorting = this->tunnels.GetListing();
	}

	void UpdateWidgetSize(int widget, Dimension *size, const Dimension &padding, Dimension *fill, Dimension *resize) override
	{
		switch (widget) {
			case WID_BTS_DROPDOWN_ORDER: {
				Dimension d = GetStringBoundingBox(this->GetWidget<NWidgetCore>(widget)->widget_data);
				d.width += padding.width + Window::SortButtonWidth() * 2; // Doubled since the string is centred and it also looks better.
				d.height += padding.height;
				*size = maxdim(*size, d);
				break;
			}
			case WID_BTS_DROPDOWN_CRITERIA: {
				Dimension d = {0, 0};
				for (const StringID *str = this->sorter_names; *str != INVALID_STRING_ID; str++) {
					d = maxdim(d, GetStringBoundingBox(*str));
				}
				d.width += padding.width;
				d.height += padding.height;
				*size = maxdim(*size, d);
				break;
			}
			case WID_BTS_TUNNEL_LIST: {
				Dimension sprite_dim = {0, 0}; // Biggest tunnel sprite dimension
				Dimension text_dim   = {0, 0}; // Biggest text dimension
				for (const BuildTunnelData &tunnel_data : this->tunnels) {
					const TunnelSpec *t = tunnel_data.spec;
					switch (transport_type)
					{
					case TRANSPORT_RAIL:
						if ((tunnel_data.index == 0) || (t->sprite_rail == 0xFF)) {
							sprite_dim = maxdim(sprite_dim, GetSpriteSize(GetRailTypeInfo((RailType)road_rail_type)->gui_sprites.build_tunnel));	
						} else {
							sprite_dim = maxdim(sprite_dim, GetSpriteSize(t->sprite_rail));	
						}
						break;
					
					case TRANSPORT_ROAD:
						if ((tunnel_data.index == 0) || (t->sprite_road == 0xFF)) {
							sprite_dim = maxdim(sprite_dim, GetSpriteSize(GetRoadTypeInfo((RoadType)road_rail_type)->gui_sprites.build_tunnel));	
						} else {
							sprite_dim = maxdim(sprite_dim, GetSpriteSize(t->sprite_road));	
						}
						break;
					
					default:
						break;
					}
					text_dim = maxdim(text_dim, GetStringBoundingBox(GetTunnelSelectString(tunnel_data)));
				}
				sprite_dim.height++; // Sprite is rendered one pixel down in the matrix field.
				text_dim.height++; // Allowing the bottom row pixels to be rendered on the edge of the matrix field.
				resize->height = std::max(sprite_dim.height, text_dim.height) + padding.height; // Max of both sizes + account for matrix edges.

				this->tunneltext_offset = sprite_dim.width + WidgetDimensions::scaled.hsep_normal; // Left edge of text, 1 pixel distance from the sprite.
				size->width = this->tunneltext_offset + text_dim.width + padding.width;
				size->height = 4 * resize->height; // Smallest tunnel gui is 1 entry high in the matrix. 4 seems to be magic number
				break;
			}
		}
	}

	Point OnInitialPosition(int16_t sm_width, int16_t sm_height, int window_number) override
	{
		/* Position the window so hopefully the first tunnel from the list is under the mouse pointer. */
		NWidgetBase *list = this->GetWidget<NWidgetBase>(WID_BTS_TUNNEL_LIST);
		Point corner; // point of the top left corner of the window.
		corner.y = Clamp(_cursor.pos.y - list->pos_y - 5, GetMainViewTop(), GetMainViewBottom() - sm_height);
		corner.x = Clamp(_cursor.pos.x - list->pos_x - 5, 0, _screen.width - sm_width);
		return corner;
	}

	void DrawWidget(const Rect &r, int widget) const override
	{
		switch (widget) {
			case WID_BTS_DROPDOWN_ORDER:
				this->DrawSortButtonState(widget, this->tunnels.IsDescSortOrder() ? SBS_DOWN : SBS_UP);
				break;

			case WID_BTS_TUNNEL_LIST: {
				Rect tr = r.WithHeight(this->resize.step_height).Shrink(WidgetDimensions::scaled.matrix);
				for (int i = this->vscroll->GetPosition(); this->vscroll->IsVisible(i) && i < (int)this->tunnels.size(); i++) {
					const BuildTunnelData &tunnel_data = this->tunnels.at(i);
					const TunnelSpec *t = tunnel_data.spec;
					switch (transport_type)
					{
					case TRANSPORT_RAIL:
						if ((tunnel_data.index == 0) || (t->sprite_rail == 0xFF)) {
							DrawSprite(GetRailTypeInfo((RailType)road_rail_type)->gui_sprites.build_tunnel, t->pal, tr.left, tr.bottom - GetSpriteSize(GetRailTypeInfo((RailType)road_rail_type)->gui_sprites.build_tunnel).height);	
						} else {
							DrawSprite(t->sprite_rail, t->pal, tr.left, tr.bottom - GetSpriteSize(t->sprite_rail).height);	
						}
						break;
					
					case TRANSPORT_ROAD:
						if ((tunnel_data.index == 0) || (t->sprite_road == 0xFF)) {
							DrawSprite(GetRoadTypeInfo((RoadType)road_rail_type)->gui_sprites.build_tunnel, t->pal, tr.left, tr.bottom - GetSpriteSize(GetRoadTypeInfo((RoadType)road_rail_type)->gui_sprites.build_tunnel).height);
						} else {
							DrawSprite(t->sprite_road, t->pal, tr.left, tr.bottom - GetSpriteSize(t->sprite_road).height);	
						}
						break;
					
					default:
						break;
					}
					DrawStringMultiLine(tr.Indent(this->tunneltext_offset, false), GetTunnelSelectString(tunnel_data));
					tr = tr.Translate(0, this->resize.step_height);
				}
				break;
			}
		}
	}

	EventState OnKeyPress(char32_t key, uint16_t keycode) override
	{
		const uint8_t i = keycode - '1';
		if (i < 9 && i < this->tunnels.size()) {
			/* Build the requested tunnel */
			this->BuildTunnel(this->tunnels[i].index);
			this->Close();
			return ES_HANDLED;
		}
		return ES_NOT_HANDLED;
	}

	void OnClick(Point pt, int widget, int click_count) override
	{
		switch (widget) {
			default: break;
			case WID_BTS_TUNNEL_LIST: {
				auto it = this->vscroll->GetScrolledItemFromWidget(this->tunnels, pt.y, this, WID_BTS_TUNNEL_LIST);
				if (it != this->tunnels.end()) {
					this->BuildTunnel(it->index);
					this->Close();
				}
				break;
			}

			case WID_BTS_DROPDOWN_ORDER:
				this->tunnels.ToggleSortOrder();
				this->SetDirty();
				break;

			case WID_BTS_DROPDOWN_CRITERIA:
				ShowDropDownMenu(this, this->sorter_names, this->tunnels.SortType(), WID_BTS_DROPDOWN_CRITERIA, 0, 0);
				break;
		}
	}

	void OnDropdownSelect(int widget, int index) override
	{
		if (widget == WID_BTS_DROPDOWN_CRITERIA && this->tunnels.SortType() != index) {
			this->tunnels.SetSortType(index);

			this->SortTunnelsList();
		}
	}

	void OnResize() override
	{
		this->vscroll->SetCapacityFromWidget(this, WID_BTS_TUNNEL_LIST);
	}
};

/** Set the default sorting for the tunnels */
Listing BuildTunnelWindow::last_sorting = {true, 2};

/** Available tunnel sorting functions. */
GUITunnelList::SortFunction * const BuildTunnelWindow::sorter_funcs[] = {
	&TunnelIndexSorter,
	&TunnelPriceSorter,
	&TunnelSpeedSorter
};

/** Names of the sorting functions. */
const StringID BuildTunnelWindow::sorter_names[] = {
	STR_SORT_BY_NUMBER,
	STR_SORT_BY_COST,
	STR_SORT_BY_MAX_SPEED,
	INVALID_STRING_ID
};

/** Widgets of the tunnel gui. */
static const NWidgetPart _nested_build_tunnel_widgets[] = {
	/* Header */
	NWidget(NWID_HORIZONTAL),
		NWidget(WWT_CLOSEBOX, COLOUR_DARK_GREEN),
		NWidget(WWT_CAPTION, COLOUR_DARK_GREEN, WID_BTS_CAPTION), SetDataTip(STR_SELECT_RAIL_TUNNEL_CAPTION, STR_TOOLTIP_WINDOW_TITLE_DRAG_THIS),
		NWidget(WWT_DEFSIZEBOX, COLOUR_DARK_GREEN),
	EndContainer(),

	NWidget(NWID_HORIZONTAL),
		NWidget(NWID_VERTICAL),
			/* Sort order + criteria buttons */
			NWidget(NWID_HORIZONTAL),
				NWidget(WWT_TEXTBTN, COLOUR_DARK_GREEN, WID_BTS_DROPDOWN_ORDER), SetFill(1, 0), SetDataTip(STR_BUTTON_SORT_BY, STR_TOOLTIP_SORT_ORDER),
				NWidget(WWT_DROPDOWN, COLOUR_DARK_GREEN, WID_BTS_DROPDOWN_CRITERIA), SetFill(1, 0), SetDataTip(0x0, STR_TOOLTIP_SORT_CRITERIA),
			EndContainer(),
			/* Matrix. */
			NWidget(WWT_MATRIX, COLOUR_DARK_GREEN, WID_BTS_TUNNEL_LIST), SetFill(1, 0), SetResize(0, 22), SetMatrixDataTip(1, 0, STR_SELECT_TUNNEL_SELECTION_TOOLTIP), SetScrollbar(WID_BTS_SCROLLBAR),
		EndContainer(),

		/* scrollbar + resize button */
		NWidget(NWID_VERTICAL),
			NWidget(NWID_VSCROLLBAR, COLOUR_DARK_GREEN, WID_BTS_SCROLLBAR),
			NWidget(WWT_RESIZEBOX, COLOUR_DARK_GREEN),
		EndContainer(),
	EndContainer(),
};

/** Window definition for the rail tunnel selection window. */
static WindowDesc _build_tunnel_desc(
	WDP_AUTO, "build_tunnel", 200, 114,
	WC_BUILD_TUNNEL, WC_BUILD_TOOLBAR,
	WDF_CONSTRUCTION,
	std::begin(_nested_build_tunnel_widgets), std::end(_nested_build_tunnel_widgets)
);

/**
 * Prepare the data for the build a tunnel window.
 *  If we can't build a tunnel under the given conditions
 *  show an error message.
 *
 * @param start The start tile of the tunnel
 * @param end The end tile of the tunnel
 * @param transport_type The transport type
 * @param road_rail_type The road/rail type
 */
void ShowBuildTunnelWindow(TileIndex tile, TileIndex tile2, TransportType transport_type, byte road_rail_type)
{
	CloseWindowByClass(WC_BUILD_TUNNEL);
	/* The tunnel length */
	const uint tunnel_len = GetTunnelBridgeLength(tile, tile2);

	/* If Ctrl is being pressed, check whether the last tunnel built is available
	 * If so, return this tunnel type. Otherwise continue normally.
	 * We store tunnel types for each transport type, so we have to check for
	 * the transport type beforehand.
	 */
	TunnelType last_tunnel_type = 0;
	switch (transport_type) {
		case TRANSPORT_ROAD: last_tunnel_type = _last_roadtunnel_type; break;
		case TRANSPORT_RAIL: last_tunnel_type = _last_railtunnel_type; break;
		default: break; // water ways and air routes don't have tunnel types
	}
	if (_ctrl_pressed && CheckTunnelAvailability(last_tunnel_type, tunnel_len + 2).Succeeded()) {
		Command<CMD_BUILD_TUNNEL>::Post(STR_ERROR_CAN_T_BUILD_TUNNEL_HERE, CcBuildTunnel, tile, transport_type, last_tunnel_type, road_rail_type);
		return;
	}
	
	/* only query tunnel building possibility once, result is the same for all tunnels!
	 * returns CMD_ERROR on failure, and price on success */
	StringID errmsg = INVALID_STRING_ID;
	CommandCost ret = Command<CMD_BUILD_TUNNEL>::Do(CommandFlagsToDCFlags(GetCommandFlags<CMD_BUILD_TUNNEL>()) | DC_QUERY_COST, tile, transport_type, last_tunnel_type, road_rail_type);

	GUITunnelList bl;
	if (ret.Failed()) {
		errmsg = ret.GetErrorMessage();
	} else {
		/* check which tunnels can be built */
		const uint tot_tunneldata_len = CalcTunnelLenCostFactor(tunnel_len);

		Money infra_cost = 0;
		switch (transport_type) {
			case TRANSPORT_ROAD: {
				/* In case we add a new road type as well, we must be aware of those costs. */
				RoadType road_rt = INVALID_ROADTYPE;
				RoadType tram_rt = INVALID_ROADTYPE;
				if (IsTunnelTile(tile)) {
					road_rt = GetRoadTypeRoad(tile);
					tram_rt = GetRoadTypeTram(tile);
				}
				if (RoadTypeIsRoad((RoadType)road_rail_type)) {
					road_rt = (RoadType)road_rail_type;
				} else {
					tram_rt = (RoadType)road_rail_type;
				}

				if (road_rt != INVALID_ROADTYPE) infra_cost += (tunnel_len + 2) * 2 * RoadBuildCost(road_rt);
				if (tram_rt != INVALID_ROADTYPE) infra_cost += (tunnel_len + 2) * 2 * RoadBuildCost(tram_rt);

				break;
			}
			case TRANSPORT_RAIL: infra_cost = (tunnel_len + 2) * RailBuildCost((RailType)road_rail_type); break;
			default: break;
		}

		bool any_available = false;
		CommandCost type_check;
		/* loop for all tunneltypes */
		for (TunnelType tun_type = 0; tun_type != MAX_TUNNELS; tun_type++) {
			type_check = CheckTunnelAvailability(tun_type, tunnel_len + 2);
			if (type_check.Succeeded()) {
				/* tunnel is accepted, add to list */
				BuildTunnelData &item = bl.emplace_back();
				item.index = tun_type;
				item.spec = GetTunnelSpec(tun_type);
				/* Add to terraforming & bulldozing costs the cost of the
				 * tunnel itself (not computed with DC_QUERY_COST) */
				item.cost = ret.GetCost() + (((int64_t)tot_tunneldata_len * _price[PR_BUILD_TUNNEL] * item.spec->price) >> 8) + infra_cost;
				any_available = true;
			}
		}
		/* give error cause if no tunnels available here*/
		if (!any_available)
		{
			errmsg = type_check.GetErrorMessage();
		}
	}

	if (!bl.empty()) {
		new BuildTunnelWindow(&_build_tunnel_desc, tile, transport_type, road_rail_type, std::move(bl));
	} else {
		ShowErrorMessage(STR_ERROR_CAN_T_BUILD_TUNNEL_HERE, errmsg, WL_INFO, TileX(tile2) * TILE_SIZE, TileY(tile2) * TILE_SIZE);
	}
}
