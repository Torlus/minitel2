#ifndef __TS9347_H__
#define __TS9347_H__

#include "emu.h"
#include "machine/devhelpr.h"


#define TS9347_PALETTE_SIZE				8


/* Some defines used in defining the screens */
#define TS9347_TOTAL_HORZ					512		// 768 * 2 / 3
#define TS9347_TOTAL_VERT_PAL				312

#define TS9347_HORZ_MARGIN					48		// 72 * 2 / 3
#define TS9347_TOP_MARGIN					16
#define TS9347_BOTTOM_MARGIN				18

#define TS9347_HORZ_DISPLAY_START			( 80 + TS9347_HORZ_MARGIN )	// 80 = 120 * 2 / 3
#define TS9347_VERT_DISPLAY_START_PAL		( 25 + TS9347_TOP_MARGIN )


#define TS9347_INTERFACE(name) \
	const ts9347_interface (name) =


#define MCFG_TS9347_ADD(_tag, _variant, _config) \
	MCFG_DEVICE_ADD(_tag, _variant, XTAL_14_31818MHz * 2 / 3 ) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_PALETTE_LENGTH(TS9347_PALETTE_SIZE) \
	MCFG_PALETTE_INIT(ts9347) \

#define MCFG_TS9347_SCREEN_ADD_PAL(_screen_tag) \
	MCFG_SCREEN_ADD(_screen_tag, RASTER ) \
	MCFG_SCREEN_RAW_PARAMS( XTAL_14_31818MHz * 2 / 3, TS9347_TOTAL_HORZ, TS9347_HORZ_DISPLAY_START - TS9347_HORZ_MARGIN, TS9347_HORZ_DISPLAY_START + 320 + TS9347_HORZ_MARGIN,	\
		 TS9347_TOTAL_VERT_PAL, TS9347_VERT_DISPLAY_START_PAL - TS9347_TOP_MARGIN, TS9347_VERT_DISPLAY_START_PAL + 250 + TS9347_BOTTOM_MARGIN)

extern const device_type TS9347;

typedef struct _ts9347_interface ts9347_interface;
struct _ts9347_interface
{
	const char			*m_screen_tag;
	int					m_vram_size;	/* 4K, 8K, or 16K. This should be replaced by fetching data from an address space? */
};

PALETTE_INIT( ts9347 );

enum xfer_mode {
	C40_B24,
	C40_B16,
	C80_B12,
	C80_B8,
	RAW_B8
};

class ts9347_device :	public device_t,
						public ts9347_interface
{
public:
	// construction/destruction
	ts9347_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( register_read );
	DECLARE_WRITE8_MEMBER( register_write );

	/* update the screen */
	UINT32 screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	bitmap_ind16 &get_bitmap() { return m_tmpbmp; }

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_0) ? &m_space_config : NULL; }

private:
	void execute_command();
	void xfer( UINT8 rnw, UINT8 aux, xfer_mode xm, UINT8 incr );
	UINT16 log2phy( UINT8 Z, UINT8 Y, UINT8 X );

	static const device_timer_id TIMER_LINE = 0;

	screen_device	*m_screen;

	/* registers */
	UINT8	m_Status;
	UINT8	m_Reg[8];
	UINT8	m_ROR;
	UINT8	m_DOR;
	UINT8	m_PAT;
	UINT8	m_MAT;
	UINT8	m_TGS;

	/* memory */
	const address_space_config		m_space_config;

	UINT8		*m_vMem;
	bitmap_ind16 m_tmpbmp;
	emu_timer	*m_line_timer;

	/* emulation settings */
	int			m_top_border;
	int			m_vertical_size;
};

#endif
