#include <core/logger.h>
#include <platform/platform.h>

int main()
{
	LFATAL("A test message: %f", 3.14f);
    LERROR("A test message: %f", 3.14f);
    LWARN("A test message: %f", 3.14f);
    LINFO("A test message: %f", 3.14f);
    LDEBUG("A test message: %f", 3.14f);
    LTRACE("A test message: %f", 3.14f);

	platform_state state;
	if (platform_init(&state, "LiSE Testing", 100, 100, 800, 600))
	{
		while (1)
		{
			platform_poll_messages(&state);
		}
	}
	platform_shutdown(&state);

	return 0;
}
