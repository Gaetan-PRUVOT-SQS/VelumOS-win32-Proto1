#include "velum/err.h"
#include "velum/libk.h"
#include "accounts.h"

static int	parse_numbers(const t_field *f, t_account *a)
{
	if (acc_dec(&f[1], &a->iterations) < 0 || a->iterations < ACC_ITER_MIN
		|| a->iterations > ACC_ITER_MAX)
		return (E_INVAL);
	if (acc_dec(&f[4], &a->flags) < 0 || (a->flags & ~ACC_FLAGS_KNOWN))
		return (E_INVAL);
	return (0);
}

static int	parse_secrets(const t_field *f, t_account *a)
{
	int	n;

	n = acc_hex(&f[2], a->salt, ACC_SALT_MAX);
	if (n < ACC_SALT_MIN)
		return (E_INVAL);
	a->salt_len = (uint32_t)n;
	if (f[3].n != 2 * SHA256_LEN || acc_hex(&f[3], a->hash, SHA256_LEN) < 0)
		return (E_INVAL);
	return (0);
}

int	acc_parse_line(const char *line, size_t len, t_account *out)
{
	t_field	f[ACC_FIELDS];

	if (len > ACC_LINE_MAX || !acc_split(line, len, f))
		return (E_INVAL);
	memset(out, 0, sizeof(*out));
	if (!acc_name_ok(f[0].p, f[0].n) || parse_numbers(f, out) < 0
		|| parse_secrets(f, out) < 0)
	{
		memset(out, 0, sizeof(*out));
		return (E_INVAL);
	}
	memcpy(out->name, f[0].p, f[0].n);
	return (0);
}
