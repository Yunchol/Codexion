#include <stdio.h>
#include <unistd.h>
#include "codexion.h"

static void	queue_push(t_queue *q, int id)
{
	q->items[q->tail] = id;
	q->tail = (q->tail + 1) % MAX_CODERS;
	q->count++;
}

static void	queue_pop(t_queue *q)
{
	q->head = (q->head + 1) % MAX_CODERS;
	q->count--;
}

static int	queue_front(t_queue *q)
{
	return (q->items[q->head]);
}


static void	take_dongle(t_shared *shared, int idx, int coder_id)
{
	char	msg[64];
	long	now;
	struct timespec	ts;
	t_dongle *d;

	d = &shared->dongles[idx];

	pthread_mutex_lock(&d->mutex);

	/* ★ 並ぶ */
	queue_push(&d->queue, coder_id);

	while (1)
	{
		now = get_timestamp_ms();
		if (!d->in_use
			&& now >= d->next_available_ms
			&& queue_front(&d->queue) == coder_id)
			break;

		ts.tv_sec = (now + 50) / 1000;
		ts.tv_nsec = ((now + 50) % 1000) * 1000000;
		pthread_cond_timedwait(&d->cond, &d->mutex, &ts);
	}

	/* ★ 先頭なので進む */
	queue_pop(&d->queue);
	d->in_use = 1;

	snprintf(msg, sizeof(msg),
		"coder %d took dongle %d (FIFO)", coder_id, idx);
	log_msg(shared, msg);

	pthread_mutex_unlock(&d->mutex);
}



static void	release_dongle(t_shared *shared, int idx, int coder_id)
{
	char	msg[64];
	long	now;
	t_dongle *d;

	d = &shared->dongles[idx];

	pthread_mutex_lock(&d->mutex);

	d->in_use = 0;
	now = get_timestamp_ms();
	d->next_available_ms = now + 200;

	snprintf(msg, sizeof(msg),
		"coder %d released dongle %d", coder_id, idx);
	log_msg(shared, msg);

	/* ★ 次の人を起こす */
	pthread_cond_broadcast(&d->cond);

	pthread_mutex_unlock(&d->mutex);
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
