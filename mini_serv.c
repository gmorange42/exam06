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

void	send_to_all(fd_set* fds, int fdmax, int serv, int sender, char* message)
{
	for(int i = 3; i <= fdmax; ++i)
	{
		if (i != sender && i != serv && FD_ISSET(i, fds))
			send(i, message, strlen(message), 0);
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
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

int	main(int ac, char** av)
{
	int	nb_clients = 0;
	int	idmax = 0;
	t_client	clients[10000];
	int	fdmax;
	fd_set	fds;
	fd_set	fds_copy;
	if (ac < 2)
		ft_error("Wrong number of arguments\n");

	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli; 

	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1)
		ft_error("Fatal error\n");
	bzero(&servaddr, sizeof(servaddr)); 

	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433);
	servaddr.sin_port = htons(atoi(av[1])); 

	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		ft_error("Fatal error\n");
	if (listen(sockfd, 10) != 0)
		ft_error("Fatal error\n");

	fdmax = sockfd;
	FD_SET(sockfd, &fds);
	while (1)
	{
		fds_copy = fds;
		if (select(fdmax + 1, &fds_copy, NULL, NULL, NULL) == -1)
			ft_error("Fatal error\n");

		for (int i = 3; i <= fdmax; ++i)
		{
			if (FD_ISSET(i, &fds_copy) == 0)
				continue;
			if (i == sockfd)
			{
				len = sizeof(cli);
				connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
				if (connfd < 0)
					ft_error("Fatal error\n");
				FD_SET(connfd, &fds);
				if (connfd > fdmax)
					fdmax = connfd;
				clients[connfd].id = idmax++;
				clients[connfd].msg = NULL;
//				clients[connfd].msg = calloc(1, sizeof(char) * 2);
				char	str[50];
				sprintf(str, "server: client %d just arrived\n", clients[connfd].id);
				send_to_all(&fds, fdmax, sockfd, connfd, str);
				break;
			}
			else 
			{
				if (clients[i].msg == NULL)
					clients[i].msg = calloc(1, sizeof(char) * 2);

				int	ret;
				char*	buf;
				buf = calloc(1, sizeof(char) * 2);
				ret = recv(i, buf, 1, 0);
				if (ret <= 0)
				{
					char	str[50];
					sprintf(str, "server: client %d just left\n", clients[i].id);
					send_to_all(&fds, fdmax, sockfd, i, str);
					FD_CLR(i, &fds);
//					free(clients[i].msg);
					close(i);
				}
				else
				{
					char*	msg;
					if (extract_message(&buf, &msg) == 0)
						clients[i].msg = str_join(clients[i].msg, buf);
					else
					{
						char	str[50];
						sprintf(str, "client %d: ", clients[i].id);
						send_to_all(&fds, fdmax, sockfd, i, str);
						clients[i].msg = str_join(clients[i].msg, msg);
						send_to_all(&fds, fdmax, sockfd, i, clients[i].msg);
						free(clients[i].msg);
						clients[i].msg = NULL;
//						clients[i].msg = calloc(1, sizeof(char) * 2);
					}
					free(msg);
					msg = NULL;
				}
				free(buf);
				buf = NULL;
			}	
		}
	}
}
