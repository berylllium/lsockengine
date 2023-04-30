#include "platform/platform.hpp"

#ifdef L_ISLINUX

#include <xcb/xcb.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>  // sudo apt-get install libx11-dev
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>  // sudo apt-get install libxkbcommon-x11-dev
#include <sys/time.h>

//#define _POSIX_C_SOURCE 199309L
#if _POSIX_C_SOURCE >= 199309L
#include <time.h>  // nanosleep
#else
#include <unistd.h>  // usleep
#endif

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include "definitions.hpp"
#include "core/logger.hpp"
#include "core/event.hpp"
#include "core/input.hpp"

#include "renderer/vulkan_platform.h"

#include <vulkan/vulkan_xcb.h>

lise_keys translate_keycode(uint32_t x_keycode);

typedef struct internal_state
{
	Display* display;
	xcb_connection_t* connection;
	xcb_window_t window;
	xcb_screen_t* screen;

	xcb_atom_t wm_protocols;
	xcb_atom_t wm_delete_win;
} internal_state;

static internal_state state;

bool lise_platform_init(
	const char* application_name,
	int32_t x, int32_t y,
	int32_t width, int32_t height
)
{
	// Connect to X
	state.display = XOpenDisplay(NULL);

	// Turn off key repeats
	XAutoRepeatOff(state.display);

	// Retrieve connection
	state.connection = XGetXCBConnection(state.display);

	if (xcb_connection_has_error(state.connection))
	{
		LFATAL("Failed to connect to X server via XCB");
		return false;
	}

	// Get data from X server
	const struct xcb_setup_t* setup = xcb_get_setup(state.connection);

	// Loop through screens using iterator
	xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
	int screen_p = 0;
	for (uint32_t s = screen_p; s > 0; s--) {
		xcb_screen_next(&it);
	}

	// After screens have been looped through, assign it.
	state.screen = it.data;

	// Allocate a XID for the window to be created.
	state.window = xcb_generate_id(state.connection);

	// Register event types.
	// XCB_CW_BACK_PIXEL = filling then window bg with a single colour
	// XCB_CW_EVENT_MASK is required.
	uint32_t event_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

	// Listen for keyboard and mouse buttons
	uint32_t event_values = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE |
					   XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
					   XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION |
					   XCB_EVENT_MASK_STRUCTURE_NOTIFY;

	// Values to be sent over XCB (bg colour, events)
	uint32_t value_list[] = {state.screen->black_pixel, event_values};

	// Create the window
	xcb_void_cookie_t cookie = xcb_create_window(
		state.connection,
		XCB_COPY_FROM_PARENT,  // depth
		state.window,
		state.screen->root,			// parent
		x,							  //x
		y,							  //y
		width,						  //width
		height,						 //height
		0,							  // No border
		XCB_WINDOW_CLASS_INPUT_OUTPUT,  //class
		state.screen->root_visual,
		event_mask,
		value_list);

	// Change the title
	xcb_change_property(
		state.connection,
		XCB_PROP_MODE_REPLACE,
		state.window,
		XCB_ATOM_WM_NAME,
		XCB_ATOM_STRING,
		8,  // data should be viewed 8 bits at a time
		strlen(application_name),
		application_name);

	// Tell the server to notify when the window manager
	// attempts to destroy the window.
	xcb_intern_atom_cookie_t wm_delete_cookie = xcb_intern_atom(
		state.connection,
		0,
		strlen("WM_DELETE_WINDOW"),
		"WM_DELETE_WINDOW");
	xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(
		state.connection,
		0,
		strlen("WM_PROTOCOLS"),
		"WM_PROTOCOLS");
	xcb_intern_atom_reply_t* wm_delete_reply = xcb_intern_atom_reply(
		state.connection,
		wm_delete_cookie,
		NULL);
	xcb_intern_atom_reply_t* wm_protocols_reply = xcb_intern_atom_reply(
		state.connection,
		wm_protocols_cookie,
		NULL);
	state.wm_delete_win = wm_delete_reply->atom;
	state.wm_protocols = wm_protocols_reply->atom;

	xcb_change_property(
		state.connection,
		XCB_PROP_MODE_REPLACE,
		state.window,
		wm_protocols_reply->atom,
		4,
		32,
		1,
		&wm_delete_reply->atom);

	// Map the window to the screen
	xcb_map_window(state.connection, state.window);

	// Flush the stream
	int32_t stream_result = xcb_flush(state.connection);
	if (stream_result <= 0) {
		LFATAL("An error occurred when flusing the stream: %d", stream_result);
		return false;
	}

	return true;
}

void lise_platform_shutdown()
{
	XAutoRepeatOn(state.display);

	xcb_destroy_window(state.connection, state.window);
}

bool lise_platform_poll_messages()
{
	xcb_generic_event_t* event;
	xcb_client_message_event_t* cm;

	// Poll for events until null is returned.
	while (event != 0)
	{
		event = xcb_poll_for_event(state.connection);

		if (event == 0)
		{
			break;
		}

		// Input events
		switch (event->response_type & ~0x80)
		{
			case XCB_KEY_PRESS:
			case XCB_KEY_RELEASE:
			{
				// Key press event - xcb_key_press_event_t and xcb_key_release_event_t are the same
				xcb_key_press_event_t *kb_event = (xcb_key_press_event_t *)event;
				bool pressed = event->response_type == XCB_KEY_PRESS;
				xcb_keycode_t code = kb_event->detail;

				KeySym key_sym = XkbKeycodeToKeysym(
					state.display,
					(KeyCode)code,  //event.xkey.keycode,
					0,
					code & ShiftMask ? 1 : 0
				);

				lise_keys key = translate_keycode(key_sym);

				// Pass to the input subsystem for processing.
				lise_input_process_keys(key, pressed);
			} break;
			case XCB_BUTTON_PRESS:
			case XCB_BUTTON_RELEASE:
			{
				// TODO: Mouse button presses and releases
				xcb_button_press_event_t *mouse_event = (xcb_button_press_event_t *)event;
				bool pressed = event->response_type == XCB_BUTTON_PRESS;
				lise_mouse_buttons mouse_button = LISE_MOUSE_MAX_MOUSE_BUTTONS;

				switch (mouse_event->detail)
				{
					case XCB_BUTTON_INDEX_1:
						mouse_button = LISE_MOUSE_LEFT;
						
						break;
					case XCB_BUTTON_INDEX_2:
						mouse_button = LISE_MOUSE_MIDDLE;

						break;
					case XCB_BUTTON_INDEX_3:
						mouse_button = LISE_MOUSE_RIGHT;

						break;
				}

				// Pass over to the input subsystem.
				if (mouse_button != LISE_MOUSE_MAX_MOUSE_BUTTONS)
				{
					lise_input_process_button(mouse_button, pressed);
				}
			} break;
			case XCB_MOTION_NOTIFY:
				// Mouse move
				xcb_motion_notify_event_t *move_event = (xcb_motion_notify_event_t *)event;

				// Pass over to the input subsystem.
				lise_input_process_mouse_move((lise_vec2i) { move_event->event_x, move_event->event_y });
				break;
			case XCB_CONFIGURE_NOTIFY:
			{
				// Window resize. Also triggered by moving the window;
				xcb_configure_notify_event_t *configure_event = (xcb_configure_notify_event_t *)event;

				lise_event_context ctx = {};
				ctx.data.u32[0] = configure_event->width;
				ctx.data.u32[1] = configure_event->height;

				lise_event_fire(LISE_EVENT_ON_WINDOW_RESIZE, ctx);
			} break;
			case XCB_CLIENT_MESSAGE:
			{
				cm = (xcb_client_message_event_t*)event;

				// Window close
				if (cm->data.data32[0] == state.wm_delete_win)
				{
					lise_event_fire(LISE_EVENT_ON_WINDOW_CLOSE, (lise_event_context) {});
				}
			} break;
			default:
				// Something else
				break;
		}

		free(event);
	}

	return true;
}

void lise_platform_console_write(const char* message, uint8_t color)
{
	// FATAL,ERROR,WARN,INFO,DEBUG,TRACE
	const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
	printf("\033[%sm%s\033[0m", colour_strings[color], message);
}

void lise_platform_console_write_error(const char* message, uint8_t color)
{
	// FATAL,ERROR,WARN,INFO,DEBUG,TRACE
	const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
	printf("\033[%sm%s\033[0m", colour_strings[color], message);
}

double lise_platform_get_absolute_time()
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return now.tv_sec + now.tv_nsec * 0.000000001;
}

void lise_platform_sleep(uint64_t ms)
{
#if _POSIX_C_SOURCE >= 199309L
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000 * 1000;
	nanosleep(&ts, 0);
#else
	if (ms >= 1000)
	{
		sleep(ms / 1000);
	}

	usleep((ms % 1000) * 1000);
#endif
}

const char** lise_platform_get_required_instance_extensions(uint32_t* out_extension_count)
{
	static const char* required_instance_extensions[] = {
		"VK_KHR_surface", "VK_KHR_xcb_surface"
	};

	*out_extension_count = 2;

	return required_instance_extensions;
}

bool lise_vulkan_platform_create_vulkan_surface(
	VkInstance instance,
	VkSurfaceKHR* out_surface
)
{
	VkXcbSurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR};
	create_info.connection = state.connection;
	create_info.window = state.window;

	if (vkCreateXcbSurfaceKHR(instance, &create_info, NULL, out_surface) != VK_SUCCESS)
	{
		LFATAL("Vulkan surface creation failed.");
		return false;
	}

	return true;
}

// Key translation
lise_keys translate_keycode(uint32_t x_keycode) {
	switch (x_keycode) {
		case XK_BackSpace:
			return LISE_KEY_BACKSPACE;
		case XK_Return:
			return LISE_KEY_ENTER;
		case XK_Tab:
			return LISE_KEY_TAB;
			//case XK_Shift: return LISE_KEY_SHIFT;
			//case XK_Control: return LISE_KEY_CONTROL;

		case XK_Pause:
			return LISE_KEY_PAUSE;
		case XK_Caps_Lock:
			return LISE_KEY_CAPITAL;

		case XK_Escape:
			return LISE_KEY_ESCAPE;

			// Not supported
			// case : return LISE_KEY_CONVERT;
			// case : return LISE_KEY_NONCONVERT;
			// case : return LISE_KEY_ACCEPT;

		case XK_Mode_switch:
			return LISE_KEY_MODECHANGE;

		case XK_space:
			return LISE_KEY_SPACE;
		case XK_Prior:
			return LISE_KEY_PRIOR;
		case XK_Next:
			return LISE_KEY_NEXT;
		case XK_End:
			return LISE_KEY_END;
		case XK_Home:
			return LISE_KEY_HOME;
		case XK_Left:
			return LISE_KEY_LEFT;
		case XK_Up:
			return LISE_KEY_UP;
		case XK_Right:
			return LISE_KEY_RIGHT;
		case XK_Down:
			return LISE_KEY_DOWN;
		case XK_Select:
			return LISE_KEY_SELECT;
		case XK_Print:
			return LISE_KEY_PRINT;
		case XK_Execute:
			return LISE_KEY_EXECUTE;
		// case XK_snapshot: return LISE_KEY_SNAPSHOT; // not supported
		case XK_Insert:
			return LISE_KEY_INSERT;
		case XK_Delete:
			return LISE_KEY_DELETE;
		case XK_Help:
			return LISE_KEY_HELP;

		case XK_Meta_L:
			return LISE_KEY_LWIN;  // TODO: not sure this is right
		case XK_Meta_R:
			return LISE_KEY_RWIN;
			// case XK_apps: return LISE_KEY_APPS; // not supported

			// case XK_sleep: return LISE_KEY_SLEEP; //not supported

		case XK_KP_0:
			return LISE_KEY_NUMPAD0;
		case XK_KP_1:
			return LISE_KEY_NUMPAD1;
		case XK_KP_2:
			return LISE_KEY_NUMPAD2;
		case XK_KP_3:
			return LISE_KEY_NUMPAD3;
		case XK_KP_4:
			return LISE_KEY_NUMPAD4;
		case XK_KP_5:
			return LISE_KEY_NUMPAD5;
		case XK_KP_6:
			return LISE_KEY_NUMPAD6;
		case XK_KP_7:
			return LISE_KEY_NUMPAD7;
		case XK_KP_8:
			return LISE_KEY_NUMPAD8;
		case XK_KP_9:
			return LISE_KEY_NUMPAD9;
		case XK_multiply:
			return LISE_KEY_MULTIPLY;
		case XK_KP_Add:
			return LISE_KEY_ADD;
		case XK_KP_Separator:
			return LISE_KEY_SEPARATOR;
		case XK_KP_Subtract:
			return LISE_KEY_SUBTRACT;
		case XK_KP_Decimal:
			return LISE_KEY_DECIMAL;
		case XK_KP_Divide:
			return LISE_KEY_DIVIDE;
		case XK_F1:
			return LISE_KEY_F1;
		case XK_F2:
			return LISE_KEY_F2;
		case XK_F3:
			return LISE_KEY_F3;
		case XK_F4:
			return LISE_KEY_F4;
		case XK_F5:
			return LISE_KEY_F5;
		case XK_F6:
			return LISE_KEY_F6;
		case XK_F7:
			return LISE_KEY_F7;
		case XK_F8:
			return LISE_KEY_F8;
		case XK_F9:
			return LISE_KEY_F9;
		case XK_F10:
			return LISE_KEY_F10;
		case XK_F11:
			return LISE_KEY_F11;
		case XK_F12:
			return LISE_KEY_F12;
		case XK_F13:
			return LISE_KEY_F13;
		case XK_F14:
			return LISE_KEY_F14;
		case XK_F15:
			return LISE_KEY_F15;
		case XK_F16:
			return LISE_KEY_F16;
		case XK_F17:
			return LISE_KEY_F17;
		case XK_F18:
			return LISE_KEY_F18;
		case XK_F19:
			return LISE_KEY_F19;
		case XK_F20:
			return LISE_KEY_F20;
		case XK_F21:
			return LISE_KEY_F21;
		case XK_F22:
			return LISE_KEY_F22;
		case XK_F23:
			return LISE_KEY_F23;
		case XK_F24:
			return LISE_KEY_F24;

		case XK_Num_Lock:
			return LISE_KEY_NUMLOCK;
		case XK_Scroll_Lock:
			return LISE_KEY_SCROLL;

		case XK_KP_Equal:
			return LISE_KEY_NUMPAD_EQUAL;

		case XK_Shift_L:
			return LISE_KEY_LSHIFT;
		case XK_Shift_R:
			return LISE_KEY_RSHIFT;
		case XK_Control_L:
			return LISE_KEY_LCONTROL;
		case XK_Control_R:
			return LISE_KEY_RCONTROL;
		// case XK_Menu: return LISE_KEY_LMENU;
		case XK_Menu:
			return LISE_KEY_RMENU;

		case XK_semicolon:
			return LISE_KEY_SEMICOLON;
		case XK_plus:
			return LISE_KEY_PLUS;
		case XK_comma:
			return LISE_KEY_COMMA;
		case XK_minus:
			return LISE_KEY_MINUS;
		case XK_period:
			return LISE_KEY_PERIOD;
		case XK_slash:
			return LISE_KEY_SLASH;
		case XK_grave:
			return LISE_KEY_GRAVE;

		case XK_a:
		case XK_A:
			return LISE_KEY_A;
		case XK_b:
		case XK_B:
			return LISE_KEY_B;
		case XK_c:
		case XK_C:
			return LISE_KEY_C;
		case XK_d:
		case XK_D:
			return LISE_KEY_D;
		case XK_e:
		case XK_E:
			return LISE_KEY_E;
		case XK_f:
		case XK_F:
			return LISE_KEY_F;
		case XK_g:
		case XK_G:
			return LISE_KEY_G;
		case XK_h:
		case XK_H:
			return LISE_KEY_H;
		case XK_i:
		case XK_I:
			return LISE_KEY_I;
		case XK_j:
		case XK_J:
			return LISE_KEY_J;
		case XK_k:
		case XK_K:
			return LISE_KEY_K;
		case XK_l:
		case XK_L:
			return LISE_KEY_L;
		case XK_m:
		case XK_M:
			return LISE_KEY_M;
		case XK_n:
		case XK_N:
			return LISE_KEY_N;
		case XK_o:
		case XK_O:
			return LISE_KEY_O;
		case XK_p:
		case XK_P:
			return LISE_KEY_P;
		case XK_q:
		case XK_Q:
			return LISE_KEY_Q;
		case XK_r:
		case XK_R:
			return LISE_KEY_R;
		case XK_s:
		case XK_S:
			return LISE_KEY_S;
		case XK_t:
		case XK_T:
			return LISE_KEY_T;
		case XK_u:
		case XK_U:
			return LISE_KEY_U;
		case XK_v:
		case XK_V:
			return LISE_KEY_V;
		case XK_w:
		case XK_W:
			return LISE_KEY_W;
		case XK_x:
		case XK_X:
			return LISE_KEY_X;
		case XK_y:
		case XK_Y:
			return LISE_KEY_Y;
		case XK_z:
		case XK_Z:
			return LISE_KEY_Z;

		default:
			return 0;
	}
}

#endif
