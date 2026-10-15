#include "proc_elf.h"
#include "velum/err.h"
#include "velum/libk.h"

int	elf_read(const t_elfimg *im, uint64_t off, void *dst, uint64_t n)
{
	if (off > im->size || n > im->size - off)
		return (E_INVAL);
	memcpy(dst, im->data + off, n);
	return (0);
}

static int	elf_check_ident(const t_elf64_ehdr *eh)
{
	if (eh->ident[0] != 0x7f || eh->ident[1] != 'E')
		return (E_INVAL);
	if (eh->ident[2] != 'L' || eh->ident[3] != 'F')
		return (E_INVAL);
	if (eh->ident[4] != 2 || eh->ident[5] != 1 || eh->ident[6] != 1)
		return (E_INVAL);
	if (eh->version != 1)
		return (E_INVAL);
	return (0);
}

static int	elf_check_ehdr(const t_elfimg *im, const t_elf64_ehdr *eh)
{
	uint64_t	ph_bytes;

	if (elf_check_ident(eh) < 0)
		return (E_INVAL);
	if (eh->type != ELF_ET_EXEC && eh->type != ELF_ET_DYN)
		return (E_INVAL);
	if (eh->machine != ELF_EM_X86_64 || eh->ehsize != sizeof(t_elf64_ehdr))
		return (E_INVAL);
	if (eh->phentsize != sizeof(t_elf64_phdr))
		return (E_INVAL);
	if (eh->phnum == 0 || eh->phnum > ELF_PHNUM_MAX)
		return (E_INVAL);
	ph_bytes = (uint64_t)eh->phnum * sizeof(t_elf64_phdr);
	if ((eh->phoff & 7) != 0 || eh->phoff < sizeof(t_elf64_ehdr))
		return (E_INVAL);
	if (eh->phoff > im->size || ph_bytes > im->size - eh->phoff)
		return (E_INVAL);
	return (0);
}

int	elf_validate(const void *image, uint64_t size, t_elfinfo *out)
{
	t_elfimg		im;
	t_elf64_ehdr	eh;
	int				rc;

	if (!image || !out)
		return (E_INVAL);
	memset(out, 0, sizeof(*out));
	if (size < sizeof(eh) || size > ELF_FILE_MAX)
		return (E_INVAL);
	im.data = image;
	im.size = size;
	memcpy(&eh, image, sizeof(eh));
	rc = elf_check_ehdr(&im, &eh);
	if (rc < 0)
		return (rc);
	out->type = eh.type;
	out->entry = eh.entry;
	out->phoff = eh.phoff;
	out->phnum = eh.phnum;
	rc = elf_scan_phdrs(&im, &eh, out);
	if (rc == 0)
		rc = elf_check_final(out);
	if (rc == 0 && out->has_dyn)
		rc = elf_check_dynamic(&im, out);
	return (rc);
}
