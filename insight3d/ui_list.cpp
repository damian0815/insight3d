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

#include "ui_list.h"

// update the list of shots
void ui_list_update()
{
	// T
	/*AG_TableBegin(ui_state.list);

	for ALL(shots, i) 
	{
		const Shot * const shot = shots.data + i; 
		
		// checkout pointer to shot metadata 
		UI_Shot_Meta * const meta = ui_check_shot_meta(i); 
		if (INDEX_IS_SET(ui_state.current_calibration))
		{
			meta->list_id = AG_TableAddRow(ui_state.list, "%s:%d", shot->name, (shot->points.count > 0 ? 10 : 0) + (shot->partial_calibration ? 1 : 0));
		}
		else
		{
			meta->list_id = AG_TableAddRow(ui_state.list, "%s:%d", shot->name, (shot->points.count > 0 ? 10 : 0) + (shot->calibrated ? 1 : 0));
		}
	}

	AG_TableEnd(ui_state.list);*/
}

// switch to next shot
void ui_next_shot(GUI_Panel * panel)
{
	if (!INDEX_IS_SET(ui_state.current_shot))
	{
		ui_workflow_default_shot();
	}
	else
	{
		for (size_t shot_id = ui_state.current_shot + 1; shot_id < shots.count; shot_id++) 
		{
			if (validate_shot(shot_id))
			{
				ui_workflow_select_shot(shot_id);
				return; 
			}
		}
	}
}

// switch to previous shot
void ui_prev_shot(GUI_Panel * panel)
{
	// note remove this in release 
	if (!INDEX_IS_SET(ui_state.current_shot))
	{
		ui_workflow_default_shot();
	}
	else
	{
		for (size_t shot_id = ui_state.current_shot; shot_id > 0;) 
		{
			shot_id--;

			if (validate_shot(shot_id))
			{
				ui_workflow_select_shot(shot_id);
				return; 
			}
		}
	}
}
