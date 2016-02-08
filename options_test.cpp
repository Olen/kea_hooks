#include <iostream>
#include <map>
using namespace std;

int main()
{
	// Test
	int OPTION_BOOT_FILE_NAME = 67;

	string PRE_POST_FIX = "@";

	// Not sure if map is the best way to do this...
	map<int,map<int,bool>> options_in;
	map<int,map<int,bool>> options_out;

	// Options we will read
	options_in[82][1] = 1;
	options_in[82][2] = 1;
	options_in[82][6] = 1;

	// Options we might write to
	options_out[43][1] = 1;
	options_out[43][2] = 1;
	options_out[OPTION_BOOT_FILE_NAME][0] = 1;


	// 
	for ( const auto &opt_i : options_in ) {
		string opt_i_match;
		for ( const auto &sub_i : options_in[opt_i.first]) {
			opt_i_match = PRE_POST_FIX + "OPTION_" + to_string(opt_i.first);
			if (sub_i.first > 0) {
				// getOption();
				opt_i_match += "_" + to_string(sub_i.first);
			}
			opt_i_match += PRE_POST_FIX;

			for ( const auto &opt_o : options_out ) {
				string opt_o_match;
				for ( const auto &sub_o : options_out[opt_o.first]) {
					opt_o_match = "option-" + to_string(opt_o.first);
					if (sub_o.first > 0) {
						// getOption()
						// if match opt_i_match out.toString() -> setOption()
						// Code here...!
						opt_o_match += "-" + to_string(sub_o.first);
					}
					cout << "in: " << opt_i_match << " out: " << opt_o_match << "\n";
				}
			}
		}
	}
}


