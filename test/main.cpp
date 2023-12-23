#include <simple-logger.hpp>

#include "core/engine.hpp"
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
	case lise::EventCodes::ON_KEY_DOWN:
		sl::log_debug("Key pressed: {}", ctx.data.u32[0]);
	break;
	case lise::EventCodes::ON_KEY_UP:
		sl::log_debug("Key released: {}", ctx.data.u32[0]);
	break;
	case lise::EventCodes::ON_MOUSE_MOVE:
		sl::log_debug("Mouse moved: {}, {}", ctx.data.u32[0], ctx.data.u32[1]);
	break;
	case lise::EventCodes::ON_MOUSE_WHEEL_MOVE:
		sl::log_debug("Mouse wheel moved: {}", ctx.data.i8[0]);
	break;
	}
}

void on_window_resize(uint16_t event_code, lise::event_context ctx)
{
	sl::log_debug("New window size: {}, {}", ctx.data.u32[0], ctx.data.u32[1]);
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
	sl::log_fatal("A test message: {}", 2.72f);
	sl::log_error("A test message: {}", 2.72f);
	sl::log_warn("A test message: {}", 2.72f);
	sl::log_info("A test message: {}", 2.72f);
	sl::log_debug("A test message: {}", 2.72f);
	sl::log_trace("A test message: {}", 2.72f);

	lise::event_add_listener(lise::EventCodes::ON_KEY_DOWN, on_event);

	lise::event_add_listener(lise::EventCodes::ON_KEY_UP, on_event);

	//lise::event_add_listener(lise::event_codes::ON_MOUSE_MOVE, on_mouse_move);

	lise::event_add_listener(lise::EventCodes::ON_MOUSE_WHEEL_MOVE, on_event);

	lise::event_add_listener(lise::EventCodes::ON_WINDOW_RESIZE, on_window_resize);

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
		sl::log_debug("Its been a second, {} game loops have passed. The average frametime was {}", count, sum / count);

		sum = 0;
		count = 0;
	}

	// temp look
	if (lise::input_is_key_down(lise::Keys::LEFT))
	{
		camera_yaw(1.0f * delta_time);
	}
	if (lise::input_is_key_down(lise::Keys::RIGHT))
	{
		camera_yaw(-1.0f * delta_time);
	}
	if (lise::input_is_key_down(lise::Keys::UP))
	{
		camera_pitch(1.0f * delta_time);
	}
	if (lise::input_is_key_down(lise::Keys::DOWN))
	{
		camera_pitch(-1.0f * delta_time);
	}

	// temp movement
	const float move_speed = 5.0f;
	lise::vector3f dir = LVEC3_ZERO;

	if (lise::input_is_key_down(lise::Keys::W))
	{
		dir = dir + view.forward();
		view_is_dirty = true;
	}
	if (lise::input_is_key_down(lise::Keys::S))
	{
		dir = dir + view.backward();
		view_is_dirty = true;
	}
	if (lise::input_is_key_down(lise::Keys::A))
	{
		dir = dir + view.left();
		view_is_dirty = true;
	}
	if (lise::input_is_key_down(lise::Keys::D))
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
	auto engine_create_info = lise::EngineCreateInfo {};
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
		sl::log_fatal("Could not create engine.");
		return -1;
	}

	if (!lise::engine_run())
	{
		sl::log_fatal("Application did not shut down engine.");
		return -1;
	}

	return 0;
}
