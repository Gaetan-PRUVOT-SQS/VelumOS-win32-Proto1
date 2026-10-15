#include "vtest.h"

static void	files_read(void)
{
	int64_t	f;
	uint8_t	h[4];
	t_vstat	st;

	f = v_open("/system/bin/vtest", O_RDONLY, 0);
	vtest_check(f > 0, "fichiers: open de l'image de vtest");
	vtest_check(v_read((t_handle)f, h, 4) == 4 && h[0] == 0x7f && h[1] == 'E'
		&& h[2] == 'L' && h[3] == 'F', "fichiers: magic ELF");
	vtest_check(v_seek((t_handle)f, 0, SEEK_SET_) == 0
		&& v_pread((t_handle)f, h, 4, 1) == 4 && h[0] == 'E',
		"fichiers: seek et pread");
	vtest_check(v_fstat((t_handle)f, &st) == 0 && st.size > 1000,
		"fichiers: fstat");
	vtest_check(v_close((t_handle)f) == 0, "fichiers: close");
	vtest_check(v_stat("/system/bin/vtest", &st) == 0
		&& (st.mode & S_TYPE_MASK) == S_TYPE_REG, "fichiers: stat");
}

static void	files_errors(void)
{
	vtest_check(v_open("/absent/fichier", O_RDONLY, 0) < 0,
		"fichiers: chemin absent refuse");
	vtest_check(v_open("relatif", O_RDONLY, 0) < 0,
		"fichiers: chemin relatif refuse");
	vtest_check(v_open("/system/bin/vtest", O_WRONLY, 0) < 0,
		"fichiers: ecriture refusee sur l'initrd");
	vtest_check(v_open(NULL, O_RDONLY, 0) == E_FAULT,
		"fichiers: chemin nul");
}

static void	files_dir(void)
{
	int64_t		d;
	t_dirent	e[4];

	d = v_open("/system/bin", O_RDONLY | O_DIRECTORY, 0);
	vtest_check(d > 0, "fichiers: open de dossier");
	vtest_check(v_readdir((t_handle)d, e, 4) > 0, "fichiers: readdir");
	v_close((t_handle)d);
}

static void	files_section(void)
{
	int64_t	s;
	int64_t	a;
	int64_t	b;
	uint8_t	*pa;
	uint8_t	*pb;

	s = v_section_create(8192, PROT_R | PROT_W);
	a = v_section_map((t_handle)s, 0, 8192, PROT_R | PROT_W);
	vtest_check(s > 0 && a > 0, "sections: creation et mappage");
	pa = (uint8_t *)(uintptr_t)a;
	pa[0] = 0xab;
	pa[8191] = 0xcd;
	b = v_section_map((t_handle)s, 0, 8192, PROT_R);
	pb = (uint8_t *)(uintptr_t)b;
	vtest_check(b > 0 && pb[0] == 0xab && pb[8191] == 0xcd,
		"sections: second mappage voit les ecritures");
	vtest_check(v_section_map((t_handle)s, 0, 8192, PROT_W | PROT_X) < 0,
		"sections: W|X refuse");
	v_vfree((uint64_t)a, 8192);
	v_vfree((uint64_t)b, 8192);
	v_close((t_handle)s);
}

void	vtest_files(void)
{
	files_read();
	files_errors();
	files_dir();
	files_section();
}
