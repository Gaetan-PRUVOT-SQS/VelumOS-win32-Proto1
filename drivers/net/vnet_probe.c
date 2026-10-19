#include "velum/err.h"
#include "velum/heap.h"
#include "velum/klog.h"
#include "velum/libk.h"
#include "velum/pmm.h"
#include "velum/random.h"
#include "velum/util.h"
#include "velum/vmm.h"
#include "vnet.h"

static t_vnet	*g_vnet;

static const t_nicops	g_vnet_ops = {vnet_send, vnet_recv};

static int	vnet_mac_read(t_vnet *v)
{
	int	rc;

	rc = vio_cfg_read(&v->vio, VNET_CFG_MAC, v->mac, 4);
	if (rc == 0)
		rc = vio_cfg_read(&v->vio, VNET_CFG_MAC + 4, v->mac + 4, 2);
	return (rc);
}

static int	vnet_mac(t_vnet *v)
{
	if (v->vio.features & VNET_F_MAC)
		return (vnet_mac_read(v));
	krandom(v->mac, 6);
	v->mac[0] = (uint8_t)((v->mac[0] & 0xfe) | 0x02);
	return (0);
}

static int	vnet_hw_init(t_vnet *v, const t_pcidev *d)
{
	int	rc;

	rc = vio_find_caps(d, &v->vio);
	if (rc == 0 && pci_enable(d, PCI_CMD_MEM | PCI_CMD_MASTER) < 0)
		rc = E_IO;
	if (rc == 0 && pci_intx_disable(d) < 0)
		klog_warn("net: INTx non coupée");
	if (rc == 0)
		rc = vio_map(&v->vio);
	if (rc == 0)
		rc = vio_reset(&v->vio);
	if (rc == 0)
		rc = vio_negotiate(&v->vio, VNET_WANTED);
	if (rc == 0)
		rc = vnet_mac(v);
	return (rc);
}

static int	vnet_dma_init(t_vnet *v)
{
	int	rc;

	rc = vio_queue_setup(&v->vio, VNET_QUEUE_RX, &v->rxq);
	if (rc == 0)
		rc = vio_queue_setup(&v->vio, VNET_QUEUE_TX, &v->txq);
	if (rc < 0)
		return (rc);
	if (v->rxq.size < VNET_RX_SLOTS || v->txq.size < 1)
		return (E_NOTSUP);
	v->dma_phys = pmm_alloc_pages(PMM_DMA, VNET_DMA_PAGES, 1, 0);
	if (!v->dma_phys)
		return (E_NOMEM);
	v->dma = phys_to_virt(v->dma_phys);
	memset(v->dma, 0, (size_t)VNET_DMA_PAGES * PAGE_SIZE);
	rc = vnet_fill_rx(v);
	if (rc == 0)
		rc = vio_start(&v->vio);
	return (rc);
}

static void	vnet_destroy(t_vnet *v, int rc)
{
	klog_warn("net: virtio-net refusée (%d)", rc);
	if (v->vio.common && vio_reset(&v->vio) < 0)
	{
		klog_err("net: remise à zéro impossible, DMA conservée");
		return ;
	}
	vio_queue_release(&v->rxq);
	vio_queue_release(&v->txq);
	if (v->dma_phys)
		pmm_free_pages(v->dma_phys, VNET_DMA_PAGES, PMM_DMA);
	kfree(v);
}

int	vnet_boot(void)
{
	const t_pcidev	*d;
	t_vnet			*v;
	int				rc;

	d = pci_find(VIO_VENDOR, VNET_PCI_DEVICE, 0);
	if (!d)
		return (E_NODEV);
	v = kmalloc_tag(sizeof(*v), HEAP_DRIVER);
	if (!v)
		return (E_NOMEM);
	memset(v, 0, sizeof(*v));
	rc = vnet_hw_init(v, d);
	if (rc == 0)
		rc = vnet_dma_init(v);
	if (rc == 0)
		rc = net_attach(&g_vnet_ops, v, v->mac);
	if (rc < 0)
	{
		vnet_destroy(v, rc);
		return (rc);
	}
	g_vnet = v;
	klog_info("net: virtio-net %02x:%02x:%02x:%02x:%02x:%02x", v->mac[0],
		v->mac[1], v->mac[2], v->mac[3], v->mac[4], v->mac[5]);
	return (0);
}
