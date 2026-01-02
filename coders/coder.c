#include <stdio.h>
#include <unistd.h>
#include "codexion.h"

static void	take_dongle(t_shared *shared, int idx, int coder_id)
{
	char			msg[64];
	long			now;
	struct timespec	ts;

	pthread_mutex_lock(&shared->dongles[idx].mutex);
	while (1)
	{
		now = get_timestamp_ms();
		if (!shared->dongles[idx].in_use
			&& now >= shared->dongles[idx].next_available_ms)
			break;

		/* ★ 次に起きるべき時刻を計算 */
		long wait_ms = shared->dongles[idx].next_available_ms;
		if (wait_ms < now)
			wait_ms = now + 10;

		ts.tv_sec = wait_ms / 1000;
		ts.tv_nsec = (wait_ms % 1000) * 1000000;

		pthread_cond_timedwait(
			&shared->dongles[idx].cond,
			&shared->dongles[idx].mutex,
			&ts);
	}

	shared->dongles[idx].in_use = 1;
	snprintf(msg, sizeof(msg),
		"coder %d took dongle %d", coder_id, idx);
	log_msg(shared, msg);
	pthread_mutex_unlock(&shared->dongles[idx].mutex);
}



static void	release_dongle(t_shared *shared, int idx, int coder_id)
{
	char	msg[64];
	long	now;

	pthread_mutex_lock(&shared->dongles[idx].mutex);
	shared->dongles[idx].in_use = 0;

	now = get_timestamp_ms();
	shared->dongles[idx].next_available_ms = now + 200;

	snprintf(msg, sizeof(msg), "coder %d released dongle %d (cooldown)", coder_id, idx);
	log_msg(shared, msg);

	/* ★ 条件が変わったので起こす */
	pthread_cond_broadcast(&shared->dongles[idx].cond);

	pthread_mutex_unlock(&shared->dongles[idx].mutex);
}



void	*coder_routine(void *arg)
{
	t_coder	*coder;
	char	msg[64];

	coder = (t_coder *)arg;

	snprintf(msg, sizeof(msg),
		"coder %d wants to compile", coder->id);
	log_msg(coder->shared, msg);

	/* ★ STEP6：順序を固定する（これだけでOK） */
	take_dongle(coder->shared, 0, coder->id);
	take_dongle(coder->shared, 1, coder->id);

	snprintf(msg, sizeof(msg),
		"coder %d compiling", coder->id);
	log_msg(coder->shared, msg);
	usleep(300 * 1000);

	release_dongle(coder->shared, 1, coder->id);
	release_dongle(coder->shared, 0, coder->id);

	snprintf(msg, sizeof(msg),
		"coder %d done", coder->id);
	log_msg(coder->shared, msg);
	return (NULL);
}
