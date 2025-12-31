#include "codexion.h"

static int	start_threads(t_shared *shared, int *created)
{
	int	i;

	i = 0;
	while (i < shared->num_coders)
	{
		if (pthread_create(&shared->coders[i].thread, NULL, coder_routine, &shared->coders[i]) != 0)
		{
			*created = i; /* count created */
			return (0);
		}
		i++;
	}

	if (pthread_create(&shared->monitor, NULL, monitor_routine, shared) != 0)
	{
		*created = i;
		return (0);
	}
	*created = i;
	return (1);
}

static void	join_threads(t_shared *shared, int created, int join_monitor)
{
	int	i;

	if (join_monitor)
		pthread_join(shared->monitor, NULL); /* wait monitor */
	i = 0;
	while (i < created)
	{
		pthread_join(shared->coders[i].thread, NULL);
		i++;
	}
}

int	main(int ac, char **av)
{
	t_shared	shared;
	int			created;

	memset(&shared, 0, sizeof(shared)); /* zero init */
	if (!parse_args(ac, av, &shared))
	{
		fprintf(stderr, "Error: invalid arguments\n");
		return (1);
	}
	if (!init_sim(&shared))
	{
		fprintf(stderr, "Error: init failed\n");
		return (1);
	}
	if (!start_threads(&shared, &created))
	{
		request_stop(&shared);
		join_threads(&shared, created, 0);
		cleanup_sim(&shared);
		fprintf(stderr, "Error: thread failure\n");
		return (1);
	}
	join_threads(&shared, shared.num_coders, 1);
	cleanup_sim(&shared);
	return (0);
}
