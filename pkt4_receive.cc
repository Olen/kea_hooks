#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include "options_to_options.h"
#include "options_to_options_log.h"
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

using namespace isc::dhcp;
using namespace isc::log;
using namespace isc::hooks;
using namespace std;
using namespace options_to_options;
extern "C" {


// This callout is called at the "pkt4_receive" hook.
// All we want to do is store the hwaddr and a few other vars so we can use it later
int pkt4_receive(CalloutHandle& handle) {
	// LOG_ERROR(user_chk_logger, USER_CHK_HOOK_LOAD_ERROR).arg(ex.what());
	LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_INIT_HOOK).arg("pkt4_receive");
	// A pointer to the packet is passed to the callout via a "boost" smart
	// pointer. The include file "pkt4.h" typedefs a pointer to the Pkt4
	// object as Pkt4Ptr.  Retrieve a pointer to the object.
	Pkt4Ptr query4_ptr;
	handle.getArgument("query4", query4_ptr);
	LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_RCV).arg("Received new dhcp-packet");

	// Point to the hardware address.
	HWAddrPtr hwaddr_ptr = query4_ptr->getHWAddr();
	string hwaddr = hwaddr_ptr->toText();
	hwaddr = hwaddr.substr(9, string::npos);

	LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_RCV).arg("Found hwaddr " + hwaddr);
	handle.setContext("hwaddr", hwaddr);

	// Get Giaddr
	isc::asiolink::IOAddress giaddr = query4_ptr->getGiaddr();
	LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_RCV).arg("Found giaddr " + giaddr.toText());
	handle.setContext("giaddr", giaddr.toBytes());

	// Get hostname from query
	string hostname;
	OptionPtr option12 = query4_ptr->getOption(12);
	if (option12) {
		hostname = option12->toString();
	}


	LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_RCV).arg("Found hostname " + hostname);
	handle.setContext("hostname", hostname);

	// Get class_id from query
	string vendor_class_id;
	OptionPtr option60 = query4_ptr->getOption(60);
	if (option60) {
		vendor_class_id = option60->toString();
	}
	LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_RCV).arg("Found vendor_class_id " + vendor_class_id);
	handle.setContext("vendor_class_id", vendor_class_id);
    	return (0);
};
}
