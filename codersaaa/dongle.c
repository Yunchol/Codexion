#include "codexion.h"

static void	ms_to_timespec(long abs_ms, struct timespec *ts)
{
	ts->tv_sec = abs_ms / 1000; /* seconds */
	ts->tv_nsec = (abs_ms % 1000) * 1000000; /* nanos */
}

int	dongle_acquire(t_shared *shared, int idx, t_coder *coder)
{
	t_dongle		*dongle;
	t_request		req;
	long			deadline;
	long			now;
	struct timespec	ts;

	dongle = &shared->dongles[idx];
	pthread_mutex_lock(&coder->mutex); /* read deadline */
	deadline = coder->last_compile_start + shared->t_burnout;
	pthread_mutex_unlock(&coder->mutex);
	req.coder_id = coder->id;
	req.deadline = deadline;
	pthread_mutex_lock(&shared->state_mutex); /* fifo seq */
	req.seq = shared->seq_counter++;
	pthread_mutex_unlock(&shared->state_mutex);
	req.heap_index = -1;
	pthread_mutex_lock(&dongle->mutex); /* enter dongle */
	heap_push(&dongle->heap, &req, shared->scheduler);
	while (!is_stopped(shared))
	{
		now = time_ms();
		if (heap_is_top(&dongle->heap, &req)
			&& dongle->holder == 0
			&& now >= dongle->available_at)
		{
			dongle->holder = coder->id;
			heap_remove(&dongle->heap, &req, shared->scheduler);
			pthread_mutex_unlock(&dongle->mutex);
			return (1);
		}
		if (heap_is_top(&dongle->heap, &req)
			&& dongle->holder == 0
			&& now < dongle->available_at)
		{
			ms_to_timespec(dongle->available_at, &ts); /* cooldown wait */
			pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
		}
		else
			pthread_cond_wait(&dongle->cond, &dongle->mutex);
	}
	heap_remove(&dongle->heap, &req, shared->scheduler); /* cancel */
	pthread_mutex_unlock(&dongle->mutex);
	return (0);
}

void	dongle_release(t_shared *shared, int idx)
{
	t_dongle	*dongle;

	dongle = &shared->dongles[idx];
	pthread_mutex_lock(&dongle->mutex); /* update state */
	dongle->holder = 0;
	dongle->available_at = time_ms() + shared->dongle_cooldown;
	pthread_cond_broadcast(&dongle->cond); /* wake waiters */
	pthread_mutex_unlock(&dongle->mutex);
}
