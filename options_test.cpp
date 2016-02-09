// #include <dhcp/dhcp4.h>

#include <iostream>
#include <map>
#include <regex>
#include <string>


using namespace std;
// using namespace isc::dhcp;


int replaceOption(int option_number, int subopt_number, string option_data, string placeholder, string replace_with) 
{
	
	cout << option_number << "." << subopt_number << " - " << option_data << " -> ";
	option_data.replace(option_data.find(placeholder), placeholder.size(), replace_with);
	cout << "option-data2: " << option_data << "\n";

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
	// Also notice that options without sub options will use sub option id = 0.  You can use the pre defined option names 
	options_out[43][1] = 1;
	options_out[43][2] = 1;
	options_out[DHO_BOOT_FILE_NAME][0] = 1;

	for ( const auto &opt_i : options_in ) {
		// string opt_i_match;
		for ( const auto &sub_i : options_in[opt_i.first]) {
			// opt_i_match = PRE_POST_FIX + "OPTION_" + to_string(opt_i.first);
			if (sub_i.first > 0) {
				// opt_i_match += "_" + to_string(sub_i.first);
				options_variables["OPTION_" + to_string(opt_i.first) + "_" + to_string(sub_i.first)] = "foo" + to_string(opt_i.first) + "." + to_string(sub_i.first); // getOption();
			}
			else {
				options_variables["OPTION_" + to_string(opt_i.first)] = "bar" + to_string(opt_i.first); // getOption();
			}
			// opt_i_match += PRE_POST_FIX;
		}
	}
	options_variables["HWADDR"] = "hwaddr";
	options_variables["IPADDR"] = "ipaddr";
	options_variables["HWADDR_CISCO"] = "hwaddr_cisco";

	// Then we search the out-options for the identificator
	for ( const auto &opt_o : options_out ) {
		string opt_o_match;
		for ( const auto &sub_o : options_out[opt_o.first]) {
			opt_o_match = "option-" + to_string(opt_o.first);
			string option_data;
			int option_number = opt_o.first;
			int subopt_number = 0;
			if (sub_o.first > 0) {
				// string option_data = getOption(sub)
				option_data = "test.@OPTION_82_" + to_string(sub_o.first) + "@.conf";
				subopt_number = sub_o.first;
				cout << option_number << "." << subopt_number << "-option-data: " << option_data << "\n";
			}
			else {
				// string option_data = getOption(opt)
				option_data = "test.@HWADDR@.conf";
				cout << option_number << "-option-data: " << option_data << "\n";
			}
			for ( const auto &ov : options_variables) {
				// cout << "Key: " << ov.first << " Val: " << ov.second << "\n";
				// regex var;
				// regcomp  ov.first;
				string var = PRE_POST_FIX + ov.first + PRE_POST_FIX;
				// cout << "Var: " << var  << "\n";
				// cout << "Len: " << var.size() << "\n";
				string res = ov.second;
       				// option_data = regex_replace (option_data, var, res);
       				// option_data.at("HWADDR") = res;
       				if (option_data.find(var) != std::string::npos) {
					replaceOption(option_number, subopt_number, option_data, var, res);
					// option_data.replace(option_data.find(var), var.size(), res);
					// cout << "option-data2: " << option_data << "\n";
				}
				// else {
				// 	cout << "Could not find " << ov.first << " in " << option_data << "\n";
				// }
				// s.replace(s.find("$name"), sizeof("Somename")-1, "Somename");

			}
			// if match opt_i_match out.toString() -> setOption()
			// Code here...!
			// opt_o_match += "-" + to_string(sub_o.first);
			// }
			// cout << "in: " << opt_i_match << " out: " << opt_o_match << "\n";
		}
	}

}


