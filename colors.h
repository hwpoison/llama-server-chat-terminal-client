#ifndef __COLORS__
#define __COLORS__
#include <map>
#include <string>

/* https://en.wikipedia.org/wiki/ANSI_escape_code */

#define ANSI_MG_C "\e[0;35m"
#define ANSI_RED_C "\e[0;31m"
#define ANSI_GRN_C "\e[0;32m"
#define ANSI_CYN_C "\x1b[36m"
#define ANSI_YEL_C "\e[0;33m"

#define ANSI_RED_BC "\e[41m"
#define ANSI_GREEN_BC "\x1B[42m"
#define ANSI_BLUE_BC "\x1b[44m"

#define ANSI_COLOR_RESET "\e[0m"

namespace ANSIColors { // change for var=key instead map
	inline std::map<std::string, std::string> all = {
		{"magenta" ,ANSI_MG_C},
		{"red", ANSI_RED_C},
		{"green", ANSI_GRN_C},
		{"cyn", ANSI_CYN_C},
		{"yellow", ANSI_YEL_C},
		{"green_bc", ANSI_GREEN_BC}
	};

}
#endif