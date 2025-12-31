#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "codexion.h"

long	get_timestamp_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void	log_msg(t_shared *shared, const char *msg)
{
	long	now;

	now = get_timestamp_ms();
	printf("%ldms %s\n", now - shared->start_time, msg);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	char	msg[64];

	coder = (t_coder *)arg;
	snprintf(msg, sizeof(msg), "coder %d starts compiling", coder->id);
	log_msg(coder->shared, msg);
	usleep(200 * 1000);

	snprintf(msg, sizeof(msg), "coder %d debugging", coder->id);
	log_msg(coder->shared, msg);
	usleep(200 * 1000);

	snprintf(msg, sizeof(msg), "coder %d refactoring", coder->id);
	log_msg(coder->shared, msg);
	return (NULL);
}

static int	parse_args(int argc, char **argv, t_shared *shared)
{
	if (argc != 3)
	{
		printf("Usage: %s num_coders num_dongles\n", argv[0]);
		return (0);
	}
	shared->num_coders = atoi(argv[1]);
	shared->num_dongles = atoi(argv[2]);
	shared->finished = 0;
	return (1);
}

int	main(int argc, char **argv)
{
	t_shared	shared;
	t_coder		*coders;
	int			i;

	if (!parse_args(argc, argv, &shared))
		return (1);

	shared.start_time = get_timestamp_ms();
	log_msg(&shared, "program started");

	coders = malloc(sizeof(t_coder) * shared.num_coders);
	if (!coders)
		return (1);

	i = 0;
	while (i < shared.num_coders)
	{
		coders[i].id = i + 1;
		coders[i].shared = &shared;
		pthread_create(&coders[i].thread, NULL, coder_routine, &coders[i]);
		i++;
	}

	log_msg(&shared, "main is waiting for all coders");

	i = 0;
	while (i < shared.num_coders)
	{
		pthread_join(coders[i].thread, NULL);
		i++;
	}

	log_msg(&shared, "program finished");
	free(coders);
	return (0);
}
