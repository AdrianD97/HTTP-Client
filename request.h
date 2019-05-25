#ifndef _REQUESTS_
#define _REQUESTS_

#include <stdlib.h>
#include <string.h>
#include <string>

class Request {
	private:
		void compute_message(char *message, char *line) {
		    strcat(message, line);
		    strcat(message, "\r\n");
		}

	public:
		// construieste un request de tip GET
		char *compute_get_request(const char *host, const char *url, char *url_params = NULL,
		    const char *auth = NULL, std::vector<std::string> *cookies = NULL) {

		    char *message = (char*)calloc(BUFLEN, sizeof(char));
		    char *line = (char*)calloc(LINELEN, sizeof(char));
		    
		    // Scriem numele metodei, calea, parametri din url (daca exista) si tipul protocolului
		    if (url_params != NULL) {
		        sprintf(line, "GET %s?%s HTTP/1.1", url, url_params);
		    } else {
		        sprintf(line, "GET %s HTTP/1.1", url);
		    }
		    compute_message(message, line);

		    // Adaugam host-ul
		    sprintf(line, "HOST: %s", host);
		    compute_message(message, line);

		    // adugam autorizatia
		    if (auth != NULL) {
		    	strcat(message, "Authorization: Bearer ");
		    	strcat(message, auth);
		    	strcpy(line, "\r\n");
		    	strcat(message, line);
		    }

		   	// adugam cookies
		   	if (cookies != NULL) {
		   		strcpy(line, "\r\n");
		   		int i = 0;
		   		int size = (int)cookies->size();
		   		strcat(message, "Cookie: ");
		   		for (; i < size - 1; ++i) {
		   			strcat(message, cookies->at(i).c_str());
		   			strcat(message, "; ");
		   		}
		   		strcat(message, cookies->at(i).c_str());
		   		strcat(message, line);
		   	}
		    
		    // Adaugam linia de final
		    strcpy(line, "\r\n");
		    strcat(message, line);

		    free(line);
		    return message;
		}

		// construieste un request de tip POST
		char *compute_post_request(const char *host, const char *url, char *form_data, const char* type, 
		    const char *auth = NULL, std::vector<std::string> *cookies = NULL) {
		    char *message = (char*)calloc(BUFLEN, sizeof(char));
		    char *line = (char*)calloc(LINELEN, sizeof(char));
		    
		    // Scriem numele metodei, calea si tipul protocolului
		    sprintf(line, "POST %s HTTP/1.1", url);
		    compute_message(message, line);

		    // Adaugam host-ul
		    sprintf(line, "HOST: %s", host);
		    compute_message(message, line);

		    // adugam autorizatia
		    if (auth != NULL) {
		    	strcat(message, "Authorization: Bearer ");
		    	strcat(message, auth);
		    	strcpy(line, "\r\n");
		    	strcat(message, line);
		    }

		   	// adugam cookies
		   	if (cookies != NULL) {
		   		strcpy(line, "\r\n");
		   		int i = 0;
		   		int size = (int)cookies->size();
		   		strcat(message, "Cookie: ");
		   		for (; i < size - 1; ++i) {
		   			strcat(message, cookies->at(i).c_str());
		   			strcat(message, "; ");
		   		}
		   		strcat(message, cookies->at(i).c_str());
		   		strcat(message, line);
		   	}

		    // adaugam tipul mesajului
		    sprintf(line, "Content-Type: %s", type);
		    compute_message(message, line);

		    // adaugam lungimea mesajului(numarul de octeti)
		    sprintf(line, "Content-Length: %d", (int)strlen(form_data));
		    compute_message(message, line);

			// Adaugam linia de final de antent
		    strcpy(line, "\r\n");
		    strcat(message, line);
			
			// Adaugam data
		    compute_message(message, form_data);

		    free(line);
		    return message;
		}
};

#endif
