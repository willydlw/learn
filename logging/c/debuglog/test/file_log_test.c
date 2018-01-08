/** Purpose: test console logging functions */

#include "debuglog.h"



int main(void)
{
	log_init(LOG_FATAL, LOG_FATAL, 1);
	log_trace("test int %d, float %f, char %c, string %s", 1, 1.23, 'A', "Ranger");
	log_debug("debug %d", 99);
	log_info("this is info");
	log_warn("test warn %d", 22);
	log_error("test error %d", 33);
	log_fatal("test fatal %f", 99.876);
	return 0;
}