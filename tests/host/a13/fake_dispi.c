#include "fakes.h"
#include "dispi.h"
#include "velum/err.h"

t_fd	g_fd;

void	fake_dispi_reset(void)
{
	memset(&g_fd, 0, sizeof(g_fd));
	g_fd.regs[DISPI_INDEX_ID] = DISPI_ID_WRITE;
	g_fd.regs[DISPI_INDEX_XRES] = 1024;
	g_fd.regs[DISPI_INDEX_YRES] = 768;
	g_fd.regs[DISPI_INDEX_BPP] = 32;
	g_fd.regs[DISPI_INDEX_ENABLE] = DISPI_ON;
	g_fd.regs[DISPI_INDEX_VIRT_WIDTH] = 1024;
	g_fd.vram64k = 256;
	g_fd.bar_bytes = 16ull * 1024 * 1024;
	g_fd.present = true;
}

uint16_t	fake_dispi_read(void *ctx, uint16_t idx)
{
	(void)ctx;
	if (idx == DISPI_INDEX_ID && g_fd.id_reply)
		return ((uint16_t)g_fd.id_reply);
	if (idx == DISPI_INDEX_VRAM64K)
		return ((uint16_t)g_fd.vram64k);
	if (idx > DISPI_INDEX_VRAM64K)
		return (0xffff);
	return (g_fd.regs[idx]);
}

static void	enable_write(uint16_t val)
{
	if (g_fd.reject_enable || g_fd.regs[DISPI_INDEX_XRES] == g_fd.reject_w)
		return ;
	g_fd.regs[DISPI_INDEX_ENABLE] = val;
	if (!(val & 1))
		return ;
	g_fd.regs[DISPI_INDEX_VIRT_WIDTH] = g_fd.regs[DISPI_INDEX_XRES];
	if (g_fd.virt_override)
		g_fd.regs[DISPI_INDEX_VIRT_WIDTH] = (uint16_t)g_fd.virt_override;
	if (g_fd.clamp_height)
		g_fd.regs[DISPI_INDEX_YRES] -= 8;
}

void	fake_dispi_write(void *ctx, uint16_t idx, uint16_t val)
{
	(void)ctx;
	if (g_fd.nlog < FAKE_DISPI_LOG)
	{
		g_fd.log[g_fd.nlog][0] = idx;
		g_fd.log[g_fd.nlog][1] = val;
	}
	g_fd.nlog++;
	if (idx >= DISPI_INDEX_VRAM64K)
		return ;
	if (idx == DISPI_INDEX_ID && (val < 45248 || val > DISPI_ID_WRITE))
		return ;
	if (idx == DISPI_INDEX_ENABLE && (val & 1))
		enable_write(val);
	else if (idx == DISPI_INDEX_ENABLE)
		g_fd.regs[idx] = val;
	else
		g_fd.regs[idx] = val;
}
