#include "codexion.h"

static int	all_done(t_shared *shared)
{
	int	i;
	int	count;

	if (shared->required_compiles <= 0)
		return (0);
	i = 0;
	count = 0;
	while (i < shared->num_coders)
	{
		pthread_mutex_lock(&shared->coders[i].mutex); /* read done */
		if (shared->coders[i].done)
			count++;
		pthread_mutex_unlock(&shared->coders[i].mutex);
		i++;
	}
	return (count == shared->num_coders);
}

void	*monitor_routine(void *arg)
{
	t_shared	*shared;
	long		now;
	int		i;
	long		last;
	int		done;

	shared = (t_shared *)arg;
	if (shared->required_compiles == 0)
	{
		request_stop(shared); /* immediate finish */
		return (NULL);
	}
	while (!is_stopped(shared))
	{
		now = time_ms();
		i = 0;
		while (i < shared->num_coders)
		{
			pthread_mutex_lock(&shared->coders[i].mutex); /* read state */
			last = shared->coders[i].last_compile_start;
			done = shared->coders[i].done;
			pthread_mutex_unlock(&shared->coders[i].mutex);
			if (!done && now - last >= shared->t_burnout)
			{
				log_state(shared, shared->coders[i].id, "burned out");
				request_stop(shared);
				return (NULL);
			}
			i++;
		}
		if (all_done(shared))
		{
			request_stop(shared);
			return (NULL);
		}
		usleep(1000); /* 1ms check */
	}
	return (NULL);
}
