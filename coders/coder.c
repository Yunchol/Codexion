#include <stdio.h>
#include <unistd.h>
#include "codexion.h"

static void	queue_push(t_queue *q, int coder_id, long deadline)
{
	q->items[q->count].coder_id = coder_id;
	q->items[q->count].deadline_ms = deadline;
	q->count++;
}

static int	queue_pick_earliest(t_queue *q)
{
	int	i;
	int	best = 0;

	i = 1;
	while (i < q->count)
	{
		if (q->items[i].deadline_ms < q->items[best].deadline_ms)
			best = i;
		i++;
	}
	return (best);
}

static void	queue_remove(t_queue *q, int idx)
{
	q->items[idx] = q->items[q->count - 1];
	q->count--;
}



static void	take_dongle(t_shared *shared, int idx, int coder_id)
{
	char				msg[64];
	long				now;
	long				deadline;
	struct timespec		ts;
	t_dongle			*d;

	d = &shared->dongles[idx];
	deadline = get_timestamp_ms() + 1000; /* ★ 仮の締切：1秒後 */

	pthread_mutex_lock(&d->mutex);

	/* ★ エントリー */
	queue_push(&d->queue, coder_id, deadline);

	while (!shared->stop)
	{
		now = get_timestamp_ms();
		int best = queue_pick_earliest(&d->queue);

		if (!d->in_use
			&& now >= d->next_available_ms
			&& d->queue.items[best].coder_id == coder_id)
			break;

		ts.tv_sec = (now + 50) / 1000;
		ts.tv_nsec = ((now + 50) % 1000) * 1000000;
		pthread_cond_timedwait(&d->cond, &d->mutex, &ts);
	}

    if (shared->stop)
    {
        pthread_mutex_unlock(&d->mutex);
        return;
    }


	/* ★ 自分が最優先だった */
	int idx_best = queue_pick_earliest(&d->queue);
	queue_remove(&d->queue, idx_best);
	d->in_use = 1;

	snprintf(msg, sizeof(msg),
		"coder %d took dongle %d (EDF)", coder_id, idx);
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

    if (coder->shared->stop)
        return (NULL);

	snprintf(msg, sizeof(msg),
		"coder %d wants to compile", coder->id);
	log_msg(coder->shared, msg);

	/* ★ STEP6：順序を固定する（これだけでOK） */
	take_dongle(coder->shared, 0, coder->id);
    if (coder->shared->stop)
        return (NULL);
	take_dongle(coder->shared, 1, coder->id);
    if (coder->shared->stop)
        return (NULL);

	snprintf(msg, sizeof(msg),
		"coder %d compiling", coder->id);
	log_msg(coder->shared, msg);
	usleep(300 * 1000);

	release_dongle(coder->shared, 1, coder->id);
	release_dongle(coder->shared, 0, coder->id);

	snprintf(msg, sizeof(msg),
		"coder %d done", coder->id);
	log_msg(coder->shared, msg);

    /* ★ 終了報告 */
    pthread_mutex_lock(&coder->shared->dongles[0].mutex);
    coder->shared->finished_coders++;
    pthread_mutex_unlock(&coder->shared->dongles[0].mutex);
	return (NULL);
}
