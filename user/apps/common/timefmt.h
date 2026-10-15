#ifndef TIMEFMT_H
# define TIMEFMT_H

# include <stddef.h>
# include <stdint.h>

# define CLOCK_TEXT_MAX 6
# define NS_PER_SEC 1000000000ull
# define NS_PER_MINUTE 60000000000ull

int			fmt_clock(uint64_t wall_ns, char *out, size_t size);
int			fmt_uptime(uint64_t ns, char *out, size_t size);
uint64_t	ns_to_next_minute(uint64_t wall_ns);

#endif
