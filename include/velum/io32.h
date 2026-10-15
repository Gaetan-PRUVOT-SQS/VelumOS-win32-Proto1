#ifndef IO32_H
# define IO32_H

# include <stdint.h>

static inline void	outl(uint16_t port, uint32_t v)
{
	__asm__ volatile ("outl %0, %1" : : "a"(v), "Nd"(port) : "memory");
}

static inline uint32_t	inl(uint16_t port)
{
	uint32_t	v;

	__asm__ volatile ("inl %1, %0" : "=a"(v) : "Nd"(port) : "memory");
	return (v);
}

#endif
