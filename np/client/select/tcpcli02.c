/* Use standard echo server; baseline measurements for nonblocking version */
#include	"unp.h"
#include <time.h>
#include <string.h>

void
sig_pipe(int signo)
{
    printf("SIGPIPE received\n");
    return;
}

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in			servaddr;

FILE *pFile;
pFile = fopen("./result.txt", "a");
time_t start, end;
float res;

	if (argc != 2)
		err_quit("usage: tcpcli <IPaddress>");

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(30061);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	Signal(SIGPIPE, sig_pipe);

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

start = clock();

	str_cli(stdin, sockfd);		/* do it all */

end = clock();
res = (float)(end-start)/CLOCKS_PER_SEC;
fprintf(pFile, "Total cost time : %.3f\n", res);
fclose(pFile);

	exit(0);
}
