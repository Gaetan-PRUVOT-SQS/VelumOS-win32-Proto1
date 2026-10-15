#include "velum/err.h"
#include "velum/klog.h"
#include "velum/timer.h"
#include "virtio.h"

static bool	status_zero(void *ctx)
{
	t_vio	*v;

	v = ctx;
	return (vio_rd8(v->common, VIO_CC_STATUS) == 0);
}

int	vio_reset(t_vio *v)
{
	vio_wr8(v->common, VIO_CC_STATUS, 0);
	if (wait_until(status_zero, v, VIO_RESET_TIMEOUT_NS) < 0)
	{
		klog_err("virtio: remise à zéro sans réponse");
		return (E_IO);
	}
	return (0);
}

void	vio_status_add(t_vio *v, uint8_t bits)
{
	uint8_t	st;

	st = vio_rd8(v->common, VIO_CC_STATUS);
	vio_wr8(v->common, VIO_CC_STATUS, st | bits);
}

int	vio_negotiate(t_vio *v, uint64_t wanted)
{
	uint64_t	offered;

	vio_status_add(v, VIO_ST_ACK);
	vio_status_add(v, VIO_ST_DRIVER);
	vio_wr32(v->common, VIO_CC_DFSELECT, 0);
	offered = vio_rd32(v->common, VIO_CC_DF);
	vio_wr32(v->common, VIO_CC_DFSELECT, 1);
	offered |= (uint64_t)vio_rd32(v->common, VIO_CC_DF) << 32;
	if (!(offered & VIO_F_VERSION_1))
	{
		klog_warn("virtio: VERSION_1 absent, mode legacy non pris en charge");
		vio_status_add(v, VIO_ST_FAILED);
		return (E_NOTSUP);
	}
	v->features = offered & (wanted | VIO_F_VERSION_1);
	vio_wr32(v->common, VIO_CC_GFSELECT, 0);
	vio_wr32(v->common, VIO_CC_GF, (uint32_t)v->features);
	vio_wr32(v->common, VIO_CC_GFSELECT, 1);
	vio_wr32(v->common, VIO_CC_GF, (uint32_t)(v->features >> 32));
	vio_status_add(v, VIO_ST_FEATURES_OK);
	if (vio_rd8(v->common, VIO_CC_STATUS) & VIO_ST_FEATURES_OK)
		return (0);
	klog_warn("virtio: fonctions refusées par l'appareil");
	vio_status_add(v, VIO_ST_FAILED);
	return (E_NOTSUP);
}

int	vio_start(t_vio *v)
{
	uint8_t	st;

	vio_status_add(v, VIO_ST_DRIVER_OK);
	st = vio_rd8(v->common, VIO_CC_STATUS);
	if (st & (VIO_ST_NEEDS_RESET | VIO_ST_FAILED))
		return (E_IO);
	return (0);
}
