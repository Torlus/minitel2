#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "video/ts9347.h"
#include "imagedev/cartslot.h"

#define VIDEO_TAG		"ts9347"
#define CPU_TAG			"i80c52"
#define SCREEN_TAG		"screen"

class minitel2_state : public driver_device
{
public:
	minitel2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }
};


static TS9347_INTERFACE(minitel2_ts9347_interface)
{
	SCREEN_TAG,
	0x2000
};


static DEVICE_IMAGE_LOAD( minitel2_cart )
{
	running_machine &machine = image.device().machine();
	minitel2_state *state = machine.driver_data<minitel2_state>();
	address_space *program = machine.device(CPU_TAG)->memory().space(AS_PROGRAM);
	UINT8 *ptr = state->memregion(CPU_TAG)->base();
	UINT32 size;

	size = image.length();
	if (image.fread( ptr, size) != size)
		return IMAGE_INIT_FAIL;
	return IMAGE_INIT_PASS;
}

// may need AS_DATA as well..

// according to my reading of the mcs51 cpu source, AS_PROGRAM is for
// external RAM, AS_DATA for internal RAM, and AS_IO for the IO ports

static ADDRESS_MAP_START(minitel2_mem, AS_PROGRAM, 8, minitel2_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(minitel2_io, AS_IO, 8, minitel2_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x4000, 0x403f) AM_DEVREADWRITE(VIDEO_TAG, ts9347_device, register_read, register_write)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( minitel2 )
INPUT_PORTS_END

static MACHINE_RESET( minitel2 )
{
}

static MACHINE_CONFIG_START( minitel2, minitel2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(CPU_TAG, I80C52, XTAL_14_31818MHz)
	MCFG_CPU_PROGRAM_MAP(minitel2_mem)
	MCFG_CPU_IO_MAP(minitel2_io)

	MCFG_MACHINE_RESET(minitel2)

	/* video hardware */
	MCFG_TS9347_ADD( VIDEO_TAG, TS9347, minitel2_ts9347_interface )
	MCFG_TS9347_SCREEN_ADD_PAL( SCREEN_TAG )
	MCFG_SCREEN_UPDATE_DEVICE( VIDEO_TAG, ts9347_device, screen_update )

	/* "cartridge" */
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("minitel2_cart")
	MCFG_CARTSLOT_LOAD(minitel2_cart)

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( minitel2 )
	ROM_REGION( 0x10000, CPU_TAG, ROMREGION_ERASEFF )
ROM_END

/* Driver */
/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1985, minitel2,  0,       0,    minitel2,   minitel2,  0,    "France Telecom", "Minitel 2", GAME_TYPE_OTHER )
