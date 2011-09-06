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

#ifndef __EVENTS
#define __EVENTS

#include "application.h"
#include "interface_opengl.h"
#include "geometry_structures.h"
#include "geometry_loader.h"
#include "geometry_export.h"
#include "ui_core.h"
#include "ui_inspection_mode.h"
#include "ui_shot_mode.h"
#include "ui_visualization_helpers.h"
#include "ui_visualization_point.h"
#include "ui_visualization.h"
#include "ui_selection.h"
#include "ui_list.h"
#include "ui_epipolars.h"
#include "tool_selection.h"

void ui_event_redraw();
void ui_event_key_down(GUI_Panel * event);
void ui_event_key_up(GUI_Panel * event);
void ui_event_mouse_button_down(Uint8 button, Uint16 x, Uint16 y);
void ui_event_agar_button_down(GUI_Panel * event);
void ui_event_mouse_move(Uint16 x, Uint16 y);
void ui_event_agar_motion(GUI_Panel * event);
void ui_event_mouse_button_up(Uint8 button, Uint16 x, Uint16 y) ;
void ui_event_agar_button_up();
void ui_event_update(const Uint32 delta_time);
void ui_event_update_shot(const Uint32 delta_time);
void ui_event_resize();
void ui_event_motion(GUI_Panel * event);
void ui_event_mouse_out(GUI_Panel * event);

#endif
