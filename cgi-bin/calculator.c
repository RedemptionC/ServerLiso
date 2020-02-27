#include "../csapp.h"

char pop(char *stack, int *top)
{
    return stack[(*top)--];
}
void push(char *stack, int *top, char x)
{

    stack[++(*top)] = x;
}
char gettop(char *stack, int top)
{
    if (top < 0)
        return -1;
    return stack[top];
}
int is_digit(char c)
{
    return c >= '0' && c <= '9';
}
int calc(char c, int op1, int op2)
{
    switch (c)
    {
    case '+':
        return op1 + op2;
    case '-':
        return op1 - op2;
    case '*':
        return op1 * op2;
    case '/':
        return op1 / op2;
    default:
        return -1;
    }
}
int isop(char c)
{
    return c == '+' || c == '-';
}

char *in2post(char *s)
{
    char *stack = (char *)malloc(sizeof(char) * (strlen(s) + 1));
    char *post = (char *)malloc(sizeof(char) * (strlen(s) * 2 + 1));
    int j = 0;
    int top = -1;
    int i = 0;
    while (s[i] != '\0')
    {
        if (is_digit(s[i]))
        {
            while (is_digit(s[i]))
            {
                post[j++] = s[i++];
            }
            post[j++] = ' ';
        }
        if (s[i] == ' ')
        {
            //i++;
            post[j++] = s[i++];
        }
        if (s[i] == ')')
        {
            while (gettop(stack, top) != '(')
            {
                char c = pop(stack, &top);
                post[j++] = c;
            }
            //pop (,but not saved in post
            pop(stack, &top);
            i++;
        }
        if (s[i] == '(')
        {
            // no op is higher than (,so just push
            push(stack, &top, s[i++]);
        }
        if (s[i] == '+' || s[i] == '-')
        {
            // only higher is (,but ( won't be removed unless c is )
            // so just push===>no,previous + or - should be poped
            while (gettop(stack, top) == '+' || gettop(stack, top) == '-')
            {
                char c = pop(stack, &top);
                post[j++] = c;
            }
            push(stack, &top, s[i++]);
        }
    }
    // read the end of input,pop all
    while (top >= 0)
    {
        char c = pop(stack, &top);
        post[j++] = c;
    }
    free(stack);
    post[j] = '\0';
    return post;
}
unsigned int evalRPN(char *post)
{
    unsigned int *stack = (unsigned int *)malloc(sizeof(unsigned int) * (strlen(post) + 1));
    int top = -1;
    int i = 0;
    while (post[i] != '\0')
    {

        if (is_digit(post[i]))
        {

            unsigned int num = 0;
            while (is_digit(post[i]))
            {
                num = num * 10 + post[i++] - '0';
            }
            // push(stack,&top,num);
            stack[++top] = num;
        }
        if (isop(post[i]))
        {
            int op2 = stack[top--];
            int op1 = stack[top--];
            int rs = calc(post[i], op1, op2);
            // push(stack,&top,rs);
            stack[++top] = rs;
            i++;
        }
        if (post[i] == ' ')
            i++;
    }
    int rs = stack[top];
    free(stack);
    free(post);
    return rs;
}
int calculate(char *s)
{
    char *post = in2post(s);
    // puts(post);
    //    free(post);
    return evalRPN(post);
}

int main()
{
    char *buf, *p;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1 = 0, n2 = 0;
    int replaced = 0;
    buf = getenv("QUERY_STRING");
    // 注意+会被编码为%2B
    p = index(buf, '=');
    strcpy(arg1, p + 1);
    if (strstr(arg1, "%2B"))
    {
        replaced = 1;
        int i = 0, j = 0;
        int len = strlen(arg1);
        for (; i < len;)
        {
            if (arg1[i] == '%')
            {
                i += 3;
                arg2[j++]='+';
            }
            else
            {
                arg2[j++] = arg1[i++];
            }
        }
    }
    fprintf(stderr, "%s\n", replaced?arg2:arg1);
    /* Make the response body */

    int rs = calculate(replaced?arg2:arg1);
    sprintf(content, "%s<h1>%s = %d</h1>\r\n<p>",
            content, replaced?arg2:arg1, rs);
    sprintf(content, "%s<h2>Thanks for visiting!</h2>\r\n", content);

    /* Generate the HTTP response */
    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);
    fflush(stdout);

    exit(0);
    // char* p="1+2+(3+2)";
    // printf("%d\n",calculate(p));
}