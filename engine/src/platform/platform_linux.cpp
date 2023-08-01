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

#include <simple-logger.hpp>

#include "definitions.hpp"
#include "core/event.hpp"
#include "core/input.hpp"

#include "renderer/vulkan_platform.hpp"

#include <vulkan/vulkan_xcb.h>

namespace lise
{

keys translate_keycode(uint32_t x_keycode);

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

bool platform_init(
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
		sl::log_fatal("Failed to connect to X server via XCB");
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
		sl::log_fatal("An error occurred when flusing the stream: %d", stream_result);
		return false;
	}

	return true;
}

void platform_shutdown()
{
	XAutoRepeatOn(state.display);

	xcb_destroy_window(state.connection, state.window);
}

bool platform_poll_messages()
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

				keys key = translate_keycode(key_sym);

				// Pass to the input subsystem for processing.
				input_process_keys(key, pressed);
			} break;
			case XCB_BUTTON_PRESS:
			case XCB_BUTTON_RELEASE:
			{
				// TODO: Mouse button presses and releases
				xcb_button_press_event_t *mouse_event = (xcb_button_press_event_t *)event;
				bool pressed = event->response_type == XCB_BUTTON_PRESS;
				mouse_buttons mouse_button = mouse_buttons::MAX_BUTTONS;

				switch (mouse_event->detail)
				{
					case XCB_BUTTON_INDEX_1:
						mouse_button = mouse_buttons::LEFT;
						
						break;
					case XCB_BUTTON_INDEX_2:
						mouse_button = mouse_buttons::MIDDLE;

						break;
					case XCB_BUTTON_INDEX_3:
						mouse_button = mouse_buttons::RIGHT;

						break;
				}

				// Pass over to the input subsystem.
				if (mouse_button != mouse_buttons::MAX_BUTTONS)
				{
					input_process_button(mouse_button, pressed);
				}
			} break;
			case XCB_MOTION_NOTIFY:
			{
				// Mouse move
				xcb_motion_notify_event_t *move_event = (xcb_motion_notify_event_t *)event;

				// Pass over to the input subsystem.
				input_process_mouse_move(vector2i { move_event->event_x, move_event->event_y });
			} break;
			case XCB_CONFIGURE_NOTIFY:
			{
				// Window resize. Also triggered by moving the window;
				xcb_configure_notify_event_t *configure_event = (xcb_configure_notify_event_t *)event;

				event_context ctx = {};
				ctx.data.u32[0] = configure_event->width;
				ctx.data.u32[1] = configure_event->height;

				event_fire(event_codes::ON_WINDOW_RESIZE, ctx);
			} break;
			case XCB_CLIENT_MESSAGE:
			{
				cm = (xcb_client_message_event_t*)event;

				// Window close
				if (cm->data.data32[0] == state.wm_delete_win)
				{
					event_fire(event_codes::ON_WINDOW_CLOSE, event_context {});
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

void platform_console_write(const char* message, uint8_t color)
{
	// FATAL,ERROR,WARN,INFO,DEBUG,TRACE
	const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
	printf("\033[%sm%s\033[0m", colour_strings[color], message);
}

void platform_console_write_error(const char* message, uint8_t color)
{
	// FATAL,ERROR,WARN,INFO,DEBUG,TRACE
	const char* colour_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
	printf("\033[%sm%s\033[0m", colour_strings[color], message);
}

double platform_get_absolute_time()
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return now.tv_sec + now.tv_nsec * 0.000000001;
}

void platform_sleep(uint64_t ms)
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

const char** platform_get_required_instance_extensions(uint32_t* out_extension_count)
{
	static const char* required_instance_extensions[] = {
		"VK_KHR_surface", "VK_KHR_xcb_surface"
	};

	*out_extension_count = 2;

	return required_instance_extensions;
}

bool vulkan_platform_create_vulkan_surface(VkInstance instance, VkSurfaceKHR* out_surface)
{
	VkXcbSurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR};
	create_info.connection = state.connection;
	create_info.window = state.window;

	if (vkCreateXcbSurfaceKHR(instance, &create_info, NULL, out_surface) != VK_SUCCESS)
	{
		sl::log_fatal("Vulkan surface creation failed.");
		return false;
	}

	return true;
}

// Key translation
keys translate_keycode(uint32_t x_keycode) {
	switch (x_keycode) {
		case XK_BackSpace:
			return keys::BACKSPACE;
		case XK_Return:
			return keys::ENTER;
		case XK_Tab:
			return keys::TAB;
			//case XK_Shift: return keys::SHIFT;
			//case XK_Control: return keys::CONTROL;

		case XK_Pause:
			return keys::PAUSE;
		case XK_Caps_Lock:
			return keys::CAPITAL;

		case XK_Escape:
			return keys::ESCAPE;

			// Not supported
			// case : return keys::CONVERT;
			// case : return keys::NONCONVERT;
			// case : return keys::ACCEPT;

		case XK_Mode_switch:
			return keys::MODECHANGE;

		case XK_space:
			return keys::SPACE;
		case XK_Prior:
			return keys::PRIOR;
		case XK_Next:
			return keys::NEXT;
		case XK_End:
			return keys::END;
		case XK_Home:
			return keys::HOME;
		case XK_Left:
			return keys::LEFT;
		case XK_Up:
			return keys::UP;
		case XK_Right:
			return keys::RIGHT;
		case XK_Down:
			return keys::DOWN;
		case XK_Select:
			return keys::SELECT;
		case XK_Print:
			return keys::PRINT;
		case XK_Execute:
			return keys::EXECUTE;
		// case XK_snapshot: return keys::SNAPSHOT; // not supported
		case XK_Insert:
			return keys::INSERT;
		case XK_Delete:
			return keys::DELETE;
		case XK_Help:
			return keys::HELP;

		case XK_Meta_L:
			return keys::LWIN;  // TODO: not sure this is right
		case XK_Meta_R:
			return keys::RWIN;
			// case XK_apps: return keys::APPS; // not supported

			// case XK_sleep: return keys::SLEEP; //not supported

		case XK_KP_0:
			return keys::NUMPAD0;
		case XK_KP_1:
			return keys::NUMPAD1;
		case XK_KP_2:
			return keys::NUMPAD2;
		case XK_KP_3:
			return keys::NUMPAD3;
		case XK_KP_4:
			return keys::NUMPAD4;
		case XK_KP_5:
			return keys::NUMPAD5;
		case XK_KP_6:
			return keys::NUMPAD6;
		case XK_KP_7:
			return keys::NUMPAD7;
		case XK_KP_8:
			return keys::NUMPAD8;
		case XK_KP_9:
			return keys::NUMPAD9;
		case XK_multiply:
			return keys::MULTIPLY;
		case XK_KP_Add:
			return keys::ADD;
		case XK_KP_Separator:
			return keys::SEPARATOR;
		case XK_KP_Subtract:
			return keys::SUBTRACT;
		case XK_KP_Decimal:
			return keys::DECIMAL;
		case XK_KP_Divide:
			return keys::DIVIDE;
		case XK_F1:
			return keys::F1;
		case XK_F2:
			return keys::F2;
		case XK_F3:
			return keys::F3;
		case XK_F4:
			return keys::F4;
		case XK_F5:
			return keys::F5;
		case XK_F6:
			return keys::F6;
		case XK_F7:
			return keys::F7;
		case XK_F8:
			return keys::F8;
		case XK_F9:
			return keys::F9;
		case XK_F10:
			return keys::F10;
		case XK_F11:
			return keys::F11;
		case XK_F12:
			return keys::F12;
		case XK_F13:
			return keys::F13;
		case XK_F14:
			return keys::F14;
		case XK_F15:
			return keys::F15;
		case XK_F16:
			return keys::F16;
		case XK_F17:
			return keys::F17;
		case XK_F18:
			return keys::F18;
		case XK_F19:
			return keys::F19;
		case XK_F20:
			return keys::F20;
		case XK_F21:
			return keys::F21;
		case XK_F22:
			return keys::F22;
		case XK_F23:
			return keys::F23;
		case XK_F24:
			return keys::F24;

		case XK_Num_Lock:
			return keys::NUMLOCK;
		case XK_Scroll_Lock:
			return keys::SCROLL;

		case XK_KP_Equal:
			return keys::NUMPAD_EQUAL;

		case XK_Shift_L:
			return keys::LSHIFT;
		case XK_Shift_R:
			return keys::RSHIFT;
		case XK_Control_L:
			return keys::LCONTROL;
		case XK_Control_R:
			return keys::RCONTROL;
		// case XK_Menu: return keys::LMENU;
		case XK_Menu:
			return keys::RMENU;

		case XK_semicolon:
			return keys::SEMICOLON;
		case XK_plus:
			return keys::PLUS;
		case XK_comma:
			return keys::COMMA;
		case XK_minus:
			return keys::MINUS;
		case XK_period:
			return keys::PERIOD;
		case XK_slash:
			return keys::SLASH;
		case XK_grave:
			return keys::GRAVE;

		case XK_a:
		case XK_A:
			return keys::A;
		case XK_b:
		case XK_B:
			return keys::B;
		case XK_c:
		case XK_C:
			return keys::C;
		case XK_d:
		case XK_D:
			return keys::D;
		case XK_e:
		case XK_E:
			return keys::E;
		case XK_f:
		case XK_F:
			return keys::F;
		case XK_g:
		case XK_G:
			return keys::G;
		case XK_h:
		case XK_H:
			return keys::H;
		case XK_i:
		case XK_I:
			return keys::I;
		case XK_j:
		case XK_J:
			return keys::J;
		case XK_k:
		case XK_K:
			return keys::K;
		case XK_l:
		case XK_L:
			return keys::L;
		case XK_m:
		case XK_M:
			return keys::M;
		case XK_n:
		case XK_N:
			return keys::N;
		case XK_o:
		case XK_O:
			return keys::O;
		case XK_p:
		case XK_P:
			return keys::P;
		case XK_q:
		case XK_Q:
			return keys::Q;
		case XK_r:
		case XK_R:
			return keys::R;
		case XK_s:
		case XK_S:
			return keys::S;
		case XK_t:
		case XK_T:
			return keys::T;
		case XK_u:
		case XK_U:
			return keys::U;
		case XK_v:
		case XK_V:
			return keys::V;
		case XK_w:
		case XK_W:
			return keys::W;
		case XK_x:
		case XK_X:
			return keys::X;
		case XK_y:
		case XK_Y:
			return keys::Y;
		case XK_z:
		case XK_Z:
			return keys::Z;

		default:
			return keys::A;
	}
}

}

#endif
