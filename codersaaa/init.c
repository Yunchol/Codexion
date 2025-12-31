#include "codexion.h"

static int	init_dongles(t_shared *shared)
{
	int	i;

	shared->dongles = (t_dongle *)malloc(sizeof(t_dongle)
			* shared->num_coders); /* dongle array */
	if (!shared->dongles)
		return (0);
	i = 0;
	while (i < shared->num_coders)
	{
		pthread_mutex_init(&shared->dongles[i].mutex, NULL);
		pthread_cond_init(&shared->dongles[i].cond, NULL);
		shared->dongles[i].available_at = shared->start_ms;
		shared->dongles[i].holder = 0;
		if (!heap_init(&shared->dongles[i].heap, shared->num_coders))
		{
			while (i >= 0)
			{
				pthread_mutex_destroy(&shared->dongles[i].mutex);
				pthread_cond_destroy(&shared->dongles[i].cond);
				heap_destroy(&shared->dongles[i].heap);
				i--;
			}
			free(shared->dongles);
			return (0);
		}
		i++;
	}
	return (1);
}

static int	init_coders(t_shared *shared)
{
	int	i;

	shared->coders = (t_coder *)malloc(sizeof(t_coder)
			* shared->num_coders); /* coder array */
	if (!shared->coders)
		return (0);
	i = 0;
	while (i < shared->num_coders)
	{
		shared->coders[i].id = i + 1;
		shared->coders[i].last_compile_start = shared->start_ms;
		shared->coders[i].compile_count = 0;
		shared->coders[i].done = 0;
		shared->coders[i].shared = shared;
		pthread_mutex_init(&shared->coders[i].mutex, NULL);
		i++;
	}
	return (1);
}

int	init_sim(t_shared *shared)
{
	int	i;

	shared->stop = 0;
	shared->seq_counter = 0;
	shared->start_ms = time_ms(); /* base time */
	pthread_mutex_init(&shared->state_mutex, NULL);
	pthread_mutex_init(&shared->log_mutex, NULL);
	if (!init_dongles(shared))
		return (0);
	if (!init_coders(shared))
	{
		i = 0;
		while (i < shared->num_coders)
		{
			pthread_mutex_destroy(&shared->dongles[i].mutex);
			pthread_cond_destroy(&shared->dongles[i].cond);
			heap_destroy(&shared->dongles[i].heap);
			i++;
		}
		free(shared->dongles);
		return (0);
	}
	return (1);
}

void	cleanup_sim(t_shared *shared)
{
	int	i;

	i = 0;
	if (shared->coders)
	{
		while (i < shared->num_coders)
		{
			pthread_mutex_destroy(&shared->coders[i].mutex);
			i++;
		}
		free(shared->coders);
	}
	i = 0;
	if (shared->dongles)
	{
		while (i < shared->num_coders)
		{
			pthread_mutex_destroy(&shared->dongles[i].mutex);
			pthread_cond_destroy(&shared->dongles[i].cond);
			heap_destroy(&shared->dongles[i].heap);
			i++;
		}
		free(shared->dongles);
	}
	pthread_mutex_destroy(&shared->state_mutex);
	pthread_mutex_destroy(&shared->log_mutex);
}
