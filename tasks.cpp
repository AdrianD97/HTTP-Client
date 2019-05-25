#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <netdb.h> 

#include "helpers.h"
#include "request.h"
#include "parson.h"
#include "client.h"

class Tasks {
	private:
		const char IP_SERVER[15] = "185.118.200.35";
		const int PORT_SERVER = 8081;
		Client* client = NULL;
		Request* request = NULL;

	private:
		// extrage din mesajul primit de la server cookies
		std::vector<std::string> getCookiesFromResponse(char *response) {
			std::vector<std::string> cookies_lines;
			char *cookie_start = strstr(response, "Set-Cookie");
			char *cookie_end = strstr(response, "; path");
			int dim = cookie_end - cookie_start - strlen("Set-Cookie: ");
			char *cookie_line = (char*)malloc((dim + 1) * sizeof(char));
			memset(cookie_line, 0, dim + 1);
			memcpy(cookie_line, cookie_start + strlen("Set-Cookie: "), dim);

			cookies_lines.push_back(cookie_line);
			free(cookie_line);

			cookie_start = strstr(cookie_end + 1, "Set-Cookie");
			cookie_end = strstr(cookie_end + 1, "; path");
			dim = cookie_end - cookie_start - strlen("Set-Cookie: ");
			cookie_line = (char*)malloc((dim + 1) * sizeof(char));
			memset(cookie_line, 0, dim + 1);
			memcpy(cookie_line, cookie_start + strlen("Set-Cookie: "), dim);

			cookies_lines.push_back(cookie_line);
			free(cookie_line);

			return cookies_lines;
		}

		// obtine adresa IP asociata server-ului identificat prin numele primit ca parametru
		const char *getIP(char *name) {
			int ret;
		    struct addrinfo hints, *res;
		    char buff[INET_ADDRSTRLEN];
		    const char *result;
		    const char *ip;

		    memset(&hints, 0, sizeof(struct addrinfo));
		    hints.ai_family = AF_INET;
		    ret = getaddrinfo(name, NULL, &hints, &res);
		    if (ret < 0) {
		    	return NULL;
		    }

		    struct sockaddr_in *help = (struct sockaddr_in *) res->ai_addr;
    		result = (char*)inet_ntop(res->ai_family, &(help->sin_addr), buff, res->ai_addrlen);
    		ip = (char*)calloc(strlen(result) + 1, sizeof(char));
    		strcpy((char*)ip, (char*)result);

    		freeaddrinfo(res);

			return ip;
		}

		// obtine vremea din Bucuresti
		char *getWeather(JSON_Value *value) {
			JSON_Object *obj;
			if (json_value_get_type(value) != JSONObject) {
				return NULL;
			}
			obj = json_value_get_object(value);
			const char *url = (char*)calloc(LINELEN, sizeof(char));
			const char *method = (char*)calloc(10, sizeof(char));
			char *params = (char*)calloc(LINELEN, sizeof(char));
			strcpy(params, "");

			// url-ul
			strcpy((char*)url, json_value_get_string(json_object_get_value_at(obj, 0)));
			// tipul metodei
			strcpy((char*)method, json_value_get_string(json_object_get_value_at(obj, 2)));

			if (strcmp((char*)method, "GET") != 0) {
				free((char*)url);
				free((char*)method);
				free(params);
				return NULL;
			}

			// parametrii routei
			value = json_object_get_value_at(obj, 1);
			obj = json_value_get_object(value);

			strcat(params, json_object_get_name(obj, 0));
			strcat(params, "=");
			strcat(params, json_value_get_string(json_object_get_value_at(obj, 0)));
			strcat(params, "&");
			strcat(params, json_object_get_name(obj, 1));
			strcat(params, "=");
			strcat(params, json_value_get_string(json_object_get_value_at(obj, 1)));

			const char *help = strchr((char*)url, '/');
			char *message, *response;
			char *hostName = (char*)calloc(help - url + 1, sizeof(char));
			memcpy(hostName, url, help - url);

			const char *IP = getIP(hostName);

			// deschidem o conexiune
			client->open_connection(IP, 80, AF_INET, SOCK_STREAM);

			// creeam request-ul
			message = request->compute_get_request(IP, help, params);

			// trimitem un request catre server
			client->send_message(message);

			// primim un raspuns
			response = client->receive_message();
			char *res = strchr(response, '{');
			char *weather = (char*)calloc(strlen(res) + 1, sizeof(char));
			strcpy(weather, res);

			// inchidem conexiunea
			client->close_connection();

			// eliberam memoria utilizata
			free(message);
			free(response);
			free((char*)IP);
			free((char*)url);
			free((char*)method);
			free(params);
			free(hostName);
			
			return weather;
		}

	public:
		Tasks(Client *client, Request *request) {
			this->client = client;
			this->request = request;
		}

		// rezolva task 1
		Fields* task1() {
			char *message, *response;
			char *route = (char*)calloc(LINELEN, sizeof(char));
			strcpy(route, "/task1/start");

			// deschidem o conexiune
			client->open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM);

			// creeam request-ul
			message = request->compute_get_request(IP_SERVER, route);

			// trimitem un request catre server
			client->send_message(message);

			// primim un raspuns
			response = client->receive_message();

			// inchidem conexiunea
			client->close_connection();

			// parsam raspunsul
			Fields *fields = new Fields;

			// obtinem "prajiturile"
			fields->cookies_lines = getCookiesFromResponse(response);

			// obtinem mesajul json
			char *body = strchr(response, '{');

			if (body == NULL) {
				return NULL;
			}

			JSON_Value *value = json_parse_string(body);
			JSON_Object *obj;
			if (json_value_get_type(value) != JSONObject) {
				return NULL;
			}
			obj = json_value_get_object(value);

			// extragem campurile de interes
			// obtinem url-ul
			fields->url = json_value_get_string(json_object_get_value_at(obj, 1));
			// obtinem tipul metodei
			fields->method = json_value_get_string(json_object_get_value_at(obj, 2));
			// obtinem tipul mesajului
			fields->type = json_value_get_string(json_object_get_value_at(obj, 3));

			// obtinem credentialele
			JSON_Value* credentials = json_object_get_value_at(obj, 4);
			obj = json_value_get_object(credentials);

			fields->data.push_back(std::pair<std::string, std::string>(json_object_get_name(obj, 0), json_value_get_string(json_object_get_value_at(obj, 0))));
			fields->data.push_back(std::pair<std::string, std::string>(json_object_get_name(obj, 1), json_value_get_string(json_object_get_value_at(obj, 1))));

			// eliberam memoria
			json_value_free(value);
			free(message);
			free(response);
			free(route);

			return fields;
		}

		// rezolva task 2
		Fields* task2(Fields *fields) {
			if (fields->method == "POST") {
				char *formData = (char*)calloc(BUFLEN, sizeof(char));
				strcpy(formData, "");
				char *message, *response;
				int size = fields->data.size();
				std::vector<std::pair<std::string, std::string>>::iterator it;

				for (it = fields->data.begin(); size > 1; --size) {
					strcat(formData, it->first.c_str());
					strcat(formData, "=");
					strcat(formData, it->second.c_str());
					++it;
					strcat(formData, "&");
				}
				strcat(formData, it->first.c_str());
				strcat(formData, "=");
				strcat(formData, it->second.c_str());

				// deschidem o conexiune
				client->open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM);

				// creea request-ul
				message = request->compute_post_request(IP_SERVER, fields->url.c_str(), formData, 
					fields->type.c_str(), NULL, &fields->cookies_lines);

				// trimitem un request catre server
				client->send_message(message);

				// primim un raspuns
				response = client->receive_message();

				// inchidem conexiunea
				client->close_connection();
				free(formData);

				// trecem la parsarea mesajului
				Fields *fieldsNew = new Fields;

				// obtinem "prajiturile"
				fieldsNew->cookies_lines = getCookiesFromResponse(response);

				// obtinem mesajul json
				char *body = strchr(response, '{');

				if (body == NULL) {
					return NULL;
				}

				JSON_Value *value = json_parse_string(body);
				JSON_Object *obj;
				if (json_value_get_type(value) != JSONObject) {
					return NULL;
				}
				obj = json_value_get_object(value);

				// extragem campurile de interes
				// obtinem url-ul
				fieldsNew->url = json_value_get_string(json_object_get_value_at(obj, 1));
				// obtinem tipul metodei
				fieldsNew->method = json_value_get_string(json_object_get_value_at(obj, 2));

				// obtinem token-ul
				JSON_Value* data = json_object_get_value_at(obj, 3);
				obj = json_value_get_object(data);

				fieldsNew->data.push_back(std::pair<std::string, std::string>(json_object_get_name(obj, 0), json_value_get_string(json_object_get_value_at(obj, 0))));

				// obtinem id-ul pe care nenea prajiturica ni l-a trmis
				JSON_Value* id = json_object_get_value_at(obj, 1);
				obj = json_value_get_object(id);

				fieldsNew->data.push_back(std::pair<std::string, std::string>(json_object_get_name(obj, 0), json_value_get_string(json_object_get_value_at(obj, 0))));

				// eliberam memoria
				json_value_free(value);
				free(message);
				free(response);

				return fieldsNew;
			}
			
			return NULL;
		}

		// rezolva task 3
		Fields* task3(Fields *fields) {
			if (fields->method == "GET") {
				char *urlParam = (char*)calloc(BUFLEN, sizeof(char));
				strcpy(urlParam, "");
				char *message, *response;

				strcat(urlParam, "raspuns1=omul&");
				strcat(urlParam, "raspuns2=numele&");
				strcat(urlParam, fields->data[1].first.c_str());
				strcat(urlParam, "=");
				strcat(urlParam, fields->data[1].second.c_str());

				// deschidem o conexiune
				client->open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM);

				// creeam request-ul
				message = request->compute_get_request(IP_SERVER, fields->url.c_str(), 
					urlParam, fields->data[0].second.c_str(), &fields->cookies_lines);

				free(urlParam);

				// trimitem un request catre server
				client->send_message(message);

				// primim un raspuns
				response = client->receive_message();

				// inchidem conexiunea
				client->close_connection();

				// trecem la parsarea mesajului
				Fields *fieldsNew = new Fields;

				// obtinem "prajiturile"
				fieldsNew->cookies_lines = getCookiesFromResponse(response);

				// obtinem mesajul json
				char *body = strchr(response, '{');

				if (body == NULL) {
					return NULL;
				}

				JSON_Value *value = json_parse_string(body);
				JSON_Object *obj;
				if (json_value_get_type(value) != JSONObject) {
					return NULL;
				}
				obj = json_value_get_object(value);

				// extragem campurile de interes
				// obtinem url-ul
				fieldsNew->url = json_value_get_string(json_object_get_value_at(obj, 1));
				// obtinem tipul metodei
				fieldsNew->method = json_value_get_string(json_object_get_value_at(obj, 2));

				// salvam token-ul
				fieldsNew->data.push_back(fields->data[0]);

				// eliberam memoria
				json_value_free(value);
				free(message);
				free(response);

				return fieldsNew;
			}

			return NULL;
		}

		// rezolva task 4
		Fields* task4(Fields *fields) {
			if (fields->method == "GET") {
				char *message, *response;

				// deschidem o conexiune
				client->open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM);

				// creeam request-ul
				message = request->compute_get_request(IP_SERVER, fields->url.c_str(), 
					NULL, fields->data[0].second.c_str(), &fields->cookies_lines);

				// trimitem un request catre server
				client->send_message(message);

				// primim un raspuns
				response = client->receive_message();

				// inchidem conexiunea
				client->close_connection();

				// trecem la parsarea mesajului
				Fields *fieldsNew = new Fields;

				// obtinem "prajiturile"
				fieldsNew->cookies_lines = getCookiesFromResponse(response);

				// obtinem mesajul json
				char *body = strchr(response, '{');

				if (body == NULL) {
					return NULL;
				}

				JSON_Value *value = json_parse_string(body);
				JSON_Object *obj;
				if (json_value_get_type(value) != JSONObject) {
					return NULL;
				}
				obj = json_value_get_object(value);

				// extragem campurile de interes
				// obtinem url-ul
				fieldsNew->url = json_value_get_string(json_object_get_value_at(obj, 1));
				// obtinem tipul metodei
				fieldsNew->method = json_value_get_string(json_object_get_value_at(obj, 2));
				// obtinem tipul mesajului
				fieldsNew->type = json_value_get_string(json_object_get_value_at(obj, 3));

				// obtinem vremea
				char *weather = getWeather(json_object_get_value_at(obj, 4));
				if (weather != NULL) {
					fieldsNew->data.push_back(std::pair<std::string, std::string>("weather", weather));
					free(weather);
				}

				// salvam token-ul
				fieldsNew->data.push_back(fields->data[0]);

				// eliberam memoria
				json_value_free(value);
				free(message);
				free(response);

				return fieldsNew;
			}
			return NULL;
		}

		// rezolva task 5
		void task5(Fields* fields) {
			if (fields->method == "POST") {
				char *message, *response;

				// deschidem o conexiune
				client->open_connection(IP_SERVER, PORT_SERVER, AF_INET, SOCK_STREAM);

				// creea request-ul
				message = request->compute_post_request(IP_SERVER, fields->url.c_str(), 
					(char*)fields->data[0].second.c_str(), 
					fields->type.c_str(), fields->data[1].second.c_str(), 
					&fields->cookies_lines);

				// trimitem un request catre server
				client->send_message(message);

				// primim un raspuns
				response = client->receive_message();
				std::cout << response << "\n";

				// inchidem conexiunea
				client->close_connection();

				//eliberam memoria
				free(message);
				free(response);
			}
		}
};

int main() {
	Client* client = new Client();
	Request* request = new Request();
	Tasks *tasks = new Tasks(client, request);
	Fields *fields1, *fields2, *fields3, *fields4;

	fields1 = tasks->task1();
	if (fields1 != NULL) {
		fields2 = tasks->task2(fields1);
		delete fields1;
		if (fields2 != NULL) {
			fields3 = tasks->task3(fields2);
			delete fields2;

			if (fields3 != NULL) {
				fields4 = tasks->task4(fields3);
				delete fields3;

				if (fields4 != NULL) {
					tasks->task5(fields4);
					delete fields4;
				}
			}
		}
	}

	delete request;
	delete client;
	delete tasks;
	
	return 0;
}