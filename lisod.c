#include "csapp.h"
#include "parse.h"

void usage(char *liso)
{
    char buf[1024];
    sprintf(buf, "Usage :%s <http port> <https port> <log file> <www folder> <cgi script path> <private key file> <certificate file> ", liso);
    unix_error(buf);
}

void process_request(int sockfd, fd_set *ready_set);

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        usage(argv[0]);
    }
    // 第一阶段只使用http port这一个参数
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
        // send(sockfd, buf, len, 0);
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
        // 解析成功，原路返回
        fprintf(stderr, "valid request\n");
        send(sockfd, buf, size, 0);
    }
    else
    {
        // 解析失败，返回一个400
        char response[MAXLINE];
        sprintf(response, "HTTP/1.1 400 Bad Request\r\n\r\n");
        send(sockfd, response, strlen(response), 0);
        fprintf(stderr, "not valid\n");
    }
    // 不加这一行，多个request会放在一个buf里(why?)
    memset(buf, '\0', MAXBUF);
}