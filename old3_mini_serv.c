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

void	ft_error(char* str, int mode)
{
	write(2, str, strlen(str));
	if (mode == 1)
		exit(1);
}

	void	clean(int fdmax, char* buf, char *msg, t_client* clients)
	{
		if (buf)
		{
			free(buf);
			buf = NULL;
		}
		if (msg)
		{
			free(msg);
			msg = NULL;
		}
		for (int i = 0; i <=fdmax; ++i)
		{
			if (clients[i].msg)
			{
				free(clients[i].msg);
				clients[i].msg = NULL;
			}
		}
		exit(1);
	}

	void	send_to_all(int sender, int sockfd, int fdmax, fd_set* fds, char *str)
	{
		for (int i = 0; i <= fdmax; ++i)
		{
			if (i != sender && i != sockfd && FD_ISSET(i, fds))
				send(i, str, strlen(str), 0);
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
					ft_error("Fatal error\n", 1);
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
			ft_error("Fatal error\n", 1);
		newbuf[0] = 0;
		if (buf != 0)
			strcat(newbuf, buf);
		free(buf);
		strcat(newbuf, add);
		return (newbuf);
	}


	int main(int ac, char** av) {
		if (ac < 2)
			ft_error("Wrong number of arguments\n", 1);
		int sockfd, connfd;
		socklen_t	len;
		struct sockaddr_in servaddr, cli; 
		int fdmax = 0;
		int id_clients = 0;
		fd_set fds;
		fd_set fds_copy;
		t_client	clients[100000];

		// socket create and verification 
		sockfd = socket(AF_INET, SOCK_STREAM, 0); 
		if (sockfd == -1)
			ft_error("Fatal error\n", 1);
		bzero(&servaddr, sizeof(servaddr)); 

		// assign IP, PORT 
		servaddr.sin_family = AF_INET; 
		servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
		servaddr.sin_port = htons(atoi(av[1])); 

		// Binding newly created socket to given IP and verification 
		if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
			ft_error("Fatal error\n", 1);
		if (listen(sockfd, 10) != 0)
			ft_error("Fatal error\n", 1);
		fdmax = sockfd;
		FD_ZERO(&fds);
		FD_SET(sockfd, &fds);
		while (1)
		{
			fds_copy = fds;
			if (select(fdmax + 1, &fds_copy, NULL, NULL, NULL) == -1)
			{
				ft_error("Fatal error\n", 0);
				clean(fdmax, NULL, NULL, clients);
		}
		for (int i = 0; i <= fdmax; ++i)
		{
			if (FD_ISSET(i, &fds_copy) == 0)
				continue;
			if (i == sockfd)
			{
				len = sizeof(cli);
				connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
				if (connfd < 0)
				{
					ft_error("Fatal error\n", 0);
					clean(fdmax, NULL, NULL, clients);
				}
				if (connfd > fdmax)
					fdmax = connfd;
				FD_SET(connfd, &fds);
				clients[connfd].id = id_clients++;
				clients[connfd].msg = NULL;
				char	str[50];
				sprintf(str, "server: client %d just arrived\n", clients[connfd].id);
				send_to_all(connfd, sockfd, fdmax, &fds, str);
				break;
			}
			else
			{
				char*	buf;
				if (!(buf = calloc(1, sizeof(char) * 51)))
				{
					ft_error("Fatal error\n", 0);
					clean(fdmax, buf, NULL, clients);
				}
				if (recv(i, buf, 50, 0) <= 0)
				{
					FD_CLR(i, &fds);
					char	str[50];
					sprintf(str, "server: client %d just left\n", clients[i].id);
					send_to_all(i, sockfd, fdmax, &fds, str);
					close(i);
				}
				else
				{
					char	*msg = NULL;
					if (clients[i].msg == NULL)
						if (!(clients[i].msg = calloc(1, sizeof(char) * 51)))
						{
							ft_error("Fatal error\n", 0);
							clean(fdmax, buf, msg, clients);
						}
					if (extract_message(&buf, &msg) == 0)
						clients[i].msg = str_join(clients[i].msg, buf);
					else
					{
						clients[i].msg = str_join(clients[i].msg, msg);
						char	str[50];
						sprintf(str, "client %d: ", clients[i].id);
						send_to_all(i, sockfd, fdmax, &fds, str);
						send_to_all(i, sockfd, fdmax, &fds, clients[i].msg);
						free(clients[i].msg);
						clients[i].msg = NULL;
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
