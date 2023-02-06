#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct	s_client
{
	int	id;
	char*	msg;
}		t_client;

void	ft_error(char* str)
{
	write(2, str, strlen(str));
	exit(1);
}

void	send_to_chan(int sender, int sender_id, int sockfd, int fdmax, int mode, fd_set* fds, char* msg)
{
	char	prefixe[64 + strlen(msg)];

	if (mode == 1)
		sprintf(prefixe, "server: client %d just arrived\n", sender_id);
	if (mode == 2)
		sprintf(prefixe, "client %d: %s", sender_id, msg);
	if (mode == 3)
		sprintf(prefixe, "server: client %d just left\n", sender_id);

	for (int i = 0; i <= fdmax; ++i)
	{
		if (i != sender && i != sockfd && FD_ISSET(i, fds) != 0)
			send(i, prefixe, strlen(prefixe), 0);
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
				ft_error("Fatal error\n");
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		ft_error("Fatal error\n");
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}


int main(int ac, char** av) {
	if (ac < 2)
		ft_error("Wrong number of arguments\n");
	int sockfd, connfd;
	socklen_t	len;
	struct sockaddr_in servaddr, cli; 
	int	fdmax;
	int	id_client = 0;
	fd_set	fds;
	fd_set	fds_copy;
	t_client	clients[100000];

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
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);
	fdmax = sockfd;
	while (1)
	{
		fds_copy = fds;
		if (select(fdmax + 1, &fds_copy, NULL, NULL, NULL) == -1)
			ft_error("Fatal error\n");
		for (int i = 0; i <= fdmax; ++i)
		{
			if (FD_ISSET(i, &fds_copy) == 0)
				continue;
			if (i == sockfd)
			{
				len = sizeof(cli);
				connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
				if (connfd < 0)
					ft_error("Fatal error\n");
				if (fdmax < connfd)
					fdmax = connfd;
				clients[connfd].id = id_client++;
				clients[connfd].msg = NULL;
				FD_SET(connfd, &fds);
				send_to_chan(connfd, clients[connfd].id, sockfd, fdmax, 1, &fds, "");
				break;
			}
			else
			{
				char	buf[32];
				bzero(&buf, sizeof(char) * 32);
				if (recv(i, buf, 31, 0) < 1)
				{
					FD_CLR(i, &fds);
					send_to_chan(i, clients[i].id, sockfd, fdmax, 3, &fds, "");
					close(i);
				}
				else
				{
					char*	msg;
					clients[i].msg = str_join(clients[i].msg, buf);
					while(extract_message(&clients[i].msg, &msg) == 1)
					{
						send_to_chan(i, clients[i].id, sockfd, fdmax, 2, &fds, msg);
						free(msg);
						msg = NULL;
					}
					free(msg);
					msg = NULL;
					if (strlen(clients[i].msg) == 0)
					{
						free(clients[i].msg);
						clients[i].msg= NULL;
					}
				}
			}
		}
	}
}
