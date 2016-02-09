#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include <dhcp/option_string.h>
// #include <dhcp/option_custom.h>
#include "library_common.h"
#include <string>
#include <sstream>
#include <iostream>
#include <regex>
#include <vector>


using namespace isc::dhcp;
using namespace isc::hooks;
using namespace std;


extern "C" {

void add4Option(Pkt4Ptr& response, uint8_t opt_code, std::string& opt_value, uint8_t sub_code = 0);

// This callout is called at the "pkt4_send" hook.
int pkt4_send(CalloutHandle& handle) {
    try {
        Pkt4Ptr response4_ptr;
        handle.getArgument("response4", response4_ptr);

	string res;

	// Get main options
	// OptionPtr option82_ptr = response4_ptr->getOption(82);
	string option82_1_data = response4_ptr->getOption(82)->getOption(1)->toString();
	string option82_2_data = response4_ptr->getOption(82)->getOption(2)->toString();

	interesting << "1\n";
        flush(interesting);

	/*

	// Get sub options
	OptionPtr option82_1_ptr = option82_ptr->getOption(1);
	OptionPtr option82_2_ptr = option82_ptr->getOption(2);
	string option82_1_data = option82_1_ptr->toText();
	string option82_2_data = option82_2_ptr->toText();
	*/

	OptionPtr option43_ptr = response4_ptr->getOption(43);
	interesting << "2\n";
        flush(interesting);
	if (!option43_ptr) {
		interesting << "END\n";
        	flush(interesting);
		return (0);
	}

	OptionPtr option43_1_ptr = response4_ptr->getOption(43)->getOption(1);
	string option43_1_data = option43_1_ptr->toString();
	interesting << "X\n";
        flush(interesting);


	/* 
	OptionPtr option43_ptr = response4_ptr->getOption(43);
	OptionPtr option43_1_ptr = option43_ptr->getOption(1);

	// Decode options to strings
	string option43_1_data = option43_1_ptr->toText();
	*/ 

	// The string contains 19 bytes of header-data...
	option82_2_data = option82_2_data.substr(19,  string::npos);
	interesting << "3\n";
        flush(interesting);


	// Decode :-separated string of ascii-codes
        stringstream ss(option82_2_data);
        string token;
        while(getline(ss, token, ':')) 
        {
                res += strtoul(token.c_str(), NULL, 16);
	}
	interesting << "4\n";
        flush(interesting);

	// Sanitize string before using it
        regex sanitize ("[^A-z0-9-]"); 
	res = regex_replace (res, sanitize, "_");
	interesting << "5\n";
        flush(interesting);

	// Replace "variable" in original packet data
	// 
	// "option_data": { "data": "foo_bar_%OPTION82_1%_baz"; }
	//
	regex opt82_1 ("@OPTION82_1@");
	regex opt82_2 ("@OPTION82_2@");

	option43_1_data = regex_replace (option43_1_data, opt82_1, res);
	option43_1_data = regex_replace (option43_1_data, opt82_2, res);
	interesting << "6\n";
        flush(interesting);

	// add4Option(response4_ptr, DHO_BOOT_FILE_NAME, opt_value);

	option43_ptr->delOption(1);
	option43_1_ptr.reset(new OptionString(Option::V4, 1, option43_1_data));
	option43_ptr->addOption(option43_1_ptr);

	string option43_1_data2 = option43_1_ptr->toText();
	interesting << "7 " << option43_1_data2 << "\n";
        flush(interesting);
	
	
	response4_ptr->addOption(option43_ptr);

	/*
	vector<char> option43_1_vector(option43_1_data.begin(), option43_1_data.end());
	OptionPtr option43_1_option = Option(4, uint16_t 1, option43_1_vector.begin()+1, option43_1_vector.end()-1);

	// new Option(V4, 1, option43_vector.begin()+1, option43_vector.end()-1)
	
	option43_ptr->addOption(option43_1_option);
	// Option::addOption(option43_ptr, option43_1_option);
	*/
	// TODO: How to actually insert the new string into the packet?
	//
	// option43_ptr->setData(option43_1_data);

        // Write the information to the log file.
        interesting << "8 " << option82_2_data << " " << res << " " << option43_1_data << "\n";
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


/// @brief Adds/updates are specific IPv4 string option in response packet.
///
/// @param response IPV4 response packet to update
/// @param opt_code DHCP standard numeric code of the option
/// @param opt_value String value of the option
void add4Option(Pkt4Ptr& response4_ptr, uint8_t opt_code, std::string& opt_value, uint8_t sub_code = 0) {
	// Remove the option if it exists.
	//

	if (sub_code > 0) {
		OptionPtr main = response4_ptr->getOption(opt_code);
		if (main) {
			OptionPtr opt = opt->getOption(sub_code);
			main->delOption(sub_code);
		}
		else {
			main.reset(OptionString(Option::V4, opt_code, NULL));
			OptionPtr opt = opt->getOption(sub_code);
		}
		opt.reset(new OptionString(Option::V4, sub_code, opt_value));
		main->addOption(opt);
		response4_ptr->addOption(main);
		
	}
	else {
		OptionPtr opt = response4_ptr->getOption(opt_code);
		if (opt) {
			response4_ptr->delOption(opt_code);
		}
		opt.reset(new OptionString(Option::V4, opt_code, opt_value));
		response4_ptr->addOption(opt);
	}

	// Now add the option.
	// response->addOption(opt);
}


}
