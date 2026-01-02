#include <stdio.h>
#include <unistd.h>
#include "codexion.h"

/* dongle を1本取る */
static void	take_dongle(t_shared *shared, int idx, int coder_id)
{
	char	msg[64];

	pthread_mutex_lock(&shared->dongles[idx].mutex);
	while (shared->dongles[idx].in_use)
	{
		pthread_mutex_unlock(&shared->dongles[idx].mutex);
		usleep(1000);
		pthread_mutex_lock(&shared->dongles[idx].mutex);
	}
	shared->dongles[idx].in_use = 1;
	snprintf(msg, sizeof(msg), "coder %d took dongle %d", coder_id, idx);
	log_msg(shared, msg);
	pthread_mutex_unlock(&shared->dongles[idx].mutex);
}

/* dongle を返す */
static void	release_dongle(t_shared *shared, int idx, int coder_id)
{
	char	msg[64];

	pthread_mutex_lock(&shared->dongles[idx].mutex);
	shared->dongles[idx].in_use = 0;
	snprintf(msg, sizeof(msg), "coder %d released dongle %d", coder_id, idx);
	log_msg(shared, msg);
	pthread_mutex_unlock(&shared->dongles[idx].mutex);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	char	msg[64];

	coder = (t_coder *)arg;

	snprintf(msg, sizeof(msg), "coder %d wants to compile", coder->id);
	log_msg(coder->shared, msg);

	/* ★ 順序を決めずに2本取る（ここがSTEP5の地雷） */
	take_dongle(coder->shared, 0, coder->id);
	usleep(10000); /* 競合を起こしやすくする */
	take_dongle(coder->shared, 1, coder->id);

	snprintf(msg, sizeof(msg), "coder %d compiling", coder->id);
	log_msg(coder->shared, msg);
	usleep(300 * 1000);

	release_dongle(coder->shared, 1, coder->id);
	release_dongle(coder->shared, 0, coder->id);

	snprintf(msg, sizeof(msg), "coder %d done", coder->id);
	log_msg(coder->shared, msg);
	return (NULL);
}
