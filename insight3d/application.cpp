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

#include "application.h"

double delta_time; // extern

// initialize debuging (at this point simply prints out some info about 
// application data structures)
bool debug_initialize()
{
	/* printf("debug mode initialization\n");
	printf("=========================\n");
	printf("size of Point structure: %d\n", sizeof(Point)); 
	printf("size of Vertex structure: %d\n", sizeof(Vertex)); 
	printf("\n"); */

	return true;
}

// initialize application subsystems 
bool initialization()
{
	// GNU GPL license notification
	printf("insight3d 0.3.2, 2007-2009\n");
	printf("licensed under GNU AGPL 3\n\n");

	// note this crashes in debug on MSVC (2008 EE), for now avoid using strdup
	/*char * s = strdup("ahoy"); 
	free(s);*/

	// test memory allocation
	printf("testing memory allocation ... "); 
	size_t * p = ALLOC(size_t, 100); 
	FREE(p);
	p = ALLOC(size_t, 1); 
	FREE(p);
	printf("ok\n"); // if we're still alive, everything's fine

	// initialize the whole package
	return 
		core_debug_initialize() && 
		debug_initialize() && // todo merge this with core_debug
		core_initialize() &&
		geometry_initialize() && 
		image_loader_initialize(4, 32) &&
		ui_initialize() &&
		visualization_initialize() &&
		ui_create()
	;
}

// main loop 
// #define USE_AGAR_EVENTLOOP 1
#ifdef USE_AGAR_EVENTLOOP
bool main_loop()
{
	AG_EventLoop();
	return true;
}
#else
extern "C" struct ag_objectq agTimeoutObjQ;
bool main_loop()
{
	bool is_active = true;
	Uint32 timestamp1 = SDL_GetTicks(), timestamp2 = 0;

	while (core_state.running)
	{
		timestamp2 = SDL_GetTicks();
		delta_time = timestamp2 - timestamp1;

		// if the window is active, do some stuff
		if (is_active)
		{
			// redraw scene
			gui_calculate_coordinates();
			gui_render();

			// let opencv do some redrawing 
			cvWaitKey(1);

			// switch SDL buffers
			SDL_GL_SwapBuffers();
		}

		SDL_Event event;

		// handle events in queue
		while (SDL_PollEvent(&event))
		{
			if (!gui_resolve_event(&event))
			{
				switch (event.type)
				{
					case SDL_KEYDOWN: 
					{
						ui_state.key_state[event.key.keysym.sym] = 1;
						break;
					}

					case SDL_KEYUP: 
					{
						ui_state.key_state[event.key.keysym.sym] = 0;
						break; 
					}

					case SDL_VIDEORESIZE:
					{
						// resize the screen 
						if (!(gui_context.surface = SDL_SetVideoMode(event.resize.w, event.resize.h, 32, gui_context.video_flags)))
						{
							fprintf(stderr, "[SDL] Could not get a surface after resize: %s\n", SDL_GetError());
							core_state.running = false;
							break;
						}

						gui_helper_initialize_opengl();
						gui_helper_opengl_adjust_size(event.resize.w, event.resize.h);
						gui_set_size(event.resize.w, event.resize.h);

						// release all opengl textures // note we're waiting for SDL 1.3 to do this right
						for (size_t i = 0; i < gui_context.panels_count; i++) 
						{
							gui_caption_discard_opengl_texture(gui_context.panels[i]);
						}

						ui_event_resize();

						break;
					}

					case SDL_QUIT:
					{
						core_state.running = false;
						break;
					}
				}
			}

			if (event.type == SDL_MOUSEBUTTONUP) 
			{
				ui_event_agar_button_up();
			}
		}

		ui_event_update(delta_time);

		timestamp1 = timestamp2; 
	}

	ui_prepare_for_deletition(true, true, true, true, true);
	gui_release();

	return true; 
}
#endif

// deallocate program structures
bool release()
{
	geometry_release();
	image_loader_release();

	return true;
}

// error reporting routine 
bool report_error()
{
	return false;
}
