#ifndef _CLIENT_
#define _CLIENT_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

class Client {
	private:
		int sockfd;
		socklen_t addrlen;
    	struct sockaddr_in server_addr;
    	int status;

	public:
		// deschide o conexiune
		void open_connection(std::string serv_IP, int serv_port, int ip_type, int socket_type, int flag = 0) {
			// creare socket
			sockfd = socket(ip_type, socket_type, flag);
			DIE(sockfd < 0, "create socket fails");

			// completare informatii despre adresa serverului
			memset((char *) &server_addr, 0, sizeof(struct sockaddr_in));
			server_addr.sin_family = ip_type;
		    server_addr.sin_addr.s_addr = inet_addr(serv_IP.c_str());
		    server_addr.sin_port = htons(serv_port);

		    // conectare server
		    status = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
		    DIE(status < 0, "connection fails");
		}

		// inchide o conexiune
		void close_connection() {
			status = close(sockfd);
			DIE(status < 0, "close socket fails");
		}

		// trimite un mesaj catre server
		void send_message(char* message) {
			int sent = 0;
		    int total = strlen(message);

		    do {
		        status = write(sockfd, message + sent, total - sent);
		        DIE(status < 0, "error send message");

		        if (!status) {
		            break;
		        }

		        sent += status;
		    } while (sent < total);
		}

		// primeste un mesaj de la server
		char *receive_message() {
			char *response = (char*)calloc(BUFLEN, sizeof(char));
		    int total = BUFLEN;
		    int received = 0;
		    
		    do {
		        status = read(sockfd, response + received, total - received);
		        DIE(status < 0, "error receive message");

		        if (!status) {
		            break;
		        }

		        received += status;
		    } while (received < total);

		    DIE(received == total, "ERROR storing complete response from socket");

		    return response;
		}
};

#endif