#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "codexion.h"

/* 今の時刻をミリ秒で返す */
long	get_timestamp_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/* 経過時間付きログ */
void	log_msg(t_shared *shared, const char *msg)
{
	long	now;

	now = get_timestamp_ms();
	printf("%ldms %s\n", now - shared->start_time, msg);
}

/* coder がやる仕事（スレッドの中身） */
void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	log_msg(coder->shared, "coder starts compiling");
	usleep(200 * 1000);
	log_msg(coder->shared, "coder debugging");
	usleep(200 * 1000);
	log_msg(coder->shared, "coder refactoring");
	return (NULL);
}

/* 引数処理 */
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
	t_coder		coder;

	if (!parse_args(argc, argv, &shared))
		return (1);

	shared.start_time = get_timestamp_ms();
	log_msg(&shared, "program started");

	coder.id = 1;
	coder.shared = &shared;

	pthread_create(&coder.thread, NULL, coder_routine, &coder);

	log_msg(&shared, "main is waiting for coder");
	pthread_join(coder.thread, NULL);

	log_msg(&shared, "program finished");
	return (0);
}
