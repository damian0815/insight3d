/*

  insight3d - image based 3d modelling software
  Copyright (C) 2007-2008  Lukas Mach
                           email: lukas.mach@gmail.com 
                           web: http://mach.matfyz.cz/

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
  
*/

#include "ui_core.h"

// initialize user interface
bool ui_initialize()
{ 
	// note why aren't we memsetting the whole thing to 0s?

	// if we're on linux, we use gtk for dialogs (as every sane person should do)  
#ifdef LINUX
	gtk_init(NULL, NULL);
#endif

	// set default application mode to "shot mode"
	ui_state.mode = UI_MODE_SHOT; 
	ui_state.previous_mode = UI_MODE_INSPECTION;

	// user interface state initialization
	ui_state.key_state = ALLOC(Uint8, SDLK_LAST);
	memset(ui_state.key_state, 0, sizeof(Uint8) * SDLK_LAST);
	ui_state.keys = SDL_GetKeyState(&ui_state.keys_length);
	ui_state.shot_clear_keys = ALLOC(bool, ui_state.keys_length); 
	ui_state.inspection_clear_keys = ALLOC(bool, ui_state.keys_length); 
	ui_state.overview_clear_keys = ALLOC(bool, ui_state.keys_length);
	ui_state.mouse_x = ui_state.mouse_y = ui_state.mouse_down_x = ui_state.mouse_down_y = 0; 
	ui_state.mouse_down_ticks = 0;
	ui_state.mouse_down = false;

	// todo options
	// todo rewrite

	// shot clear keys 
	memset(ui_state.shot_clear_keys, 0, ui_state.keys_length * sizeof(bool));
	ui_state.shot_clear_keys[SDLK_LEFT] = ui_state.shot_clear_keys[SDLK_RIGHT] = ui_state.shot_clear_keys[SDLK_UP] 
	= ui_state.shot_clear_keys[SDLK_DOWN] = ui_state.shot_clear_keys[SDLK_PAGEDOWN] = ui_state.shot_clear_keys[SDLK_PAGEUP] 
	= ui_state.shot_clear_keys[SDLK_HOME] = ui_state.shot_clear_keys[SDLK_END]
	= true;

	// inspection clear keys 
	memset(ui_state.inspection_clear_keys, 0, ui_state.keys_length * sizeof(bool));
	ui_state.inspection_clear_keys[SDLK_g] = true; 

	// overview clear keys
	memset(ui_state.overview_clear_keys, 0, ui_state.keys_length * sizeof(bool));
	
	// keys cleared in all states 
	ui_state.overview_clear_keys[SDLK_TAB] = ui_state.overview_clear_keys[SDLK_F10] = true; 
	ui_state.shot_clear_keys[SDLK_TAB] = ui_state.shot_clear_keys[SDLK_F10] = true; 
	ui_state.inspection_clear_keys[SDLK_TAB] = ui_state.inspection_clear_keys[SDLK_F10] = true; 
	ui_state.inspection_clear_keys[SDLK_0] = ui_state.shot_clear_keys[SDLK_0] = ui_state.overview_clear_keys[SDLK_0] = true; 
	ui_state.inspection_clear_keys[SDLK_1] = ui_state.shot_clear_keys[SDLK_1] = ui_state.overview_clear_keys[SDLK_1] = true; 
	ui_state.inspection_clear_keys[SDLK_2] = ui_state.shot_clear_keys[SDLK_2] = ui_state.overview_clear_keys[SDLK_2] = true; 
	ui_state.inspection_clear_keys[SDLK_3] = ui_state.shot_clear_keys[SDLK_3] = ui_state.overview_clear_keys[SDLK_3] = true; 
	ui_state.inspection_clear_keys[SDLK_4] = ui_state.shot_clear_keys[SDLK_4] = ui_state.overview_clear_keys[SDLK_4] = true; 
	ui_state.inspection_clear_keys[SDLK_5] = ui_state.shot_clear_keys[SDLK_5] = ui_state.overview_clear_keys[SDLK_5] = true; 

	// create at least five object groups (and the default group 0)
	for (int i = 0; i <= 5; i++) 
	{
		DYN(ui_state.groups, 0);
	}

	// initialize indices 
	INDEX_CLEAR(ui_state.current_shot); 
	INDEX_CLEAR(ui_state.processed_polygon); 
	INDEX_CLEAR(ui_state.processed_vertex); 
	INDEX_CLEAR(ui_state.focused_point);

	// initialize tools // note it's necessary to create at least one tool...
	memset(&tools_state, 0, sizeof(Tools_State));
	tools_state.count = 0;
	tools_state.current = 0; 
	DYN_INIT(tools_state.menu_items);
	tools_state.finalized = false;

	return true;
}

// release allocated memory (called when program terminates)
void ui_release()
{
	FREE(ui_state.shot_clear_keys);
	FREE(ui_state.inspection_clear_keys);
	FREE(ui_state.overview_clear_keys);
}

// create new GUI item structure 
UI_Meta * ui_create_meta(UI_Item_Type type, size_t index)
{
	UI_Meta * meta = ALLOC(UI_Meta, 1);
	if (!meta)
	{
		core_state.error = CORE_ERROR_OUT_OF_MEMORY;
		return NULL;
	}
	memset(meta, 0, sizeof(UI_Meta));
	meta->index = index; 
	meta->type = type;
	return meta; 
}

// create new GUI item structure for section
UI_Section_Meta * ui_check_section_meta(UI_Section_Meta * & meta)
{
	if (!meta) 
	{
		meta = ALLOC(UI_Section_Meta, 1);
		meta->type = UI_ITEM_SECTION;
		meta->unfolded = true;
	}

	return meta;
}

// create new GUI item structure for shot 
UI_Shot_Meta * ui_check_shot_meta(size_t shot_id) 
{
	// check if this shot already has meta structure
	ASSERT(validate_shot(shot_id), "invalid shot supplied when checking for meta structure");

	if (!shots.data[shot_id].ui)
	{
		UI_Shot_Meta * meta = ALLOC(UI_Shot_Meta, 1);
		if (!meta) 
		{
			core_state.error = CORE_ERROR_OUT_OF_MEMORY; 
			return NULL; 
		}
		memset(meta, 0, sizeof(UI_Shot_Meta));
		meta->index = shot_id; 
		meta->type = UI_ITEM_SHOT;
		meta->view_center_x = 0.5; 
		meta->view_center_y = 0.5; 
		meta->view_zoom = -1;
		shots.data[shot_id].ui = (void *)meta;
	}

	return (UI_Shot_Meta *)shots.data[shot_id].ui;
}

// create new GUI item structure for vertices
/*UI_Meta * ui_check_vertex_meta(size_t vertex_id) 
{
	// check if this vertex already has meta structure
	ASSERT(validate_vertex(vertex_id), "invalid vertex supplied when checking for meta structure");
	if (!vertices.data[vertex_id].ui)
	{
		vertices.data[vertex_id].ui = ui_create_meta(UI_ITEM_VERTEX, vertex_id);
	}

	return (UI_Meta *)vertices.data[vertex_id].ui;
}*/

// initialize Agar GUI library
bool ui_agar_initialization()
{
	const int width = 1160, height = 800; 

	gui_initialize();
	gui_helper_initialize_sdl(width, height);
	gui_helper_initialize_opengl();
	gui_helper_opengl_adjust_size(width, height);
	SDL_WM_SetCaption("insight3d", NULL);
	gui_set_size(width, height);

	return true; 

	// T
	// initialize 
	/*if (AG_InitCore("insight3d - opensource image based 3d modeling software", 0) == -1)
	{
		fprintf(stderr, "%s\n", AG_GetError());
		core_state.error = CORE_ERROR_GUI_INITIALIZATION;
		return false;
	}

	if (AG_InitVideo(800, 600, 32, AG_VIDEO_OPENGL_OR_SDL | AG_VIDEO_RESIZABLE) == -1) 
	{
		fprintf(stderr, "%s\n", AG_GetError());
		core_state.error = CORE_ERROR_GUI_INITIALIZATION;
		return false;
	}

	AG_SetRefreshRate(30);
	// agColors[WINDOW_BG_COLOR] = sdl_map_rgb_vector(agVideoFmt, UI_STYLE_BACKGROUND);

	return true; // ui_icons_initialize();*/
}

// OpenGL settings 
bool ui_opengl_initialization()
{
	glShadeModel(GL_SMOOTH);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_POLYGON_SMOOTH);
	glDisable(GL_POINT_SMOOTH);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	// glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	return true; 
}

// create dialogs for opening and saving files 
bool ui_create_file_dialogs()
{
	// note deprecated
	return true;
}

// create additional dialogs
bool ui_create_dialogs() 
{
	return true;
}

// create main window with glview and friends
bool ui_create_main_window()
{
	// define the user interface
	ui_state.root_panel = gui_get_root_panel();
	ui_state.top = gui_new_panel(ui_state.root_panel, NULL, gui_top_hfill);
	ui_state.side = gui_new_panel(ui_state.root_panel, ui_state.top, gui_below_left_vfill);
	ui_state.gl = gui_new_panel(ui_state.root_panel, ui_state.side, gui_below_on_the_right_fill);
	gui_set_height(ui_state.top, 30);
	gui_set_width(ui_state.side, 250);
	gui_set_style(ui_state.top, gui_style_background); 
	gui_make_menu(ui_state.top);
	gui_make_glview(ui_state.gl, ui_event_redraw);

	// left tab
	ui_state.side_top = gui_new_panel(ui_state.side, NULL, gui_top_hfill);
	ui_state.side_bottom = gui_new_panel(ui_state.side, ui_state.side_top, gui_below_fill);
	gui_set_height(ui_state.side_top, 180);
	gui_set_style(ui_state.side_top, gui_style_background);
	gui_set_style(ui_state.side_bottom, gui_style_background);
	ui_state.side_top_last = gui_new_label(ui_state.side_top, NULL, "Tools you can use:");

	// tabs
	ui_state.tabs = gui_new_tabs(ui_state.side_bottom, NULL, gui_fill);

	// register events 
	GUI_EVENT_HANDLER(ui_state.gl, mousemove) = ui_event_agar_motion; 
	GUI_EVENT_HANDLER(ui_state.gl, mousedown) = ui_event_agar_button_down; 
	// note we're now sending all mousebuttonup events to the our routine, 
	// shouldn't we change the name (remove _agar_)?
	// GUI_EVENT_HANDLER(ui_state.gl, mouseup) = ui_event_agar_button_up;
	GUI_EVENT_HANDLER(ui_state.gl, unfocus) = ui_event_mouse_out;

	// T
	// create main window
	/*AG_Window * & win = ui_state.window;
	win = AG_WindowNew(AG_WINDOW_PLAIN);
	AG_WindowSetPadding(win, 0, 0, 0, 0);
	AG_WindowSetSpacing(win, 0);
	AG_WindowSetGeometry(win, 0, 22, agView->w, agView->h - 22);
	AG_WindowSetCaption(win, "Main window");
	AG_WindowSetCloseAction(win, AG_WINDOW_HIDE);

	// divide it into two parts
	ui_state.pane = AG_PaneNew(win, AG_PANE_HORIZ, AG_PANE_EXPAND);
	ui_state.div_tools = ui_state.pane->div[0];
	ui_state.div_glview = ui_state.pane->div[1];

	// divide the left part into yet another pair
	ui_state.pane_tools = AG_PaneNew(ui_state.div_tools, AG_PANE_VERT, AG_PANE_EXPAND);
	ui_state.div_selections = ui_state.pane_tools->div[0];
	ui_state.div_tabs = ui_state.pane_tools->div[1];
	ui_state.vbox_selections = AG_VBoxNew(ui_state.div_selections, AG_VBOX_VFILL | AG_VBOX_HFILL);
	ui_state.vbox_tabs = AG_VBoxNew(ui_state.div_tabs, AG_VBOX_VFILL | AG_VBOX_HFILL | AG_VBOX_HOMOGENOUS);

	// * selections *

	// toolbar contains buttons which activate tools 
	ui_state.div_toolbar = AG_VBoxNew(ui_state.vbox_selections, AG_VBOX_HFILL | AG_VBOX_HOMOGENOUS);

	// list (actually a table) enables us to switch between views
	ui_state.list = AG_TableNew(ui_state.vbox_selections, AG_TABLE_EXPAND | AG_TABLE_MULTI);
	AG_TableAddCol(ui_state.list, "Filename", NULL, NULL);
	AG_TableAddCol(ui_state.list, "Calib", "<***>", NULL);

	// tabs contain settings for certain tools 
	ui_state.tabs = AG_NotebookNew(ui_state.vbox_tabs, AG_NOTEBOOK_EXPAND);

	// left panel 
	// ui_treeview_initialize();
	// ui_state.treeview = AG_TlistNewPolled(vbox, 0, ui_treeview_poll, NULL);
	// AG_TlistSetChangedFn(ui_state.treeview, ui_treeview_changed, NULL);
	// AG_TlistSizeHint(ui_state.treeview, "<88888 8888888>", 16);
	
	// right panel 
	ui_state.glview = AG_GLViewNew(ui_state.div_glview, AG_GLVIEW_EXPAND);
	unsigned int glview_flags = AGWIDGET(ui_state.glview)->flags;
	// glview_flags = glview_flags & ~AG_WIDGET_FOCUSABLE;
	glview_flags |= AG_WIDGET_UNFOCUSED_MOTION
	AGWIDGET(ui_state.glview)->flags = glview_flags;
	AG_GLViewDrawFn(ui_state.glview, ui_event_agar_redraw, NULL);
	AG_GLViewMotionFn(ui_state.glview, ui_event_agar_motion, NULL); 
	AG_GLViewButtondownFn(ui_state.glview, ui_event_agar_button_down, NULL);
	AG_GLViewButtonupFn(ui_state.glview, ui_event_agar_button_up, NULL);

	// add handles for processing unprocessed keydown events
	AG_SetEvent(ui_state.glview, "window-keydown", ui_event_key_down, NULL);
	AG_SetEvent(ui_state.glview, "window-keyup", ui_event_key_up, NULL);*/

	return true;
}

// create main application menu
bool ui_create_menu()
{
	// T
	// pointers to menu structures 
	GUI_Panel * & application_menu = tools_state.application_menu; 

	// main menu container 
	application_menu = ui_state.top;

	/*
	// submenu "edit"
	menu_item = AG_MenuNode(application_menu->root, "Edit", NULL);
	AG_MenuItem * submenu_item = AG_MenuAction(menu_item, "Mode", NULL, NULL, NULL);
	AG_MenuAction(submenu_item, "Overview mode", NULL, ui_event_menu_mode_overview, NULL);
	AG_MenuAction(submenu_item, "Inspection mode", NULL, ui_event_menu_mode_inspection, NULL);
	AG_MenuAction(submenu_item, "Shot mode", NULL, ui_event_menu_mode_shot, NULL);
	AG_MenuSeparator(menu_item); 
	// AG_MenuAction(menu_item, "Select all", NULL, ui_event_menu_select_all, NULL);
	submenu_item = AG_MenuAction(menu_item, "Select", NULL, NULL, NULL); 
	AG_MenuAction(submenu_item, "Points on current shot", NULL, ui_event_menu_select_points_on_current_shot, NULL);
	AG_MenuAction(submenu_item, "All points", NULL, ui_event_menu_select_all_points, NULL);
	AG_MenuAction(submenu_item, "Points with reconstructed vertices", NULL, ui_event_menu_select_points_with_reconstructed_vertices, NULL);
	AG_MenuAction(submenu_item, "Points sharing vertex with currently selected points", NULL, ui_event_menu_select_points_sharing_vertex, NULL);
	AG_MenuAction(menu_item, "Deselect all", NULL, ui_event_menu_deselect_all, NULL); 
	AG_MenuSeparator(menu_item);
	AG_MenuAction(menu_item, "Erase selected points", NULL, ui_event_menu_erase_selected_points, NULL);
	AG_MenuAction(menu_item, "Erase current polygon", NULL, ui_event_menu_erase_current_polygon, NULL);
	submenu_item = AG_MenuAction(menu_item, "Erase (more)", NULL, NULL, NULL); 
	AG_MenuAction(submenu_item, "All vertices and points", NULL, ui_event_menu_erase_all_points_and_vertices, NULL);
	// AG_MenuSeparator(menu_item); 
	// AG_MenuAction(submenu_item, "Erase all", NULL, ui_event_menu_erase_all, NULL);

	// submenu "reconstruction"
	/*menu_item = AG_MenuNode(application_menu->root, "Reconstruction", NULL);
	AG_MenuAction(menu_item, "Triangulate vertices", NULL, ui_event_menu_triangulation, NULL);
	AG_MenuAction(menu_item, "Camera resection", NULL, ui_event_menu_resection, NULL);
	submenu_item = AG_MenuAction(menu_item, "Camera resection (more)", NULL, NULL, NULL); 
	AG_MenuAction(submenu_item, "All uncalibrated cameras satisfying lattice test", NULL, ui_event_menu_resection_all_lattice, NULL);
	AG_MenuAction(submenu_item, "All uncalibrated cameras with enough known points", NULL, ui_event_menu_resection_all_enough, NULL);
	AG_MenuAction(submenu_item, "All uncalibrated cameras", NULL, ui_event_menu_resection_all_uncalibrated, NULL);
	AG_MenuAction(submenu_item, "All cameras", NULL, ui_event_menu_resection_all, NULL);

	// submenu "modelling"
	menu_item = AG_MenuNode(application_menu->root, "Modelling", NULL); 
	AG_MenuAction(menu_item, "Define coordinate frame", NULL, ui_event_menu_coordinate_frame, NULL);
	AG_MenuAction(menu_item, "Find major plane", NULL, NULL, NULL);
	AG_MenuAction(menu_item, "Deform image into pinhole camera", NULL, ui_event_menu_pinhole_deform, NULL);

	// submenu "windows"
	menu_item = AG_MenuNode(application_menu->root, "Windows", NULL);
	AG_MenuAction(menu_item, "OpenGL layer", NULL, ui_event_menu_opengl_layer, NULL);*/

	return true;
}

// create tools 
bool ui_register_tools() 
{
	// create root elements for tools 
	tool_register_menu_void("Main menu|"); 

	// menu 
	tool_file_create();
	tool_edit_create();

	// toolbar tools 
	tool_selection_create();
	tool_points_create();
	tool_polygons_create();

	// note move this somewhere
	// add controls to change current image
	ui_state.side_top_last = gui_new_label(ui_state.side_top, ui_state.side_top_last, "Browse pictures:"); 
	ui_state.side_top_last = gui_new_button(ui_state.side_top, ui_state.side_top_last, "Previous", ui_prev_shot);
	ui_state.side_top_last = gui_new_button(ui_state.side_top, ui_state.side_top_last, "Next", ui_next_shot);

	// other tols 
	tool_matching_create();
	tool_calibration_create();
	tool_resection_create();
	tool_triangulation_create();
	tool_coordinates_create();
	tool_image_create();

	// finalize tools creation 
	tool_finalize();

	return true; 
}

// finalize GUI creation 
bool ui_done()
{
	// go through all parameters of all tools and fill in the defaults (for enum parameters) 
	for (size_t i = 0; i < tools_state.count; i++) 
	{
		for ALL(tools_state.tools[i].parameters, j) 
		{
			const Tool_Parameter * const parameter = tools_state.tools[i].parameters.data + j; 

			// we care only about enum parameters 
			if (parameter->type == TOOL_PARAMETER_ENUM)
			{
				int new_select = 1; 
				int * sel = &new_select;
				// AG_PostEvent(NULL, parameter->enum_widget, "radio-changed", "i", 1);
			}
		}
	}

	// activate first tool 
	if (tools_state.tools[0].begin) 
	{
		tools_state.tools[0].begin();
	}

	return true;
}

// create gui structures and initialize 
bool ui_create()
{
	return 
		ui_agar_initialization() &&
		ui_opengl_initialization() && 
		ui_create_file_dialogs() &&
		ui_create_dialogs() &&
		ui_create_main_window() && 
		ui_create_menu() && 
		ui_context_initialize() &&
		ui_register_tools() &&
		ui_done()
	;
}

/*// get viewport width and height in shot pixels 
// note unused and not tested
void ui_get_viewport_width_in_shot_coordinates(double & width, double & height) 
{
	double x1, y1, x2, y2; 

	ui_convert_xy_from_screen_to_shot(0, 0, x1, y1); 
	ui_convert_xy_from_screen_to_shot(gui_get_width(ui_state.gl) - 1, gui_get_height(ui_state.gl) - 1, x2, y2); 

	width = x2 - x1; 
	height = y2 - y1;
}*/

// converts screen coordinates to shot coordinate; if no shot is displayed, 
// convert to percentages of screen (aka virtual shot) 
void ui_convert_xy_from_screen_to_shot(Uint16 screen_x, Uint16 screen_y, double & x, double & y)
{
	// check if there's shot being displayed 
	if (INDEX_IS_SET(ui_state.current_shot))
	{
		const double
			shot_ratio = shots.data[ui_state.current_shot].width / (double)shots.data[ui_state.current_shot].height,
			window_ratio = gui_get_width(ui_state.gl) / (double)gui_get_height(ui_state.gl);
		UI_Shot_Meta * meta = ui_check_shot_meta(ui_state.current_shot);
		const double zoom_x = meta->view_zoom * window_ratio / shot_ratio; 
		x = meta->view_center_x + (screen_x / (double)gui_get_width(ui_state.gl) - 0.5) * 2 * zoom_x; 
		y = meta->view_center_y + (screen_y / (double)gui_get_height(ui_state.gl) - 0.5) * 2 * meta->view_zoom;
	}
	else
	{
		x = screen_x / (double)gui_get_width(ui_state.gl);
		y = screen_y / (double)gui_get_height(ui_state.gl);
	}
}

// converts shot coordinates to opengl coordinates
void ui_convert_xy_from_shot_to_opengl(const double shot_x, const double shot_y, double & x, double & y)
{
	x = -1 + 2 * shot_x;
	y = 1 - 2 * shot_y;
}

// check if viewport is set
bool ui_viewport_set(const size_t shot_id) 
{
	const UI_Shot_Meta * const meta = ui_check_shot_meta(shot_id);
	return meta->view_zoom >= 0;
}

// release dualview 
void ui_release_dualview() 
{
	if (INDEX_IS_SET(ui_state.dualview))
	{
		// release request for this image 
		// ASSERT(image_loader_nonempty_handle(shots.data[ui_state.dualview].image_loader_request), "dualview image doesn't have nonempty request even though dualview was shown");
		// note this probably shouldn't be an invariant - for example, the user can turn on and off the dualview so quickly, that no request is sent.
		if (image_loader_nonempty_handle(shots.data[ui_state.dualview].image_loader_request))
		{
			image_loader_cancel_request(&shots.data[ui_state.dualview].image_loader_request);
		}

		INDEX_CLEAR(ui_state.dualview);
	}
}

// clears key state 
void ui_clear_key(int key) 
{
	ui_state.key_state[key] = 0;
}

// prepare user interface for deletition of points, vertices or polygons
void ui_prepare_for_deletition(bool points, bool vertices, bool polygons, bool shots, bool calibrations)
{
	ui_empty_selection_list();
	if (points) { INDEX_CLEAR(ui_state.focused_point); }
	if (vertices) { ui_workflow_no_vertex(); }
	if (polygons) { INDEX_CLEAR(ui_state.processed_polygon); }
	if (shots) { INDEX_CLEAR(ui_state.current_shot); }
	if (calibrations) { INDEX_CLEAR(ui_state.current_calibration); }
}
