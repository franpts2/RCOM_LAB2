#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

#define FTP_PORT 21
#define MAX_LENGTH 1024

struct URL {
    char user[128];
    char password[128];
    char host[128];
    char path[256];
    char filename[128];
};

int parse_url(char *input, struct URL *url) {
    char *p = input;
    
    // default credentials 
    strcpy(url->user, "anonymous");
    strcpy(url->password, "anonymous@");

    // check for ftp://
    if (strncmp(p, "ftp://", 6) != 0) {
        printf("Error: URL must start with ftp://\n");
        return -1;
    }
    p += 6; // move past ftp://

    // check for credentials (@)
    char *at_sign = strchr(p, '@');
    char *slash = strchr(p, '/');

    if (at_sign && (!slash || at_sign < slash)) {
        char credentials[256];
        int len = at_sign - p;
        strncpy(credentials, p, len);
        credentials[len] = '\0';
        
        char *colon = strchr(credentials, ':');
        if (colon) {
            *colon = '\0';
            strcpy(url->user, credentials);
            strcpy(url->password, colon + 1);
        } else {
            strcpy(url->user, credentials);
        }
        p = at_sign + 1;
    }

    // parse host and path
    slash = strchr(p, '/');
    if (slash) {
        int len = slash - p;
        strncpy(url->host, p, len);
        url->host[len] = '\0';
        strcpy(url->path, slash + 1);
    } else {
        strcpy(url->host, p);
        strcpy(url->path, ""); // no path provided
    }

    // extract filename
    char *last_slash = strrchr(url->path, '/');
    if (last_slash) {
        strcpy(url->filename, last_slash + 1);
    } else {
        strcpy(url->filename, url->path);
    }

    if (strlen(url->filename) == 0) strcpy(url->filename, "index.html"); // default

    return 0;
}


int read_response(int sockfd, char *buffer, size_t size) {
    memset(buffer, 0, size);
    int res = 0;
    char ch;
    int line_start = 0; 

    while(read(sockfd, &ch, 1) > 0) {
        if (res < size - 1) {
            buffer[res++] = ch;
        }

        if (ch == '\n') {
            if (buffer[line_start + 3] == ' ') {
                break;
            }
            line_start = res;
        }
    }
    buffer[res] = '\0'; 
    printf("%s", buffer); 
    return res;
}


int create_socket(char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    /*server address handling*/
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);    
    server_addr.sin_port = htons(port); 

    /*open a TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }
    /*connect to the server*/
    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("connect()");
        return -1;
    }
    return sockfd;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        exit(-1);
    }

    struct URL url;
    if (parse_url(argv[1], &url) < 0) exit(-1);

    printf("Parsed Info:\nUser: %s\nPass: %s\nHost: %s\nPath: %s\nFile: %s\n\n", 
           url.user, url.password, url.host, url.path, url.filename);

    // dns resolution 
    struct hostent *h;
    if ((h = gethostbyname(url.host)) == NULL) {
        herror("gethostbyname()");
        exit(-1);
    }
    char *server_ip = inet_ntoa(*((struct in_addr *) h->h_addr));
    printf("IP Address : %s\n", server_ip);

    // control connection 
    int control_sock = create_socket(server_ip, FTP_PORT);
    if (control_sock < 0) exit(-1);

    char buf[MAX_LENGTH];
    read_response(control_sock, buf, MAX_LENGTH); //  220 greeting

    // authentication
    sprintf(buf, "USER %s\r\n", url.user);
    printf("C: %s", buf);
    write(control_sock, buf, strlen(buf));
    read_response(control_sock, buf, MAX_LENGTH); //expect 331

    sprintf(buf, "PASS %s\r\n", url.password);
    printf("C: PASS *****\r\n"); // hide password 
    write(control_sock, buf, strlen(buf));
    read_response(control_sock, buf, MAX_LENGTH); // expect 230

    // passive mode 
    sprintf(buf, "PASV\r\n");
    printf("C: %s", buf);
    write(control_sock, buf, strlen(buf));
    read_response(control_sock, buf, MAX_LENGTH); // expect 227

    // parse PASV response: 227 entering passive mode 
    int ip1, ip2, ip3, ip4, p1, p2;
 
    char *start_ip = strchr(buf, '(');
    if (start_ip == NULL) {
        printf("Error: Could not parse PASV response. Server said: %s\n", buf);
        close(control_sock);
        exit(-1);
    }
    sscanf(start_ip, "(%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &p1, &p2);
    
    int data_port = p1 * 256 + p2;
    char data_ip_str[30];
    sprintf(data_ip_str, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
    printf("Data connection to: %s : %d\n", data_ip_str, data_port);

    // data connection
    int data_sock = create_socket(data_ip_str, data_port);
    if (data_sock < 0) {
        printf("Failed to connect to data port.\n");
        exit(-1);
    }

    // request file 
    sprintf(buf, "RETR %s\r\n", url.path);
    printf("C: %s", buf);
    write(control_sock, buf, strlen(buf));
    read_response(control_sock, buf, MAX_LENGTH); // expect 150

    // download file
    FILE *fp = fopen(url.filename, "wb");
    int bytes_read;
    long long total_bytes = 0; 

    printf("Starting download... please wait.\n");

    while ((bytes_read = read(data_sock, buf, MAX_LENGTH)) > 0) {
        fwrite(buf, 1, bytes_read, fp);
        total_bytes += bytes_read;

        // print progress every 10 mb
        if (total_bytes % (10 * 1024 * 1024) < MAX_LENGTH) {
            printf("\rDownloaded: %lld MB", total_bytes / (1024 * 1024));
            fflush(stdout); 
        }
    }
    printf("\n"); 
    fclose(fp);
    close(data_sock);

    // final Response
    read_response(control_sock, buf, MAX_LENGTH); // esperar 226 transfer complete

    // clean up
    sprintf(buf, "QUIT\r\n");
    printf("C: %s", buf);
    write(control_sock, buf, strlen(buf));
    
    close(control_sock);
    printf("Download Complete.\n");

    return 0;
}