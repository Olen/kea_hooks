#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include "library_common.h"
#include <string>
#include <sstream>
#include <iostream>
#include <regex>

using namespace isc::dhcp;
using namespace isc::hooks;
using namespace std;
extern "C" {

void pkt4_replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}
// This callout is called at the "pkt4_send" hook.
int pkt4_send(CalloutHandle& handle) {
    try {
        Pkt4Ptr response4_ptr;
        handle.getArgument("response4", response4_ptr);

	string res;

	OptionPtr option82_ptr = response4_ptr->getOption(82);
	OptionPtr option43_ptr = response4_ptr->getOption(43);

	OptionPtr option82_1_ptr = option82_ptr->getOption(1);
	OptionPtr option82_2_ptr = option82_ptr->getOption(2);
	OptionPtr option43_1_ptr = option43_ptr->getOption(1);

	string option82_1_data = option82_1_ptr->toString();
	string option82_2_data = option82_2_ptr->toString();
	string option43_1_data = option43_1_ptr->toString();

	option82_2_data = option82_2_data.substr(19,  string::npos);

        stringstream ss(option82_2_data);
        string token;
        regex e ("[^A-z0-9-]"); 
        while(std::getline(ss, token, ':')) 
        {
                res += strtoul(token.c_str(), NULL, 16);
	}
	// Sanitize string before using it
	res = std::regex_replace (res,e,"_");

	pkt4_replaceAll(option43_1_data, "%OPTION82_2%", res);
	// option43_ptr->setData(option43_1_data);

        // Write the information to the log file.
        interesting << hwaddr << " " << ipaddr << " " << option82_1_data << " " << res << " " << option43_1_data << "\n";
        // ... and to guard against a crash, we'll flush the output stream.
        flush(interesting);
    } catch (const NoSuchCalloutContext&) {
        // No such element in the per-request context with the name "hwaddr".
        // This means that the request was not an interesting, so do nothing
        // and dismiss the exception.
        interesting << "Que Pasa\n";
        flush(interesting);
     }
    return (0);
}


}
