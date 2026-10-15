#include "d09.h"

void	fk_case3(const t_case *c)
{
	uint16_t	u[3];

	u[0] = c->op;
	u[1] = 0x0402;
	u[2] = 0x0010;
	fk_play(u, 3, c);
}

void	fk_case2(const t_case *c)
{
	uint16_t	u[2];

	u[0] = (uint16_t)(c->op | 0x2000);
	u[1] = 0x0010;
	fk_play(u, 2, c);
}

void	fk_case2a(const t_case *c)
{
	uint16_t	u[2];

	u[0] = (uint16_t)(c->op | 0x4200);
	u[1] = 0x0210;
	fk_play(u, 2, c);
}

void	fk_caselit(const t_case *c)
{
	uint16_t	u[3];

	u[0] = (uint16_t)(c->op | 0x2000);
	u[1] = (uint16_t)c->y;
	if (c->op >= OP_ADD_INT_LIT8)
	{
		u[0] = c->op;
		u[1] = (uint16_t)(2 | ((c->y & 0xff) << 8));
	}
	u[2] = 0x0010;
	fk_play(u, 3, c);
}

void	fk_table(const t_case *t, void (*fn)(const t_case *))
{
	while (t->what)
		fn(t++);
}
