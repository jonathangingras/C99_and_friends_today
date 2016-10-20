#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <signal.h>
#include <sys/wait.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define PORT 1234

int peer_socket, my_socket;

char buffer[2048];

void sigint(int signal) {
  close(peer_socket);
  close(my_socket);
  exit(0);
}

void sigchld(int sig) {
  while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
}

void exit_by(char *msg) {
  fprintf(stderr, "process failed on: %s\n", msg);
  exit(1);
}

int get_content_length(void* output, char *buffer) {
  *((int*)output) = atoi(buffer);
  return 0;
}

int get_expecte_continue(void* output, char *buffer) {
  *((int*)output) = !memcmp("100-continue", buffer, strlen("100-continue"));
  return *((int*)output);
}

size_t find_next(const char* str, char c) {
  size_t off = 0;
  while(str[off] != c && str[off] != EOF) {
    ++off;
  }
  return off;
}

int get_route(void *output, char *buffer) {
  size_t off;
  off = find_next(buffer, ' ');
  if(!off) return -1;
  *((char**)output) = malloc(off + 1);
  (*((char**)output))[0] = 0;
  memcpy((*((char**)output)), buffer, off);
  (*((char**)output))[off] = 0;
  return 0;
}

int resolve_http_header_att(void* output, char *header, size_t header_size, char *header_attr, int(*process)(void *, char *)) {
  int result = 1;
  char buffer[256];
  size_t header_attr_len = strlen(header_attr);

  size_t oldoff = 0, off = 0;
  while(1) {
    if(off >= header_size) break;
    oldoff = off;
    off += find_next(header + off, '\n');
    if(oldoff == off) break;
    ++off;

    if(!memcmp(header_attr, header + oldoff, header_attr_len)) {
      if(!process(output, header + oldoff + header_attr_len)) {
        result = 0;
        break;
      }
    }
  }

  return result;
}

#define SEND(x, str) send(x, str, strlen(str), 0)

int http_method_GET(int peer_socket, char *http_header_data, size_t header_size) {
  char *route;

  if(resolve_http_header_att(&route, http_header_data, header_size, "GET ", &get_route)) {
    fprintf(stderr, "could not find route\n");
  } else {
    printf("route: %s\n", route);
    free(route);
  }

  printf("raw content:\n");
  fwrite(http_header_data, 1, header_size, stdout);
  printf("\n");

  if (time(NULL)%2) SEND(peer_socket,
       "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<html><h1>HELLO GURL</h1></html>"
    );
  else SEND(peer_socket, "HTTP/1.1 200 OK\nContent-Type: application/json\n\n{\"answer\":\"thanks\"}");

  return 0;
}


int http_method_PUT(int peer_socket, char *http_header_data, size_t header_size) {
  SEND(peer_socket, "HTTP/1.1 418 I'm a teapot\nContent-Type: application/json\n\n{\"answer\":\"cup of tea\"}");
  return 0;
}


int http_method_POST(int peer_socket, char *http_header_data, size_t header_size) {
  char *content_data;
  char buffer[2048];
  int needed_len = 0, received_len = 0, increment = 0;

  if(resolve_http_header_att(&needed_len, http_header_data, header_size, "Content-Length: ", &get_content_length)) {
    fprintf(stderr, "could not find content length\n");
    return -1;
  }

  int need_continue = !resolve_http_header_att(&needed_len, http_header_data, header_size, "Expect: ", &get_expecte_continue);

  printf("received (content length: %d):\n---\n%s\n---\n", needed_len, http_header_data);

  if(needed_len > 100000) {
    fprintf(stderr, "content length is too long\n");
    return -1;
  }

  content_data = malloc(needed_len);
  if(!content_data) {
    printf("could not allocate data for buffer\n");
    return -1;
  }

  while(need_continue && received_len < needed_len) {
    increment = recv(peer_socket, buffer, 2048, 0);

    if(increment <= 0) {
      printf("if: %d\n", received_len);
    } else {
      memcpy(content_data + received_len, buffer, increment);
      received_len += increment;
    }

    if(received_len == needed_len) {
      break;
    }
  }

  content_data[received_len] = 0;

  if(received_len) {
    printf("content:\n");
    fwrite(content_data, 1, received_len, stdout);
    printf("\n\n");
  }

  SEND(peer_socket, "HTTP/1.1 200 OK\n");

  free(content_data);
  return 0;
}

typedef int (*http_method_t)(int peer_socket, char *http_header_data, size_t header_size);

int resolve_http_method(http_method_t *output, char *header, size_t header_size) {
  int result = 1;

  #define case_http_method(buf, method)        \
    if(header_size >= strlen(#method) && !memcmp(buf, #method, strlen(#method))) { \
    result = 0;                                \
    *output = &http_method_##method;           \
  }

  case_http_method(header, GET);
  case_http_method(header, POST);
  case_http_method(header, PUT);

  return result;
}

char *resolve_hostname(struct sockaddr_in *addr) {
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_in *h;
  int rv;
  char *hostname = NULL;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_CANONNAME;

  if ( (rv = getaddrinfo(NULL, "http" , &hints , &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return NULL;
  }

  for(p = servinfo; p != NULL; p = p->ai_next) {
    h = (struct sockaddr_in *) p->ai_addr;

    if(h->sin_addr.s_addr == addr->sin_addr.s_addr) {
      hostname = malloc(strlen(p->ai_canonname) + 1);
      hostname[0] = 0;
      strcpy(hostname, p->ai_canonname);
      break;
    }
  }

  freeaddrinfo(servinfo);
  return hostname;
}

int server(int argc, char **argv) {
  struct sockaddr_in my_address_, peer_address_;
  struct sockaddr *my_address = &my_address_, *peer_address = &peer_address_;
  socklen_t my_address_length = sizeof(struct sockaddr_in), peer_address_length = sizeof(struct sockaddr_in);
  char http_header_data[2048];
  size_t header_length;
  int my_socket, peer_socket;

  //init my address
  memset(&my_address_, 0, sizeof(my_address_));
  my_address_.sin_family = AF_INET;
  my_address_.sin_addr.s_addr = htonl(INADDR_ANY);
  my_address_.sin_port = htons(PORT);

  //create socket
  if((my_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    exit_by("socket");
  }

  //bind address to socket
  if(bind(my_socket, my_address, sizeof(struct sockaddr)) == -1) {
    exit_by("bind");
  }

  //wait for a connection
  if(listen(my_socket, 5) == -1) {
    exit_by("listen");
  }

  while(1) {
    memset(http_header_data, 0, 2048);

    printf("waiting connection\n\n");

    if((peer_socket = accept(my_socket, peer_address, &peer_address_length)) == -1) {
      exit_by("accept");
    }
    printf("Incoming connection from %s:\n", inet_ntoa(peer_address_.sin_addr));

    char *hostname = resolve_hostname(&peer_address_);
    if(hostname) {
      printf("hostname: %s\n", hostname);
      free(hostname);
    }

    if((header_length = recv(peer_socket, http_header_data, 2048, 0)) < 0) {
      exit_by("recv");
    }
    http_header_data[header_length] = 0;
    //printf("%s\n", http_header_data);

    http_method_t http_method;
    if(resolve_http_method(&http_method, http_header_data, header_length)) {
      fprintf(stderr, "could not resolve corresponding http method\n");
      close(peer_socket);
      continue;
    }

    if(!fork()) {
      http_method(peer_socket, http_header_data, header_length);
      close(peer_socket);
      exit(0);
    }

    close(peer_socket);
  }
}

int main(int argc, char **argv) {
  if(argc < 2) return 1;

  signal(SIGINT, &sigint);
  signal(SIGCHLD, &sigchld);

  return server(argc, argv);

  return 1;
}
