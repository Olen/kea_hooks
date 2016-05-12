#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include <dhcp/option_string.h>
#include "options_to_options.h"
#include "options_to_options_log.h"
#include <string>
#include <sstream>
#include <iostream>
#include <regex>
#include <vector>
#include <map>


using namespace isc::dhcp;
using namespace isc::hooks;
using namespace isc::log;
using namespace std;
using namespace options_to_options;


extern "C" {

void add4Option(Pkt4Ptr& response, uint8_t opt_code, uint8_t sub_code, std::string& opt_value);
void replace4Option(Pkt4Ptr& response4_ptr, uint8_t opt_code, uint8_t sub_code, string &option_data, string placeholder, string replace_with);
string get4Option(Pkt4Ptr& response4_ptr, uint8_t opt_code, uint8_t sub_code, bool sanitize = 1);

// This callout is called at the "pkt4_send" hook.
int pkt4_send(CalloutHandle& handle) {
    LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_INIT_HOOK).arg("pkt4_send");
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

	// Giaddr
	// string giaddr;
	vector<uint8_t> giaddr;
	handle.getContext("giaddr", giaddr);

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
	map<int,map<int,string>> options_initial;

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
	options_out[DHO_ROUTERS][0] = 1;

	// Get the option-values from the dhcp-request
	for ( const auto &opt_i : options_in ) {
		for ( const auto &sub_i : options_in[opt_i.first]) {
			int opt_code = opt_i.first;
			int sub_code = 0;
			LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("OC: " + to_string(opt_code));
			if (sub_i.first > 0) {
				sub_code = sub_i.first;
				LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("Setting OPTION_" + to_string(opt_code) + "_" + to_string(sub_code));
				options_variables["OPTION_" + to_string(opt_code) + "_" + to_string(sub_code)] = get4Option(response4_ptr, opt_code, sub_code, true);
			}
			else {
				LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("Setting OPTION_" + to_string(opt_code));
				options_variables["OPTION_" + to_string(opt_code)] = get4Option(response4_ptr, opt_code, sub_code, true);
			}
		}
	}


	// Also get some other variables
	// Note that all values here are strings so some might have to be casted
	options_variables["HOSTNAME"] = hostname;
	options_variables["HWADDR"] = hwaddr;
	options_variables["IPADDR"] = ipaddr;

	if (!giaddr.empty()) {
		string giaddress;
		giaddress.resize(giaddr.size());
		memmove(&giaddress[0], &giaddr[0], giaddr.size());
		options_variables["GIADDR"] = giaddress;
	}

	// ... and generate a few variants that might be useful
	options_variables["HWADDR_CISCO"] = hwaddr_cisco;	// 1234.5678.90ab
	options_variables["HWADDR_WINDOWS"] = hwaddr_windows;	// 12-34-56-78-90-ab
	options_variables["IPADDR_HEX"] = ipaddr_hex;		// c39f0a01

	options_variables["VENDOR_CLASS_ID"] = vendor_class_id;

	// Debug 
	LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("All defined options and variables:");
	for ( const auto &ov : options_variables) {
		string var = PRE_POST_FIX + ov.first + PRE_POST_FIX;
		string res = ov.second;
		LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("OV: " + var + " = " + res);
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
			LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("Getting writable option " + to_string(opt_code) + "." + to_string(sub_code));

			// This must NOT be sanitized, as it might contain the placeholder-identificator which is supposed to be replaced later...
			option_data = get4Option(response4_ptr, opt_code, sub_code, false);
			options_initial[opt_code][sub_code] = option_data;
			if (opt_code == DHO_ROUTERS) {
				// Special handling of ROUTERS if set to 0.0.0.0 -> set to giaddr
				if (option_data == "0.0.0.0") {
					LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("Router = 0.0.0.0 -> Setting router to @GIADDR@");
					option_data = "@GIADDR@";
				}
			}
			for ( const auto &ov : options_variables) {
				string var = PRE_POST_FIX + ov.first + PRE_POST_FIX;
				string res = ov.second;
				LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("Searching for " + var + " in " + option_data);
       				if (option_data.find(var) != std::string::npos) {
					LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("Replace " + var + " with " + res + " in option " + option_data);
					// If the placeholder is present in the option-data, we replace it with the correct value
					replace4Option(response4_ptr, opt_code, sub_code, option_data, var, res);
				}
			}
		}
	}
	handle.setContext("options_initial", options_initial);
    } catch (const NoSuchCalloutContext&) {
	LOG_ERROR(options_to_options_logger, OPTIONS_TO_OPTIONS_PKT_SND).arg("No Callout");
    }
    return (0);
}


// This callout is called at the "buffer4_send" hook.
int buffer4_send(CalloutHandle& handle) {
    LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_INIT_HOOK).arg("buffer4_send");
    try {
        // The only purpose of this hook is to reset the original "reply" back to its initial state 
        // to get ready to process the next request
        Pkt4Ptr response4_ptr;
        handle.getArgument("response4", response4_ptr);

        map<int,map<int,string>> options_initial;

        handle.getContext("options_initial", options_initial);

        for ( const auto &opt_i : options_initial ) {
                for ( const auto &sub_i : options_initial[opt_i.first]) {
                        string init_value = sub_i.second;
			LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_BUF_SND).arg("Resetting opt " + to_string(opt_i.first) + " sub " + to_string(sub_i.first) + " to " + init_value);
                        add4Option(response4_ptr, opt_i.first, sub_i.first, init_value);
                }
        }
    } catch (const NoSuchCalloutContext&) {
	LOG_ERROR(options_to_options_logger, OPTIONS_TO_OPTIONS_BUF_SND).arg("No Callout");
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
	LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("Replacing " + placeholder + " with " + replace_with + " in option " + option_data);

	size_t found;
	found = option_data.find(placeholder);
	while (found != string::npos) {
		option_data.replace(option_data.find(placeholder), placeholder.size(), replace_with);
		found = option_data.find(placeholder);
	}
	LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("New option-value: " + option_data);
	add4Option(response4_ptr, opt_code, sub_code, option_data);
}

/// @brief Adds/updates are specific IPv4 string option in response packet.
///
/// @param response4_ptr IPV4 response packet to update
/// @param opt_code DHCP standard numeric code of the option
/// @param sub_code DHCP standard numeric code of the sub option
/// @param opt_value String value of the option
void add4Option(Pkt4Ptr& response4_ptr, uint8_t opt_code, uint8_t sub_code, std::string& opt_value) {
	LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("Adding option-value: " + opt_value + " in " + to_string(opt_code) + "." + to_string(sub_code));
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
			LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("No option " + to_string(opt_code) + " found");
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
	LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("Option added");
}

/// @brief Gets a string value from an option (or sub option) code
///
/// @param response4_ptr IPV4 response packet to update
/// @param opt_code DHCP standard numeric code of the option
/// @param sub_code DHCP standard numeric code of the sub option
/// @param sanitize Do some simple cleanup of the string before returning it
string get4Option(Pkt4Ptr& response4_ptr, uint8_t opt_code, uint8_t sub_code, bool sanitize) {
	LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("Getting option " + to_string(opt_code) + "." + to_string(sub_code));
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
		LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("Decoding data-1: " + option_data);
		option_data = option_data.substr(19,  string::npos);
		LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("Decoding data-2: " + option_data);
		if (option_data.find(":") != string::npos) {
			stringstream ss(option_data);
			string token;
			option_data = "";
			while(getline(ss, token, ':')) {
				LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("Token: " + token);
				option_data += strtoul(token.c_str(), NULL, 16);
			}
		}
	}
	
	if (sanitize > 0) {
		// Sanitize the string if needed
		LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("Sanitizing data: " + option_data);
        	regex sanitize ("[^A-z0-9-]"); 
		option_data = regex_replace (option_data, sanitize, "_");
	}
	LOG_DEBUG(options_to_options_logger, MIN_DEBUG_LEVEL, OPTIONS_TO_OPTIONS_PKT_SND).arg("Returning data: " + option_data);
	// Return it	
	return option_data;
}


}

