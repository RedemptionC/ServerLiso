#include "csapp.h"
#include "parse.h"

void usage(char *liso)
{
    char buf[1024];
    sprintf(buf, "Usage :%s <http port> <https port> <log file> <www folder> <cgi script path> <private key file> <certificate file> ", liso);
    unix_error(buf);
}

void process_request(int sockfd, fd_set *ready_set);
void deal_with_request(int sockfd, Request *req);
void print_request(Request *req);
void do_head(int sockfd, Request *req);
void do_get(int sockfd, Request *req);
void do_post(int sockfd, Request *req);

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        usage(argv[0]);
    }
    char *http_port_str = argv[1];

    Signal(SIGPIPE, SIG_IGN);

    int listenfd, connfd;
    fd_set read_fds, ready_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&read_fds);

    // 记录最大的fd+1，在select中使用
    int max_fd;
    listenfd = Open_listenfd(http_port_str);
    max_fd = listenfd + 1;

    // 监听listenfd,如果有表示可以accept
    FD_SET(listenfd, &read_fds);
    struct sockaddr_storage clientaddr;
    socklen_t clientlen;

    char port[MAXBUF], hostname[MAXBUF];

    while (1)
    {
        // select每次返回后，会将待监听的集合修改为ready_set
        ready_fds = read_fds;
        // 设置timeout为NULL,永远不会timeout，一直等到有readyfd
        int rs = Select(max_fd, &ready_fds, NULL, NULL, NULL);
        printf("result of select:%d\n", rs);
        // 如果listenfd处于ready，说明可以accept
        if (FD_ISSET(listenfd, &ready_fds))
        {
            clientlen = sizeof(clientaddr);
            // proxy这里的connfd是malloc的，但是这里是单线程
            // 不知道有没有必要 CHECK
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            // 尚不清楚最后一个参数flags的作用 CHECK
            Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXBUF, port, MAXBUF, 0);
            printf("Accepted connection from (%s, %s)\n", hostname, port);
            // 将新加入的connfd加入到被监听的
            FD_SET(connfd, &read_fds);
            // 并修改max_fd(if necessary)
            if (connfd + 1 > max_fd)
            {
                max_fd = connfd + 1;
                // FOR DEBUG
                printf("maxfd: %d\n", max_fd);
            }
        }
        else
        {
            // 如果不是listenfd,说明已连接的socket可读
            for (int i = 0; i < max_fd; i++)
            {
                if (FD_ISSET(i, &ready_fds))
                {
                    process_request(i, &read_fds);
                }
            }
        }
    }
}
void process_request(int sockfd, fd_set *ready_set)
{
    int len;
    char buf[MAXBUF];
    char line[MAXLINE];
    int size = 0;
    rio_t rio;
    Rio_readinitb(&rio, sockfd);
    while ((len = Rio_readlineb(&rio, line, MAXLINE)) > 0)
    {
        fprintf(stdout, "%d says :%s", sockfd, line);
        sprintf(buf, "%s%s", buf, line);
        // 但是末尾的\r\n也要算进去
        size += len;
        // 遇到空行表示结束
        if (!strcmp(line, "\r\n"))
            break;
        memset(line, '\0', len);
    }

    // 如果recv的返回值为0,表示已关闭?CHECK
    if (len == 0)
    {
        FD_CLR(sockfd, ready_set);
        return;
    }
    Request *req = parse(buf, size, sockfd);
    if (req != NULL)
    {
        deal_with_request(sockfd, req);
    }
    else
    {
        clienterror(sockfd, "Bad Request", "400", "Bad Request", "Bad Request");
        memset(buf, '\0', MAXBUF);
    }
    memset(buf, '\0', MAXBUF);
}
void do_head(int sockfd, Request *req)
{
    char filename[MAXLINE], cgiargs[MAXLINE];
    struct stat sbuf;
    int is_static;
    is_static = parse_uri(req->http_uri, filename, cgiargs);
    // 文件是否存在
    if (stat(filename, &sbuf) < 0)
    {

        clienterror(sockfd, filename, "404", "Not Found", "Not Found");
        return;
    }
    if (is_static)
    {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
        {
            clienterror(sockfd, filename, "403", "Forbidden", "Liso can not read the file");
            return;
        }
        // 返回静态内容
        serve_static(sockfd, filename, sbuf.st_size, 0);
    }
}
void do_get(int sockfd, Request *req)
{
    char filename[MAXLINE], cgiargs[MAXLINE];
    struct stat sbuf;
    int is_static;
    is_static = parse_uri(req->http_uri, filename, cgiargs);
    // 文件是否存在
    if (stat(filename, &sbuf) < 0)
    {

        clienterror(sockfd, filename, "404", "Not Found", "Not Found");
        return;
    }
    if (is_static)
    {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
        {
            clienterror(sockfd, filename, "403", "Forbidden", "Liso can not read the file");
            return;
        }
        // 返回静态内容
        serve_static(sockfd, filename, sbuf.st_size, 1);
    }
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
    {
        clienterror(sockfd, filename, "403", "Forbidden",
                    "Liso couldn't run the CGI program");
        return;
    }
    serve_dynamic(sockfd, filename, cgiargs);
}
void do_post(int sockfd, Request *req)
{
    char response[MAXLINE];
    char content_length[32];

    memset(content_length, 0, sizeof(content_length));
    get_header_value(req, "Content-Length", content_length);
    if (strlen(content_length) == 0)
    {
        clienterror(sockfd, "Length Reqired", "401", "Length Reqired", "Length Reqired");
        return 0;
    }
    char filename[MAXLINE], cgiargs[MAXLINE];
    struct stat sbuf;
    int is_static;
    is_static = parse_uri(req->http_uri, filename, cgiargs);
    // 文件是否存在
    if (stat(filename, &sbuf) < 0)
    {

        clienterror(sockfd, filename, "404", "Not Found", "Not Found");
        return;
    }
    if (!is_static)
    {
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
        {
            clienterror(sockfd, filename, "403", "Forbidden",
                        "Liso couldn't run the CGI program");
            return;
        }
        serve_dynamic(sockfd, filename, cgiargs);
    }
}
void deal_with_request(int sockfd, Request *req)
{
    if (!strcmp(req->http_method, "GET"))
    {
        do_get(sockfd, req);
    }
    else if (!strcmp(req->http_method, "HEAD"))
    {
        do_head(sockfd, req);
    }
    else if (!strcmp(req->http_method, "POST"))
    {
        do_post(sockfd, req);
    }
    else
    {
        clienterror(sockfd, req->http_method, "501", "Not Implemented", "Not Implemented");
        return;
    }
}
void print_request(Request *request)
{
    int index;
    printf("Http Method %s\n", request->http_method);
    printf("Http Version %s\n", request->http_version);
    printf("Http Uri %s\n", request->http_uri);
    for (index = 0; index < request->header_count; index++)
    {
        printf("Request Header\n");
        printf("Header name %s Header Value %s\n", request->headers[index].header_name, request->headers[index].header_value);
    }
}
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;

    if (!strstr(uri, "cgi-bin"))
    { /* Static content */                 
        strcpy(cgiargs, "");               
        strcpy(filename, ".");             
        strcat(filename, uri);             
        if (uri[strlen(uri) - 1] == '/')   
            strcat(filename, "home.html"); 
        return 1;
    }
    else
    { /* Dynamic content */    
        ptr = index(uri, '?'); 
        if (ptr)
        {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        }
        else
            strcpy(cgiargs, ""); 
        strcpy(filename, ".");   
        strcat(filename, uri);   
        return 0;
    }
}
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Liso Error</title>");
    sprintf(body, "%s<body bgcolor="
                  "ffffff"
                  ">\r\n",
            body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Liso  Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}
void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}
void serve_static(int fd, char *filename, int filesize, int withBody)
{
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    /* Send response headers to client */
    get_filetype(filename, filetype);    
    sprintf(buf, "HTTP/1.0 200 OK\r\n"); 
    sprintf(buf, "%sServer: Liso Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    Rio_writen(fd, buf, strlen(buf)); 
    printf("Response headers:\n");
    printf("%s", buf);

    if (withBody)
    {
        /* Send response body to client */
        srcfd = Open(filename, O_RDONLY, 0);                        
        srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); 
        Close(srcfd);                                               
        Rio_writen(fd, srcp, filesize);                             
        Munmap(srcp, filesize);                                     
    }
}
// 根据给定键，设置值
// 如果不存在就不设置hvalue
void get_header_value(Request *request, const char *hname, char *hvalue)
{
    for (int i = 0; i < request->header_count; i++)
    {
        if (!strcmp(request->headers[i].header_name, hname))
        {
            strcpy(hvalue, request->headers[i].header_value);
            return;
        }
    }
}
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = {NULL};

    /* Return first part of HTTP response */
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Liso Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    if (Fork() == 0)
    { /* Child */ 
        /* Real server would set all CGI vars here */
        setenv("QUERY_STRING", cgiargs, 1);                         
        Dup2(fd, STDOUT_FILENO); /* Redirect stdout to client */    
        Execve(filename, emptylist, environ); /* Run CGI program */ 
    }
    Wait(NULL); /* Parent waits for and reaps child */ 
}