#include "../common/tbuf.h"
#include "../common/typo.h"
#include "logon_flow.h"
#include "logon_text.h"

static void	put_wait(t_tbuf *b, const char *lead, uint32_t seconds)
{
	tb_str(b, lead);
	tb_num(b, seconds, 1);
	tb_str(b, NBSP "s");
}

int	logon_message(const t_logonflow *f, uint32_t remaining_s, char *out,
		size_t size)
{
	t_tbuf	b;

	tb_init(&b, out, size);
	if (f->msg == LMSG_BAD)
	{
		tb_str(&b, "Échec de la connexion.");
		if (remaining_s)
			put_wait(&b, " Patientez ", remaining_s);
		else
			tb_str(&b, " Réessayez.");
	}
	else if (f->msg == LMSG_WAIT)
		put_wait(&b, "Trop d'échecs. Patientez ", f->wait_s);
	else if (f->msg == LMSG_ERROR)
		tb_str(&b, "Ce compte n'est pas disponible.");
	else if (f->msg == LMSG_POWER)
		tb_str(&b, "Impossible d'éteindre l'ordinateur.");
	else if (f->msg == LMSG_EMPTY)
		tb_str(&b, "Aucun compte utilisateur n'est disponible.");
	return (tb_result(&b));
}
