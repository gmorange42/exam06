#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

void	ft_error(char* str)
{
	write(2, str, strlen(str));
	exit(1);
}

void	send_to_clients(int sender, int sockfd, int fdmax, fd_set* rfds, char* info, char* message)
{
	for (int i = sockfd + 1; i <= fdmax; ++i)
	{
		if (i != sender)
		{
			if (FD_ISSET(i, rfds))
			{
				send(i, info, strlen(info), 0);
				send(i, message, strlen(message), 0);
			}
		}
	}
}

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			free(*buf);
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	*msg = *buf;
	*buf = NULL;
	return (0);
}

int main(int ac, char** av) {

	fd_set rfds;
	fd_set tempfds;
	char	buf[200000];
	char	prefixe[50];
	char*	buffer;
	char*	msg;
	if (ac < 2)
		ft_error("Wrong number of arguments\n");
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli; 

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1)
		ft_error("Fatal error\n");
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1])); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		ft_error("Fatal error\n");
	if (listen(sockfd, 10) != 0)
		ft_error("Fatal error\n");
	FD_SET(sockfd, &rfds);
	int	fdmax = sockfd;
	while (1)
	{
		buffer = NULL;
		msg = NULL;
		bzero(&buf, sizeof(buf));
		bzero(&prefixe, sizeof(prefixe));
		tempfds = rfds;
		if (select(fdmax + 1, &tempfds, NULL, NULL, NULL) == -1)
			ft_error("Fatal error\n");
		for (int i = sockfd; i <= fdmax; ++i)
		{
			if (FD_ISSET(i, &tempfds) == 0)
				continue;
			if (i == sockfd)
			{
				puts("In \"new connect\"");
				len = sizeof(cli);
				connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
				if (connfd < 0)
					ft_error("Fatal error\n");
				FD_SET(connfd, &rfds);
				sprintf(prefixe, "server: client %d just arrived\n", connfd - sockfd - 1);
				send_to_clients(connfd, sockfd, fdmax, &rfds, (char*)prefixe, "");
				++fdmax;
				break;
			}
			else
			{
				puts("In \"ELSE\"");
				ssize_t size;
				size = recv(i, buf, sizeof(buf), 0);
				if (size == -1)
					ft_error("Fatal error\n");
				if (size == 0)
				{
					FD_CLR(i, &rfds);
					sprintf(prefixe, "server: client %d just left\n", i - sockfd - 1);
					send_to_clients(i, sockfd, fdmax, &rfds, (char*)prefixe, "");
				}
				else
				{
					buffer = malloc(sizeof(char) * (strlen(buf) + 1));
					if (buffer == 0)
						ft_error("Fatal error\n");
					strcpy(buffer, buf);

					while(extract_message(&buffer, &msg) == 1)
					{
						printf("MSG = %s\n", msg);
						sprintf(prefixe, "client %d: ", i - sockfd - 1);
						send_to_clients(i, sockfd, fdmax, &rfds, (char*)prefixe, msg);
						free(msg);
						msg = NULL;
					}
					free(buffer);
					buffer = NULL;
				}
			}
		}
	}
}
