#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include "library_common.h"
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
using namespace isc::dhcp;
using namespace isc::hooks;
using namespace std;
extern "C" {


// This callout is called at the "pkt4_receive" hook.
// All we want to do is store the hwaddr and a few other vars so we can use it later
int pkt4_receive(CalloutHandle& handle) {
	// A pointer to the packet is passed to the callout via a "boost" smart
	// pointer. The include file "pkt4.h" typedefs a pointer to the Pkt4
	// object as Pkt4Ptr.  Retrieve a pointer to the object.
	Pkt4Ptr query4_ptr;
	handle.getArgument("query4", query4_ptr);
	interesting << "Received new dhcp-packet\n";
	flush(interesting);

	// Point to the hardware address.
	HWAddrPtr hwaddr_ptr = query4_ptr->getHWAddr();
	string hwaddr = hwaddr_ptr->toText();
	hwaddr = hwaddr.substr(9, string::npos);

	interesting << "Found hwaddr " << hwaddr << "\n";
	flush(interesting);
	handle.setContext("hwaddr", hwaddr);

	// Get hostname from query
	string hostname;
	OptionPtr option12 = query4_ptr->getOption(12);
	if (option12) {
		hostname = option12->toString();
	}
	interesting << "Found hostname " << hostname << "\n";
	flush(interesting);
	handle.setContext("hostname", hostname);

	// Get class_id from query
	string vendor_class_id;
	OptionPtr option60 = query4_ptr->getOption(60);
	if (option60) {
		vendor_class_id = option60->toString();
	}
	interesting << "Found vendor_class_id " << vendor_class_id << "\n";
	flush(interesting);
	handle.setContext("vendor_class_id", vendor_class_id);

    	return (0);
};
}
