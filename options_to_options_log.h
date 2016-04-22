#ifndef OPTIONS_TO_OPTIONS_LOG
#define OPTIONS_TO_OPTIONS_LOG

#include <log/message_initializer.h>
#include <log/macros.h>
#include <options_to_options_messages.h>

namespace options_to_options {

/// @brief Options to Options Logger
///
/// Define the logger used to log messages.  We could define it in multiple
/// modules, but defining in a single module and linking to it saves time and
/// space.
extern isc::log::Logger options_to_options_logger;

} // end of namespace options_to_options

#endif // OPTIONS_TO_OPTIONS_LOG
