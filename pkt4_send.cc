#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include <dhcp/option_string.h>
#include "library_common.h"
#include <string>
#include <sstream>
#include <iostream>
#include <regex>
#include <vector>
#include <map>


using namespace isc::dhcp;
using namespace isc::hooks;
using namespace std;


extern "C" {

void add4Option(Pkt4Ptr& response, uint8_t opt_code, uint8_t sub_code, std::string& opt_value);
void replace4Option(Pkt4Ptr& response4_ptr, uint8_t opt_code, uint8_t sub_code, string &option_data, string placeholder, string replace_with);
string get4Option(Pkt4Ptr& response4_ptr, uint8_t opt_code, uint8_t sub_code, bool sanitize = 1);

// This callout is called at the "pkt4_send" hook.
int pkt4_send(CalloutHandle& handle) {
    try {
        Pkt4Ptr response4_ptr;
        handle.getArgument("response4", response4_ptr);

	// Format mac-addresses
	string hwaddr;
	string hwaddr_cisco;
	string hwaddr_windows;
	handle.getContext("hwaddr", hwaddr);
        regex colon (":"); 
	hwaddr_windows = regex_replace (hwaddr, colon, "-");
	// There must be a better way to do this...
	hwaddr_cisco = hwaddr;
	hwaddr_cisco.replace(hwaddr_cisco.find(":"), 1, "");
	hwaddr_cisco.replace(hwaddr_cisco.find(":"), 1, ".");
	hwaddr_cisco.replace(hwaddr_cisco.find(":"), 1, "");
	hwaddr_cisco.replace(hwaddr_cisco.find(":"), 1, ".");
	hwaddr_cisco.replace(hwaddr_cisco.find(":"), 1, "");
	

	// hostname 
	string hostname;
	handle.getContext("hostname", hostname);

	// Vendor class id
	string vendor_class_id;
	handle.getContext("vendor_class_id", vendor_class_id);

	// IP address
	string ipaddr = response4_ptr->getYiaddr().toText();
	const char *ipchar = ipaddr.c_str();

	stringstream iptream;
	iptream << hex << ntohl(inet_addr(ipchar));
	string ipaddr_hex = iptream.str();



	// Identificator for variables in kea.conf
	// IE @HWADDR@ 
	string PRE_POST_FIX = "@";

	// First we dont want to define a lot of possible variables, so we use a map
	map<string,string> options_variables;

	// Not sure if map is the best way to do this, though...
	map<int,map<int,bool>> options_in;
	map<int,map<int,bool>> options_out;

	// TODO:
	// Add proper logging, not use the "interesting" file

	// Options we will read
	options_in[82][1] = 1;
	options_in[82][2] = 1;
	options_in[82][6] = 1;

	// Options we might write to
	// These are the options that will be scanned for "variables" or "placeholders". Just add all _potential_ options here, as nothing is done if the placceholder is not present.
	// You can use the pre defined option names 
	// Also notice that options without sub options will use sub option id = 0.  
	options_out[43][1] = 1;
	options_out[43][2] = 1;
	options_out[43][3] = 1;
	options_out[43][4] = 1;
	// TODO: Add more options we can write to (hostname, others?)
	options_out[DHO_BOOT_FILE_NAME][0] = 1;

	// Get the option-values from the dhcp-request
	for ( const auto &opt_i : options_in ) {
		for ( const auto &sub_i : options_in[opt_i.first]) {
			int opt_code = opt_i.first;
			int sub_code = 0;
			// Debug
			interesting << "oc: " << to_string(opt_code) << "\n";
        		flush(interesting);
			if (sub_i.first > 0) {
				sub_code = sub_i.first;
				// Debug
				interesting << "Setting OPTION_" << to_string(opt_code) << "_" << to_string(sub_code) << "\n";
        			flush(interesting);

				options_variables["OPTION_" + to_string(opt_code) + "_" + to_string(sub_code)] = get4Option(response4_ptr, opt_code, sub_code, true);
			}
			else {
				// Debug
				interesting << "Setting OPTION_" << to_string(opt_code) << "\n";
        			flush(interesting);

				options_variables["OPTION_" + to_string(opt_code)] = get4Option(response4_ptr, opt_code, sub_code, true);
			}
		}
	}


	// Also get some other variables
	// Note that all values here are strings so some might have to be casted
	options_variables["HOSTNAME"] = hostname;
	options_variables["HWADDR"] = hwaddr;
	options_variables["IPADDR"] = ipaddr;

	// ... and generate a few variants that might be useful
	options_variables["HWADDR_CISCO"] = hwaddr_cisco;	// 1234.5678.90ab
	options_variables["HWADDR_WINDOWS"] = hwaddr_windows;	// 12-34-56-78-90-ab
	options_variables["IPADDR_HEX"] = ipaddr_hex;		// c39f0a01

	options_variables["VENDOR_CLASS_ID"] = vendor_class_id;

	// Debug
	interesting << "All defined options and variables:\n";
        flush(interesting);
	for ( const auto &ov : options_variables) {
		string var = PRE_POST_FIX + ov.first + PRE_POST_FIX;
		string res = ov.second;
		interesting << "ov: " << var << " = " << res << "\n";
        	flush(interesting);
	}

	// Then we search the out-options for the placeholder
	for ( const auto &opt_o : options_out ) {
		for ( const auto &sub_o : options_out[opt_o.first]) {
			string option_data;
			int opt_code = opt_o.first;
			int sub_code = 0;
			if (sub_o.first > 0) {
				sub_code = sub_o.first;
			}
			// Debug
			interesting << "Getting writable option " << to_string(opt_code) << "." << to_string(sub_code) << "\n";
        		flush(interesting);
			// This must NOT be sanitized, as it might contain the placeholder-identificator which is supposed to be replaced later...
			option_data = get4Option(response4_ptr, opt_code, sub_code, false);

			for ( const auto &ov : options_variables) {
				string var = PRE_POST_FIX + ov.first + PRE_POST_FIX;
				string res = ov.second;

       				if (option_data.find(var) != std::string::npos) {
					// Debug
					interesting << "Replace " << var << " with " << res << " in option " << option_data  << "\n";
        				flush(interesting);
					// If the placeholder is present in the option-data, we replace it with the correct value
					replace4Option(response4_ptr, opt_code, sub_code, option_data, var, res);
				}
			}
		}
	}

	// Debug - we flush the file here in case we forget somewhere else
        flush(interesting);
    } catch (const NoSuchCalloutContext&) {
        interesting << "Que Pasa\n";
        flush(interesting);
    }
    return (0);
}


/// @brief Replace a placeholder in an option before adding it to the outgoing packet
/// 
/// @param response4_ptr IPV4 response packet to update
/// @param opt_code DHCP standard numeric code of the option
/// @param sub_code DHCP standard numeric code of the sub option
/// @param option_data The original data in the option-field (from kea.conf)
/// @param placeholder String to search for in the orignal option value
/// @param replace_with String to replace the placeholder with
void replace4Option(Pkt4Ptr& response4_ptr, uint8_t opt_code, uint8_t sub_code, string &option_data, string placeholder, string replace_with) {
	// Debug
	interesting << "Replacing " << placeholder << " with " << replace_with << " in option " << option_data  << "\n";
        flush(interesting);
	option_data.replace(option_data.find(placeholder), placeholder.size(), replace_with);
	// Debug
	interesting << "New option-value: " << option_data << "\n";
        flush(interesting);
	add4Option(response4_ptr, opt_code, sub_code, option_data);
}

/// @brief Adds/updates are specific IPv4 string option in response packet.
///
/// @param response4_ptr IPV4 response packet to update
/// @param opt_code DHCP standard numeric code of the option
/// @param sub_code DHCP standard numeric code of the sub option
/// @param opt_value String value of the option
void add4Option(Pkt4Ptr& response4_ptr, uint8_t opt_code, uint8_t sub_code, std::string& opt_value) {
	// Debug
	interesting << "Adding option-value: " << opt_value << " in " << to_string(opt_code) << "." << to_string(sub_code) << "\n";
        flush(interesting);

	if (sub_code > 0) {
		OptionPtr main = response4_ptr->getOption(opt_code);
		OptionPtr opt;
		if (main) {
			opt = main->getOption(sub_code);
			main->delOption(sub_code);
			opt.reset(new OptionString(Option::V4, sub_code, opt_value));
			main->addOption(opt);
		}
		else {
			interesting << "No option " << to_string(opt_code) << "found\n";
       			flush(interesting);
			return;
		}
	}
	else {
		OptionPtr opt = response4_ptr->getOption(opt_code);
		if (opt) {
			response4_ptr->delOption(opt_code);
		}
		opt.reset(new OptionString(Option::V4, opt_code, opt_value));
		response4_ptr->addOption(opt);
	}
	interesting << "Option added\n";
       	flush(interesting);
}

/// @brief Gets a string value from an option (or sub option) code
///
/// @param response4_ptr IPV4 response packet to update
/// @param opt_code DHCP standard numeric code of the option
/// @param sub_code DHCP standard numeric code of the sub option
/// @param sanitize Do some simple cleanup of the string before returning it
string get4Option(Pkt4Ptr& response4_ptr, uint8_t opt_code, uint8_t sub_code, bool sanitize) {
	// Debug
	interesting << "Getting option: " << to_string(opt_code) <<  "." << to_string(sub_code) << "\n";
        flush(interesting);
	
	string option_data;
	if (sub_code > 0) {
		OptionPtr opt_ptr = response4_ptr->getOption(opt_code);
		if (opt_ptr) {
			OptionPtr sub_ptr = opt_ptr->getOption(sub_code);
			if (sub_ptr) {
				option_data = sub_ptr->toString();
			}
		}
	}
	else {
		OptionPtr opt_ptr = response4_ptr->getOption(opt_code);
		if (opt_ptr) {
			option_data = opt_ptr->toString();
		}
	}

	// Decode :-separated string of ascii-codes
	// TODO: Find a better way to do this...
       	if (option_data.find("type=") == 0) {
		// Debug
		interesting << "Decoding data: " << option_data << "\n";
        	flush(interesting);
		option_data = option_data.substr(19,  string::npos);
        	stringstream ss(option_data);
        	string token;
		option_data = "";
        	while(getline(ss, token, ':')) 
        	{
        	        option_data += strtoul(token.c_str(), NULL, 16);
		}
	}
	
	if (sanitize > 0) {
		// Sanitize the string if needed
		// Debug
		interesting << "Sanitizing data: " << option_data << "\n";
        	flush(interesting);
        	regex sanitize ("[^A-z0-9-]"); 
		option_data = regex_replace (option_data, sanitize, "_");
	}
	// Debug
	interesting << "Returning data: " << option_data << "\n";
        flush(interesting);
	// Return it	
	return option_data;
}


}

