/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "./csapp.h"

int main(void) {
    char *buf, *p;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1=0, n2=0;

    /* Extract the two arguments */
    if ((buf = getenv("QUERY_STRING")) != NULL) {
	p = strchr(buf, '&');
	*p = '\0';
	// strcpy(arg1, buf);
	// strcpy(arg2, p+1);
    }

    /* Make the response body */

    char * ptr=index(buf,'=');
    strcpy(arg1,ptr+1);
    ptr=index(p+1,'=');
    strcpy(arg2,ptr+1);
    sprintf(content, "%s<h1>Hello,%s %s</h1>\r\n<p>", 
	    content, arg1,arg2);
    sprintf(content, "%s<h2>Thanks for visiting!</h2>\r\n", content);
  
    /* Generate the HTTP response */
    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);
    fflush(stdout);

    exit(0);
}
/* $end adder */
