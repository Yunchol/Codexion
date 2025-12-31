#include "codexion.h"

int	is_stopped(t_shared *shared)
{
	int	stop;

	pthread_mutex_lock(&shared->state_mutex); /* guard stop */
	stop = shared->stop;
	pthread_mutex_unlock(&shared->state_mutex);
	return (stop);
}

void	request_stop(t_shared *shared)
{
	int	i;

	pthread_mutex_lock(&shared->state_mutex); /* guard stop */
	if (!shared->stop)
		shared->stop = 1;
	pthread_mutex_unlock(&shared->state_mutex);
	i = 0;
	while (i < shared->num_coders)
	{
		pthread_mutex_lock(&shared->dongles[i].mutex); /* wake waiters */
		pthread_cond_broadcast(&shared->dongles[i].cond);
		pthread_mutex_unlock(&shared->dongles[i].mutex);
		i++;
	}
}

void	log_state(t_shared *shared, int id, const char *msg)
{
	long	stamp;
	int	stop;

	pthread_mutex_lock(&shared->log_mutex); /* serialize output */
	pthread_mutex_lock(&shared->state_mutex); /* check stop */
	stop = shared->stop;
	pthread_mutex_unlock(&shared->state_mutex);
	if (!stop || strcmp(msg, "burned out") == 0)
	{
		stamp = sim_time_ms(shared); /* timestamp */
		printf("%ld %d %s\n", stamp, id, msg);
	}
	pthread_mutex_unlock(&shared->log_mutex);
}
