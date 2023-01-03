#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

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
	*msg = *buf;
	*buf = NULL;
	return (1);
}

int	main(void)
       {
	       char *buf = NULL;
	       char *msg = NULL;

	       buf = strdup("hello, je suis un test\npouet haha\na");
	       printf("[buf => %s]", buf);
	       printf("[msg => %s\n]", msg);

	       while (extract_message(&buf, &msg) == 1)
	       {
		       printf("[buf => %s]", buf);
		       printf("[msg => %s\n]", msg);
		       free(msg);
	       }
	       exit(EXIT_SUCCESS);
       }

