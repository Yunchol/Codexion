#ifndef CODEXION_H
#define CODEXION_H

#include <pthread.h>

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	int				in_use;
}	t_dongle;

typedef struct s_shared
{
	int		num_coders;
	int		num_dongles;
	int		finished;
	long	start_time;
	t_dongle dongle;
}	t_shared;

typedef struct s_coder
{
	int			id;
	t_shared	*shared;
	pthread_t	thread;
}	t_coder;

long	get_timestamp_ms(void);
void	log_msg(t_shared *shared, const char *msg);

void	*coder_routine(void *arg);

#endif
