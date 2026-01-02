#include <stdio.h>
#include <stdlib.h>
#include "codexion.h"

static int	parse_args(int argc, char **argv, t_shared *shared)
{
	if (argc != 2)
	{
		printf("Usage: %s num_coders\n", argv[0]);
		return (0);
	}
	shared->num_coders = atoi(argv[1]);
	return (1);
}

int	main(int argc, char **argv)
{
	t_shared	shared;
	t_coder		*coders;
	int			i;

	if (!parse_args(argc, argv, &shared))
		return (1);

	shared.start_time = get_timestamp_ms();

	/* dongle 初期化 */
	i = 0;
	while (i < 2)
	{
		pthread_mutex_init(&shared.dongles[i].mutex, NULL);
		pthread_cond_init(&shared.dongles[i].cond, NULL);
		shared.dongles[i].in_use = 0;
		shared.dongles[i].next_available_ms = 0;
		shared.dongles[i].queue.head = 0;
		shared.dongles[i].queue.tail = 0;
		shared.dongles[i].queue.count = 0;

		i++;
	}

	log_msg(&shared, "program started");

	coders = malloc(sizeof(t_coder) * shared.num_coders);
	if (!coders)
		return (1);

	i = 0;
	while (i < shared.num_coders)
	{
		coders[i].id = i + 1;
		coders[i].shared = &shared;
		pthread_create(&coders[i].thread, NULL, coder_routine, &coders[i]);
		i++;
	}

	i = 0;
	while (i < shared.num_coders)
	{
		pthread_join(coders[i].thread, NULL);
		i++;
	}

	log_msg(&shared, "program finished");

	i = 0;
	while (i < 2)
	{
		pthread_cond_destroy(&shared.dongles[i].cond);
		pthread_mutex_destroy(&shared.dongles[i].mutex);
		i++;
	}

	free(coders);
	return (0);
}
