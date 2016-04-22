#include <hooks/hooks.h>
#include "options_to_options.h"
#include "options_to_options_log.h"
using namespace isc::hooks;
using namespace options_to_options;
// "Interesting clients" log file handle definition.
// std::fstream options_to_options::interesting;
extern "C" {
int load(LibraryHandle&) {
	LOG_INFO(options_to_options_logger, OPTIONS_TO_OPTIONS_LOAD).arg("loaded");
	return (0);
}
int unload() {
	LOG_INFO(options_to_options_logger, OPTIONS_TO_OPTIONS_LOAD).arg("unloaded");
	return (0);
}
}
