#include <stdio.h>
#include <string.h>
#include "harness.h"
#include "blk_int.h"
#include "fake.h"

static t_fkram	g_img;

static int	hdr_try(const t_fkedit *e, uint32_t ss, uint64_t ns)
{
	uint8_t		buf[512];
	t_gptgeo	g;
	t_gpthdr	h;

	memcpy(buf, g_img.data + 512, sizeof(buf));
	if (e->width == 1)
		buf[e->off] = (uint8_t)e->val;
	else if (e->width == 4)
		fk_le32(buf + e->off, (uint32_t)e->val);
	else if (e->width == 8)
		fk_le64(buf + e->off, e->val);
	else
		buf[e->off] ^= 1;
	fk_le32(buf + 16, 0);
	if (e->width && rd_le32(buf + 12) <= sizeof(buf))
		fk_le32(buf + 16, crc32_calc(buf, rd_le32(buf + 12)));
	if (!e->width)
		memcpy(buf + 16, g_img.data + 512 + 16, 4);
	g.ss = ss;
	g.nsectors = ns;
	g.lba = 1;
	return (gpt_header(buf, &g, &h));
}

static void	hdr_table(void)
{
	static const t_fkedit	edits[] = {
	{60, 1, 0x5c, 0}, {0, 1, 'X', E_PROTO}, {8, 4, 0x00020000, E_PROTO},
	{8, 4, 0x00010001, 0}, {12, 4, 91, E_PROTO}, {12, 4, 513, E_PROTO},
	{12, 4, 512, 0}, {20, 4, 1, E_PROTO}, {24, 8, 2, E_PROTO},
	{32, 8, 8192, E_PROTO}, {32, 8, 1, E_PROTO}, {40, 8, 9000, E_PROTO},
	{48, 8, 8192, E_PROTO}, {72, 8, 0, E_PROTO}, {72, 8, 8170, E_PROTO},
	{72, 8, 40, E_PROTO}, {72, 8, 1, E_PROTO}, {80, 4, 0, E_PROTO},
	{80, 4, 4097, E_PROTO}, {80, 4, 0xffffffff, E_PROTO}, {84, 4, 0, E_PROTO},
	{84, 4, 100, E_PROTO}, {84, 4, 192, E_PROTO}, {84, 4, 8192, E_PROTO},
	{84, 4, 256, E_PROTO}, {60, 0, 0, E_PROTO}, {0, 9, 0, 0}};
	int						i;
	char					what[64];

	i = 0;
	while (edits[i].width != 9)
	{
		snprintf(what, sizeof(what), "en-tete cas %d (off %u)", i,
			edits[i].off);
		h_eq_i64(what, hdr_try(&edits[i], 512, 8192), edits[i].want);
		i++;
	}
}

static void	hdr_geometry(void)
{
	static const t_fkedit	same = {60, 1, 0x5c, 0};

	h_eq_i64("taille de bloc 256", hdr_try(&same, 256, 8192), E_INVAL);
	h_eq_i64("taille de bloc 8192", hdr_try(&same, 8192, 8192), E_INVAL);
	h_eq_i64("disque de 2 secteurs", hdr_try(&same, 512, 2), E_INVAL);
	h_eq_i64("disque plus petit que l'en-tete", hdr_try(&same, 512, 4096),
		E_PROTO);
}

int	main(void)
{
	static const t_fkpart	p[1] = {{2048, 4095, 0}};

	fk_ram_init(&g_img, "img", 512, 8192);
	fk_gpt_build(&g_img, p, 1);
	h_begin("a11/gpt_entete");
	h_run("table de corruptions", hdr_table);
	h_run("geometrie", hdr_geometry);
	fk_ram_free(&g_img);
	return (h_end());
}
