#include "codexion.h"

static void	single_coder(t_coder *coder)
{
	t_shared	*shared;

	shared = coder->shared;
	if (!dongle_acquire(shared, 0, coder))
		return;
	log_state(shared, coder->id, "has taken a dongle");
	while (!is_stopped(shared))
		precise_sleep(shared, 1); /* wait for burnout */
	dongle_release(shared, 0);
}

static void	set_compile_start(t_coder *coder)
{
	pthread_mutex_lock(&coder->mutex); /* update start */
	coder->last_compile_start = time_ms();
	pthread_mutex_unlock(&coder->mutex);
}

static void	mark_compile_done(t_coder *coder)
{
	t_shared	*shared;

	shared = coder->shared;
	pthread_mutex_lock(&coder->mutex); /* update count */
	coder->compile_count++;
	if (shared->required_compiles > 0
		&& coder->compile_count >= shared->required_compiles)
		coder->done = 1;
	pthread_mutex_unlock(&coder->mutex);
}

static int	is_done(t_coder *coder)
{
	int	done;

	pthread_mutex_lock(&coder->mutex); /* read done */
	done = coder->done;
	pthread_mutex_unlock(&coder->mutex);
	return (done);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	t_shared	*shared;
	int		left;
	int		right;
	int		first;
	int		second;

	coder = (t_coder *)arg;
	shared = coder->shared;
	if (shared->num_coders == 1)
	{
		single_coder(coder);
		return (NULL);
	}
	left = coder->id - 1;
	right = coder->id % shared->num_coders;
	first = left;
	second = right;
	if (first > second)
	{
		first = right;
		second = left;
	}
	while (!is_stopped(shared) && !is_done(coder))
	{
		if (!dongle_acquire(shared, first, coder))
			break;
		log_state(shared, coder->id, "has taken a dongle");
		if (!dongle_acquire(shared, second, coder))
		{
			dongle_release(shared, first);
			break;
		}
		log_state(shared, coder->id, "has taken a dongle");
		set_compile_start(coder);
		log_state(shared, coder->id, "is compiling");
		precise_sleep(shared, shared->t_compile);
		dongle_release(shared, second);
		dongle_release(shared, first);
		mark_compile_done(coder);
		log_state(shared, coder->id, "is debugging");
		precise_sleep(shared, shared->t_debug);
		log_state(shared, coder->id, "is refactoring");
		precise_sleep(shared, shared->t_refactor);
	}
	return (NULL);
}
