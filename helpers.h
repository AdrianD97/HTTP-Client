#ifndef _HELPERS_
#define _HELPERS_
#include <string>
#include <vector>

#define BUFLEN 4096
#define LINELEN 300
#define URL_LENGTH 300

// macro de verificarea a erorilor
#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

// faimoasa structura; please, citeste README-UL
struct Fields {
	std::string url;
	std::string method;
	std::string type;
	std::vector<std::pair<std::string, std::string>> data;
    std::vector<std::string> cookies_lines;
};

#endif