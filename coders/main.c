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

/* dongle を取る */
static void	take_dongle(t_shared *shared, int coder_id)
{
	char	msg[64];

	pthread_mutex_lock(&shared->dongle.mutex);
	while (shared->dongle.in_use)
	{
		/* 今回は busy wait（後で改善する） */
		pthread_mutex_unlock(&shared->dongle.mutex);
		usleep(1000);
		pthread_mutex_lock(&shared->dongle.mutex);
	}
	shared->dongle.in_use = 1;
	snprintf(msg, sizeof(msg), "coder %d took dongle", coder_id);
	log_msg(shared, msg);
	pthread_mutex_unlock(&shared->dongle.mutex);
}

/* dongle を返す */
static void	release_dongle(t_shared *shared, int coder_id)
{
	char	msg[64];

	pthread_mutex_lock(&shared->dongle.mutex);
	shared->dongle.in_use = 0;
	snprintf(msg, sizeof(msg), "coder %d released dongle", coder_id);
	log_msg(shared, msg);
	pthread_mutex_unlock(&shared->dongle.mutex);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	char	msg[64];

	coder = (t_coder *)arg;

	snprintf(msg, sizeof(msg), "coder %d wants to compile", coder->id);
	log_msg(coder->shared, msg);

	take_dongle(coder->shared, coder->id);

	snprintf(msg, sizeof(msg), "coder %d compiling", coder->id);
	log_msg(coder->shared, msg);
	usleep(300 * 1000);

	release_dongle(coder->shared, coder->id);

	snprintf(msg, sizeof(msg), "coder %d done", coder->id);
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

	/* dongle 初期化 */
	pthread_mutex_init(&shared.dongle.mutex, NULL);
	shared.dongle.in_use = 0;

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

	i = 0;
	while (i < shared.num_coders)
	{
		pthread_join(coders[i].thread, NULL);
		i++;
	}

	log_msg(&shared, "program finished");

	pthread_mutex_destroy(&shared.dongle.mutex);
	free(coders);
	return (0);
}
