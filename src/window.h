#ifndef WINDOW_DEF
#define WINDOW_DEF

#define WINDOW_TITLE "OSB LINUX"

#define WINDOW_INIT_WIDTH 			(1920/2.0)
#define WINDOW_INIT_HEIGHT 			(1080/2.0)

#include <SDL2/SDL_events.h>

void Window_Swap();
void Window_Close();
int Window_GetTicks();
int Window_Open();
void Window_PollEvent(void (*callback)(SDL_Event ev));

#endif