#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"
int logged_in;

char *parse_response_to_cookie(char *response){
	char *cookie = strstr(response, "connect.sid");
	if(cookie == NULL){
		printf("Cookie not found\n");
		return NULL;
	}
	else{
		char *token = strtok(cookie, ";");
		return token;
	}
}

char *parse_response_to_token(char *response){
	char *token = strstr(response, "token\":\"");
	if(token == NULL){
		return NULL;
	}
	else{
		token += 8;
		char *tok = strtok(token, "\"}");
		return tok;
	}
}

char *get_login_output(char *response){
	char *aux = strstr(response,"error");
	if(aux == NULL){
		return "Login successful";
	}
	else{
		logged_in = 0;
		return aux;
	}
}

char *get_registration_output(char *response){
	char *aux = strstr(response,"error");
	if(aux == NULL){
		return "Registration successful";
	}
	else{
		return aux;
	}
}

char *get_enter_library_output(char *response){
	char *aux = strstr(response, "error");
	if(aux == NULL){
		return NULL;
	}
	else{
		return aux;
	}
}

char *get_getBooks_output(char *response){
	char *response1 = malloc(strlen(response) * sizeof(char));
	strcpy(response1, response);
	char *aux = strstr(response1,"[{");
	if(aux == NULL){
		free(response1);
		aux = strstr(response, "error");
		return aux;
	}
	else{
		return aux;
	}
}
char *get_getBook_output(char *response){
	char *response1 = malloc(strlen(response) * sizeof(char));
	strcpy(response1, response);
	char *aux = strstr(response1,"[{");
	if(aux == NULL){
		free(response1);
		aux = strstr(response, "error");
		return aux;
	}
	else{
		return aux;
	}
}

char *get_addBook_output(char *response){
	char *aux = strstr(response,"{\"error");
	if(aux == NULL){
		return "Book added successful";
	}
	else{
		return aux;
	}
}

char *get_deleteBook_output(char *response){
	char *aux = strstr(response,"{\"error");
	if(aux == NULL){
		return "Book removed successful";
	}
	else{
		return aux;
	}
}

char *get_logout_output(char *response){
	char *aux = strstr(response,"{\"error");
	if(aux == NULL){
		return "Logout successful";
	}
	else{
		return aux;
	}
}

int main() 
{
	char command[50];
	char *cookie = (char*) malloc(250 * sizeof(char));
	char *token = (char*) malloc(500 * sizeof(char));
	logged_in = 0;
	while(1){
		memset(command, 0, sizeof(command));
		printf("Command = ");
		scanf("%s", command);

		if(strcmp(command,"exit") == 0){
			break;
		}

		if(strcmp(command,"login") == 0 || strcmp(command, "register") == 0){
			char username[50], password[50];

			printf("username = ");
			scanf("%s", username);

			printf("password = ");
			scanf("%s", password);
			printf("\n");

			int sockfd = open_connection("3.8.116.10", 8080, 
		                            AF_INET, SOCK_STREAM, 0);

			JSON_Value *root_value = json_value_init_object();
    		JSON_Object *root_object = json_value_get_object(root_value);
    		char *serialized_string = NULL;
    		json_object_set_string(root_object, "username", username);
    		json_object_set_string(root_object, "password", password);
    		serialized_string = json_serialize_to_string_pretty(root_value);

			if(strcmp(command, "login") == 0) {
				if(logged_in == 1){
					printf("Someone else is already logged in\n");
					continue;
				}
				char *message = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
			                            "/api/v1/tema/auth/login", "application/json", 
			                            serialized_string, NULL, 0);
				send_to_server(sockfd, message);
			    char *response = receive_from_server(sockfd);
			    cookie = parse_response_to_cookie(response);
			    logged_in = 1;
			    char *output = get_login_output(response);
			    puts(output);
			    close_connection(sockfd);
			}
			else if(strcmp(command, "register") == 0){
				char *message = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
			                            "/api/v1/tema/auth/register", "application/json", 
			                            serialized_string, NULL, 0);
				send_to_server(sockfd, message);
			    char *response = receive_from_server(sockfd);
			    char *output = get_registration_output(response);
			    puts(output);
			    close_connection(sockfd);
			}
		}
		else if(strcmp(command,"enter_library") == 0){
			int sockfd = open_connection("3.8.116.10", 8080, 
		                            AF_INET, SOCK_STREAM, 0);
			char *message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
			                            "/api/v1/tema/library/access", NULL, cookie);
			send_to_server(sockfd, message);
			char *response = receive_from_server(sockfd);
			token = parse_response_to_token(response);
			if(token == NULL){
				char *output = get_enter_library_output(response);
				puts(output);
			}
			else{
				puts(token);
			}
			printf("\n");
			close_connection(sockfd);
		}
		else if(strcmp(command,"get_books") == 0){
			int sockfd = open_connection("3.8.116.10", 8080, 
		                            AF_INET, SOCK_STREAM, 0);
			char *message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
			                            "/api/v1/tema/library/books", token, cookie);
			send_to_server(sockfd, message);
			char *response = receive_from_server(sockfd);
			char *output = get_getBooks_output(response);
			puts(output);
			close_connection(sockfd);
			printf("\n");
		}
		else if(strcmp(command, "get_book") == 0){
			int id;
			printf("id = ");
			scanf("%d",&id);
			char api[50];
    		snprintf(api,50,"/api/v1/tema/library/books/%d", id);
    		int sockfd = open_connection("3.8.116.10", 8080, 
		                            AF_INET, SOCK_STREAM, 0);
			char *message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
			                            api, token, cookie);
			send_to_server(sockfd, message);
			char *response = receive_from_server(sockfd);
			char *output = get_getBook_output(response);
			puts(output);
			close_connection(sockfd);
			printf("\n");
		}
		else if(strcmp(command,"add_book") == 0) {
			int page_count;
			char title[50], author[50],publisher[50],genre[50];

			printf("title = ");
			scanf("%s",title);

			printf("author = ");
			scanf("%s",author);

			printf("publisher = ");
			scanf("%s",publisher);

			printf("genre = ");
			scanf("%s",genre);

			printf("page_count = ");
			scanf("%d", &page_count);

			JSON_Value *root_value = json_value_init_object();
    		JSON_Object *root_object = json_value_get_object(root_value);
    		char *serialized_string = NULL;
    		json_object_set_string(root_object, "title", title);
    		json_object_set_string(root_object, "author", author);
    		json_object_set_string(root_object, "publisher", publisher);
    		json_object_set_string(root_object, "genre", genre);
    		json_object_set_number(root_object, "page_count", page_count);
    		serialized_string = json_serialize_to_string_pretty(root_value);

    		int sockfd = open_connection("3.8.116.10", 8080, 
		                            AF_INET, SOCK_STREAM, 0);
			char *message = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
			                            "/api/v1/tema/library/books/", "application/json", 
			                            serialized_string, token, 0);
			send_to_server(sockfd, message);
			char *response = receive_from_server(sockfd);
			char *output = get_addBook_output(response);
			puts(output);
			close_connection(sockfd);
			printf("\n");
		}
		else if(strcmp(command,"delete_book") == 0){
			int id;
			printf("id = ");
			scanf("%d",&id);

			char api[50];
    		snprintf(api,50,"/api/v1/tema/library/books/%d",id);

			int sockfd = open_connection("3.8.116.10", 8080, 
		                            AF_INET, SOCK_STREAM, 0);
			char *message = compute_delete_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
			                            api,token);
			send_to_server(sockfd, message);
			char *response = receive_from_server(sockfd);
			char *output = get_deleteBook_output(response);
			puts(output);
			close_connection(sockfd);
			printf("\n");
		}
		else if(strcmp(command,"logout") == 0){
			int sockfd = open_connection("3.8.116.10", 8080, 
		                            AF_INET, SOCK_STREAM, 0);
			char *message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
			                            "/api/v1/tema/auth/logout", token, cookie);
			send_to_server(sockfd, message);
			char *response = receive_from_server(sockfd);
			char *output = get_logout_output(response);
			puts(output);
			close_connection(sockfd);
			logged_in = 0;
			if(strlen(token) > 0){
				token = (char*) malloc(500 * sizeof(char));
			}
			printf("\n");
			continue;
		}
	}

	return 0;
}