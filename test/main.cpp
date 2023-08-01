#include "core/engine.hpp"
#include "core/logger.hpp"

#include <core/event.hpp>
#include <core/input.hpp>
#include <math/mat4x4.hpp>
#include <math/vector3.hpp>

// TODO: temp hack
#include <renderer/vulkan_backend.hpp>

static lise::mat4x4 view;
static lise::vector3f cam_pos;
static lise::vector3f cam_rot;
static bool view_is_dirty = false;

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

void camera_yaw(float amount) 
{
    cam_rot.y += amount;
    view_is_dirty = true;
}

void camera_pitch(float amount) {
    cam_rot.x += amount;

    // Clamp to avoid Gimball lock.
    float limit = 89.0f / 180.0f * LPI;
    cam_rot.x = lise::clamp(cam_rot.x, -limit, limit);

    view_is_dirty = true;
}

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

	cam_pos = (lise::vector3f) { 0.0f, 0.0f, 10.0f };
	cam_rot = LVEC3_ZERO;

	view = lise::mat4x4::translation(cam_pos);
	view = view.inversed();

	lise::vulkan_set_view_matrix_temp(view);
	
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

	// temp look
	if (lise::input_is_key_down(lise::keys::LEFT))
	{
		camera_yaw(1.0f * delta_time);
	}
	if (lise::input_is_key_down(lise::keys::RIGHT))
	{
		camera_yaw(-1.0f * delta_time);
	}
	if (lise::input_is_key_down(lise::keys::UP))
	{
		camera_pitch(1.0f * delta_time);
	}
	if (lise::input_is_key_down(lise::keys::DOWN))
	{
		camera_pitch(-1.0f * delta_time);
	}

	// temp movement
	const float move_speed = 5.0f;
	lise::vector3f dir = LVEC3_ZERO;

	if (lise::input_is_key_down(lise::keys::W))
	{
		dir = dir + view.forward();
		view_is_dirty = true;
	}
	if (lise::input_is_key_down(lise::keys::S))
	{
		dir = dir + view.backward();
		view_is_dirty = true;
	}
	if (lise::input_is_key_down(lise::keys::A))
	{
		dir = dir + view.left();
		view_is_dirty = true;
	}
	if (lise::input_is_key_down(lise::keys::D))
	{
		dir = dir + view.right();
		view_is_dirty = true;
	}

	if (view_is_dirty)
	{
		if (dir != LVEC3_ZERO)
		{
			dir.normalize();
			
			cam_pos = cam_pos + move_speed * delta_time * dir;
		}

		lise::mat4x4 translation = lise::mat4x4::translation(cam_pos);
		lise::mat4x4 rotation = lise::mat4x4::euler_xyz(cam_rot.x, cam_rot.y, cam_rot.z);

		view = rotation * translation;
		view = view.inversed();

		lise::vulkan_set_view_matrix_temp(view);
	}

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
	lise::EngineCreateInfo engine_create_info = {};

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
