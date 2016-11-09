// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    pci9050.h - PLX PCI9050 PCI to 4x Local Bus Bridge

    by R. Belmont

*********************************************************************/

#ifndef _PCI9050_H
#define _PCI9050_H

#include "machine/pci.h"

#define MCFG_PCI9050_ADD(_tag) \
	MCFG_PCI_DEVICE_ADD(_tag, PCI9050, 0x10b59050, 0x01, 0x06800000, 0x10b59050)

#define MCFG_PCI9050_SET_MAP(id, map) \
	downcast<pci9050_device *>(device)->set_map(id, ADDRESS_MAP_NAME(map), #map, owner);

#define MCFG_PCI9050_USER_INPUT_CALLBACK(_write) \
	devcb = &pci9050_device::set_user_input_callback(*device, DEVCB_##_write);

#define MCFG_PCI9050_USER_OUTPUT_CALLBACK(_read) \
	devcb = &pci9050_device::set_user_output_callback(*device, DEVCB_##_read);

class pci9050_device :
	public pci_device
{
public:
	pci9050_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// PCI9050 I/O register space handlers
	DECLARE_READ32_MEMBER( lasrr_r  );
	DECLARE_WRITE32_MEMBER(lasrr_w  );
	DECLARE_READ32_MEMBER( eromrr_r );
	DECLARE_WRITE32_MEMBER(eromrr_w );
	DECLARE_READ32_MEMBER( lasba_r  );
	DECLARE_WRITE32_MEMBER(lasba_w  );
	DECLARE_READ32_MEMBER( eromba_r );
	DECLARE_WRITE32_MEMBER(eromba_w );
	DECLARE_READ32_MEMBER( lasbrd_r );
	DECLARE_WRITE32_MEMBER(lasbrd_w );
	DECLARE_READ32_MEMBER( erombrd_r);
	DECLARE_WRITE32_MEMBER(erombrd_w);
	DECLARE_READ32_MEMBER( csbase_r );
	DECLARE_WRITE32_MEMBER(csbase_w );
	DECLARE_READ32_MEMBER( intcsr_r );
	DECLARE_WRITE32_MEMBER(intcsr_w );
	DECLARE_READ32_MEMBER( cntrl_r  );
	DECLARE_WRITE32_MEMBER(cntrl_w  );

	template<class _Object> static devcb_base &set_user_input_callback(device_t &device, _Object object) { return downcast<pci9050_device &>(device).m_user_input_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_user_output_callback(device_t &device, _Object object) { return downcast<pci9050_device &>(device).m_user_output_handler.set_callback(object); }

	void set_map(int id, address_map_constructor map, const char *name, device_t *device);

protected:
	virtual void device_start() override;
	virtual void device_config_complete() override;
	virtual void device_reset() override;

private:
	DECLARE_ADDRESS_MAP(map, 32);
	DECLARE_ADDRESS_MAP(empty, 32);

	const char *m_names[4];
	device_t *m_devices[4];
	address_map_constructor m_maps[4];

	uint32_t m_lasrr[4], m_lasba[4], m_lasbrd[4], m_csbase[4];
	uint32_t m_eromrr, m_eromba, m_erombrd, m_intcsr, m_cntrl;

	void remap_local(int id);
	void remap_rom();

	template<int id> void map_trampoline(address_map &map) {
		m_maps[id](map);
	}

	devcb_read32 m_user_input_handler;
	devcb_write32 m_user_output_handler;

};

extern const device_type PCI9050;

#endif
