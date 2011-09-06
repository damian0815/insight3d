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

#ifndef __TOOL_RESECTION
#define __TOOL_RESECTION

#include "tool_typical_includes.h"
#include "geometry_routines.h"
#include "ui_list.h"
#include "actions.h"
#include "sba/sba.h"

void tool_resection_create();
void tool_resection_current_camera();
void tool_resection_all_uncalibrated();
void tool_resection_all();
void tool_resection_all_enough();
void tool_resection_all_lattice();

#endif

