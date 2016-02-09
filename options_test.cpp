// #include <dhcp/dhcp4.h>

#include <iostream>
#include <map>
#include <regex>
#include <string>


using namespace std;
// using namespace isc::dhcp;


int o2o_replaceOption(int option_number, int subopt_number, string option_data, string placeholder, string replace_with) 
{
	cout << option_number << "." << subopt_number << " - " << option_data << " -> ";
	option_data.replace(option_data.find(placeholder), placeholder.size(), replace_with);
	cout << "option-data2: " << option_data << "\n";
}

string o2o_getOption(int option_number, int subopt_number, int sanitize = 1) {
	string value;
	// Get option
	if (subopt_number > 0) {

	}
	else {

	}
	if (sanitize > 0) {
		// Sanitize it
	}
	// Return it	
	return value;
}

int main()
{
	// Test
	int DHO_BOOT_FILE_NAME = 67;


	// Identificator for variables in kea.conf
	// IE @HWADDR@ 
	string PRE_POST_FIX = "@";

	// First we define a lot of possible variables
	map<string,string> options_variables;

	// Not sure if map is the best way to do this...
	map<int,map<int,bool>> options_in;
	map<int,map<int,bool>> options_out;


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
	options_out[DHO_BOOT_FILE_NAME][0] = 1;

	// Get the option-values from the dhcp-request
	for ( const auto &opt_i : options_in ) {
		for ( const auto &sub_i : options_in[opt_i.first]) {
			if (sub_i.first > 0) {
				options_variables["OPTION_" + to_string(opt_i.first) + "_" + to_string(sub_i.first)] = "foo" + to_string(opt_i.first) + "." + to_string(sub_i.first); // getOption();
			}
			else {
				options_variables["OPTION_" + to_string(opt_i.first)] = "bar" + to_string(opt_i.first); // getOption();
			}
		}
	}

	// Also get some other variables
	options_variables["HWADDR"] = "hwaddr";
	options_variables["IPADDR"] = "ipaddr";

	// ... and generate a few variants that might be useful
	// Note that all values here are strings
	options_variables["HWADDR_CISCO"] = "hwaddr_cisco";	// 1234.5678.90ab
	options_variables["IPADDR_HEX"] = "ipaddr_hex";		// c39f0a01


	// Then we search the out-options for the identificator
	for ( const auto &opt_o : options_out ) {
		for ( const auto &sub_o : options_out[opt_o.first]) {
			string option_data;
			int option_number = opt_o.first;
			int subopt_number = 0;
			if (sub_o.first > 0) {
				// string option_data = getOption(sub)
				// This must NOT be sanitized, as it might contain the placeholder-identificator which is supposed to be replaced...
				option_data = "test.@OPTION_82_" + to_string(sub_o.first) + "@.conf";
				subopt_number = sub_o.first;
				cout << option_number << "." << subopt_number << "-option-data: " << option_data << "\n";
			}
			else {
				// string option_data = getOption(opt)
				// This must NOT be sanitized, as it might contain the placeholder-identificator which is supposed to be replaced...
				option_data = "test.@HWADDR@.conf";
				cout << option_number << "-option-data: " << option_data << "\n";
			}
			for ( const auto &ov : options_variables) {
				string var = PRE_POST_FIX + ov.first + PRE_POST_FIX;
				string res = ov.second;

       				if (option_data.find(var) != std::string::npos) {
					// If the placeholder is present in the option-data, we replace it with the correct value
					o2o_replaceOption(option_number, subopt_number, option_data, var, res);
					// Then we write the option back to the dhcp-packet
				}
			}
		}
	}
}


