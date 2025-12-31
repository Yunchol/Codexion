#include "codexion.h"

static int	is_number(const char *s)
{
	int	i;

	if (s[0] == '\0')
		return (0);
	i = 0;
	if (s[0] == '+')
		i++;
	if (s[i] == '\0')
		return (0);
	while (s[i])	
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		i++;
	}
	return (1); /* all digits */
}

static long	parse_long(const char *s, int *ok)
{
	long	res;
	int		i;

	if (!is_number(s))
	{
		*ok = 0;
		return (0);
	}
	i = 0;
	if (s[0] == '+')
		i++;
	res = 0;
	while (s[i])
	{
		res = res * 10 + (s[i] - '0'); /* build number */
		i++;
	}
	*ok = 1;
	return (res);
}

int	parse_args(int ac, char **av, t_shared *shared)
{
	int	ok;
	long	tmp;

	if (ac != 9)
		return (0);
	ok = 1; /* parse all fields */
	shared->num_coders = (int)parse_long(av[1], &ok);
	shared->t_burnout = parse_long(av[2], &ok);
	shared->t_compile = parse_long(av[3], &ok);
	shared->t_debug = parse_long(av[4], &ok);
	shared->t_refactor = parse_long(av[5], &ok);
	tmp = parse_long(av[6], &ok);
	shared->required_compiles = (int)tmp;
	shared->dongle_cooldown = parse_long(av[7], &ok);
	if (!ok)
		return (0);
	if (shared->num_coders <= 0 || shared->t_burnout <= 0
		|| shared->t_compile <= 0 || shared->t_debug <= 0
		|| shared->t_refactor <= 0 || shared->dongle_cooldown < 0)
		return (0);
	if (shared->required_compiles < 0)
		return (0);
	if (strcmp(av[8], "fifo") == 0)
		shared->scheduler = CODEX_SCHED_FIFO;
	else if (strcmp(av[8], "edf") == 0)
		shared->scheduler = CODEX_SCHED_EDF;
	else
		return (0);
	return (1);
}
