#include "core/engine.hpp"
#include "core/logger.hpp"

#include <core/event.hpp>
#include <core/input.hpp>
//#include <math/mat4x4.h>
//#include <math/vector3.h>

// TODO: temp hack
//#include <renderer/vulkan_backend.h>

//static lise::mat4x4 view;
//static lise::vec3 cam_pos;
//static lise::vec3 cam_rot;
//static bool view_is_dirty = false;

void on_event(uint16_t event_code, lise::event_context ctx)
{
	switch (event_code)
	{
	case lise::event_codes::ON_KEY_DOWN:
		LDEBUG("Key pressed: %d", ctx.data.u32[0]);
	break;
	case lise::event_codes::ON_KEY_UP:
		LDEBUG("Key released: %d", ctx.data.u32[0]);
	break;
	case lise::event_codes::ON_MOUSE_MOVE:
		LDEBUG("Mouse moved: %d, %d", ctx.data.u32[0], ctx.data.u32[1]);
	break;
	case lise::event_codes::ON_MOUSE_WHEEL_MOVE:
		LDEBUG("Mouse wheel moved: %d", ctx.data.i8[0]);
	break;
	}
}

void on_window_resize(uint16_t event_code, lise::event_context ctx)
{
	LDEBUG("New window size: %d, %d", ctx.data.u32[0], ctx.data.u32[1]);
}

//void camera_yaw(float amount) 
//{
//    cam_rot.y += amount;
//    view_is_dirty = true;
//}
//
//void camera_pitch(float amount) {
//    cam_rot.x += amount;
//
//    // Clamp to avoid Gimball lock.
//    float limit = 89.0f / 180.0f * LPI;
//    cam_rot.x = lise::clamp(cam_rot.x, -limit, limit);
//
//    view_is_dirty = true;
//}

bool game_initialize() 
{
	LFATAL("A test message: %f", 2.72f);
	LERROR("A test message: %f", 2.72f);
	LWARN("A test message: %f", 2.72f);
	LINFO("A test message: %f", 2.72f);
	LDEBUG("A test message: %f", 2.72f);
	LTRACE("A test message: %f", 2.72f);

	lise::event_add_listener(lise::event_codes::ON_KEY_DOWN, on_event);

	lise::event_add_listener(lise::event_codes::ON_KEY_UP, on_event);

	//lise::event_add_listener(lise::event_codes::ON_MOUSE_MOVE, on_mouse_move);

	lise::event_add_listener(lise::event_codes::ON_MOUSE_WHEEL_MOVE, on_event);

	lise::event_add_listener(lise::event_codes::ON_WINDOW_RESIZE, on_window_resize);

//	cam_pos = (lise::vec3) { 0.0f, 0.0f, 10.0f };
//	cam_rot = LVEC3_ZERO;
//
//	view = lise::mat4x4_translation(cam_pos);
//	view = lise::mat4x4_inverse(view);
//
//	lise::vulkan_set_view_matrix_temp(view);
	
    return true;
}

bool game_update(float delta_time) 
{
	static float sum = 0;
	static int count = 0;

	sum += delta_time;
	count++;

	if (sum >= 1)
	{
		LDEBUG("Its been a second, %d game loops have passed. The average frametime was %f", count, sum / count);

		sum = 0;
		count = 0;
	}

//	// temp look
//	if (lise::input_is_key_down(LISE_KEY_LEFT))
//	{
//		camera_yaw(1.0f * delta_time);
//	}
//	if (lise::input_is_key_down(LISE_KEY_RIGHT))
//	{
//		camera_yaw(-1.0f * delta_time);
//	}
//	if (lise::input_is_key_down(LISE_KEY_UP))
//	{
//		camera_pitch(1.0f * delta_time);
//	}
//	if (lise::input_is_key_down(LISE_KEY_DOWN))
//	{
//		camera_pitch(-1.0f * delta_time);
//	}
//
//	// temp movement
//	const float move_speed = 5.0f;
//	lise::vec3 dir = LVEC3_ZERO;
//
//	if (lise::input_is_key_down(LISE_KEY_W))
//	{
//		lise::vec3 forward = lise_mat4x4_forward(view);
//		dir = lise::vec3_add(dir, forward);
//		view_is_dirty = true;
//	}
//	if (lise::input_is_key_down(LISE_KEY_S))
//	{
//		lise::vec3 backward = lise_mat4x4_backward(view);
//		dir = lise::vec3_add(dir, backward);
//		view_is_dirty = true;
//	}
//	if (lise::input_is_key_down(LISE_KEY_A))
//	{
//		lise::vec3 left = lise_mat4x4_left(view);
//		dir = lise::vec3_add(dir, left);
//		view_is_dirty = true;
//	}
//	if (lise::input_is_key_down(LISE_KEY_D))
//	{
//		lise::vec3 right = lise_mat4x4_right(view);
//		dir = lise::vec3_add(dir, right);
//		view_is_dirty = true;
//	}
//
//	if (view_is_dirty)
//	{
//		if (!lise::vec3_compare(dir, LVEC3_ZERO, 0.00002f))
//		{
//			lise::vec3_normalize(&dir);
//
//			cam_pos = lise::vec3_add(cam_pos, lise_vec3_mul_scalar(move_speed, lise_vec3_mul_scalar(delta_time, dir)));
//		}
//
//		lise::mat4x4 translation = lise_mat4x4_translation(cam_pos);
//		lise::mat4x4 rotation = lise_mat4x4_euler_xyz(cam_rot.x, cam_rot.y, cam_rot.z);
//
//		view = lise::mat4x4_mul(rotation, translation);
//		view = lise::mat4x4_inverse(view);
//
//		lise::vulkan_set_view_matrix_temp(view);
//	}

    return true;
}

bool game_render(float delta_time) 
{
    return true;
}

void game_on_resize(uint32_t width, uint32_t height) 
{
}


int main()
{
	lise::engine_create_info engine_create_info = {};

	engine_create_info.window_pos_x = 100;
	engine_create_info.window_pos_y = 100;
	engine_create_info.window_width = 1280;
	engine_create_info.window_height = 720;
	engine_create_info.window_name = "LiSE Testing";
	engine_create_info.entry_points.update = game_update;
	engine_create_info.entry_points.render = game_render;
	engine_create_info.entry_points.initialize = game_initialize;
	engine_create_info.entry_points.on_window_resize = game_on_resize;

	if (!lise::engine_create(engine_create_info))
	{
		LFATAL("Could not create engine.");
		return -1;
	}

	if (!lise::engine_run())
	{
		LFATAL("Application did not shut down engine.");
		return -1;
	}

	return 0;
}
