#include "emu.h"
#include "ts9347.h"
#include "machine/devhelpr.h"

const device_type TS9347 = &device_creator<ts9347_device>;

// In B,G,R order
static const rgb_t ts9347_palette[TS9347_PALETTE_SIZE] =
{
	/*MAKE_RGB(0, 0, 0),
	MAKE_RGB(255, 0, 0),
	MAKE_RGB(0, 255, 0),
	MAKE_RGB(255, 255, 0),
	MAKE_RGB(0, 0, 255),
	MAKE_RGB(255, 0, 255),
	MAKE_RGB(0, 255, 255),
	MAKE_RGB(255, 255, 255),*/

	//GE http://www.mikekohn.net/file_formats/yuv_rgb_converter.php

	MAKE_RGB(0, 0, 0),
	MAKE_RGB(76, 76, 76),
	MAKE_RGB(149, 149, 149),
	MAKE_RGB(225, 225, 225),
	MAKE_RGB(29, 29, 29),
	MAKE_RGB(105, 105, 105),
	MAKE_RGB(178, 178, 178),
	MAKE_RGB(255, 255, 255),
};

/*
** initialize the palette
*/
PALETTE_INIT( ts9347 )
{
	logerror("ts9347 palette_init\n");
	palette_set_colors(machine, 0, ts9347_palette, TS9347_PALETTE_SIZE);
}

ts9347_device::ts9347_device( const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock )
	: device_t( mconfig, TS9347, "ts9347", tag, owner, clock )
{
}

READ8_MEMBER( ts9347_device::register_read )
{
	int regNo = offset & 0x7;
	UINT8 data = m_Status;

	if (regNo != 0)
		data = m_Reg[regNo];

	// logerror("ts9347 register_read regNo=%01X data=%02X\n", regNo, data);	
	return data;
}

WRITE8_MEMBER( ts9347_device::register_write )
{
	if ( (offset & 0xff) < 0x20 || (offset & 0xff) > 0x28 )
		return;
	int regNo = offset & 0x7;
	int XQR = offset & 0x8;

	m_Reg[regNo] = data;

	// logerror("ts9347 register_write regNo=%01X data=%02X\n", regNo, data);
	if (XQR)
		execute_command();
}

void ts9347_device::execute_command()
{
	UINT8 code = (m_Reg[0] >> 4) & 0xf;
	UINT8 param = m_Reg[0] & 0xf;

	UINT8 rnw = param & 0x8;
	UINT8 incr = param & 0x1;
	UINT8 reg = param & 0x7; // IND

	m_Status &= ~0x78; // clears S3 to S6

	logerror("ts9347 execute_command ");
	switch(code) {
	case 0x8:
		logerror("IND ");
		logerror( (rnw ? "read " : "write ") );
		switch(reg) {
		case 0:
			logerror("ROM *TODO*");
			break;
		case 1:
			if (rnw) {
				m_Reg[1] = m_TGS;
			} else {
				m_TGS = m_Reg[1];
			}
			logerror("TGS %02X", m_TGS);
			break;
		case 2:
			if (rnw) {
				m_Reg[1] = m_MAT;
			} else {
				m_MAT = m_Reg[1];
			}
			logerror("MAT %02X", m_MAT);
			break;
		case 3:
			if (rnw) {
				m_Reg[1] = m_PAT;
			} else {
				m_PAT = m_Reg[1];
			}
			logerror("PAT %02X", m_PAT);
			break;
		case 4:
			if (rnw) {
				m_Reg[1] = m_DOR;
			} else {
				m_DOR = m_Reg[1];
			}
			logerror("DOR %02X", m_DOR);
			break;
		case 7:
			if (rnw) {
				m_Reg[1] = m_ROR;
			} else {
				m_ROR = m_Reg[1];
			}
			logerror("ROR %02X", m_ROR);
			break;
		default:
			logerror("???");
			break;
		}
		break;

	case 0x0:
		if (param == 0x6) {
			logerror("CLL *TODO*");
		} else {
			logerror("TLM ");
			xfer( rnw, 0, C40_B24, incr );
		}
		break;
	case 0x2:
		logerror("TLA ");
		xfer( rnw, 1, C40_B24, incr );
		break;
	case 0x6:
		if (param == 0x6) {
			logerror("CLS *TODO*");
		} else {
			logerror("TSM ");
			xfer( rnw, 0, C40_B16, incr );
		}
		break;
	case 0x7:
		logerror("TSA ");
		xfer( rnw, 1, C40_B16, incr );
		break;
	case 0x4:
		logerror("KRS ");
		xfer( rnw, 0, C80_B8, incr );
		break;
	case 0x5:
		logerror("KRL ");
		xfer( rnw, 0, C80_B12, incr );
		break;
	case 0x3:
		if (param & 0x4) {
			logerror("TBA ");
			xfer( rnw, 1, RAW_B8, incr );
		} else {
			logerror("TBM ");
			xfer( rnw, 0, RAW_B8, incr );
		}
		break;
	case 0xD:
		logerror("MVB *TODO*");
		break;
	case 0xE:
		logerror("MVD *TODO*");
		break;
	case 0xF:
		logerror("MVT *TODO*");
		break;
	case 0x9:
		logerror("VSM/VRM/NOP *TODO* %02X %02X %02X", m_Reg[1], m_Reg[2], m_Reg[3]);
		break;
	case 0x0B:
		logerror("INY *TODO*");
		break;
	default:
		logerror("???");
		break;
	}
	logerror("\n");
}



void ts9347_device::xfer( UINT8 rnw, UINT8 aux, xfer_mode xm, UINT8 incr )
{
	UINT8 Ra, Rb;
	UINT8 D, B, X, Y;
	UINT16 addr;

	if (aux) {
		Ra = m_Reg[4];
		Rb = m_Reg[5];
	} else {
		Ra = m_Reg[6];
		Rb = m_Reg[7];
	}

	D = (Ra>>5) & 0x7;
	Y = Ra & 0x1f;
	// B = (Rb>>6) & 0x3;
	B = ((Rb>>7)<<1) | ((Rb>>5)&0x2);
	X = Rb & 0x3f;
	logerror("D=%01X B=%01X Y=%02X X=%02X %s ", D, B, Y, X, (rnw ? "read" : "write") );
	
	switch( xm ) {
	case C40_B24:
		addr = log2phy( (D<<2) | ((B&~1)), Y, X );
		if (rnw) {
			m_Reg[1] = m_vMem[addr & (m_vram_size-1)];
		} else {
			m_vMem[addr & (m_vram_size-1)] = m_Reg[1];
		}
		logerror("%04X=%02X ", addr, m_Reg[1]);
		
		addr = log2phy( (D<<2) | (((B&~1)+1) & 0x3), Y, X );
		if (rnw) {
			m_Reg[2] = m_vMem[addr & (m_vram_size-1)];
		} else {
			m_vMem[addr & (m_vram_size-1)] = m_Reg[2];
		}
		logerror("%04X=%02X ", addr, m_Reg[2]);
		
		addr = log2phy( (D<<2) | (((B&~1)+2) & 0x3), Y, X );
		if (rnw) {
			m_Reg[3] = m_vMem[addr & (m_vram_size-1)];
		} else {
			m_vMem[addr & (m_vram_size-1)] = m_Reg[3];
		}
		logerror("%04X=%02X ", addr, m_Reg[3]);
		break;
	
	case RAW_B8:
		addr = log2phy( (D<<2) | B, Y, X );
		if (rnw) {
			m_Reg[1] = m_vMem[addr & (m_vram_size-1)];
		} else {
			m_vMem[addr & (m_vram_size-1)] = m_Reg[1];
		}
		logerror("%04X=%02X ", addr, m_Reg[1]);

		break;
	default:
		logerror("*TODO* ");
		break;
	}

	if (X == 39) {
		if (aux) {
			m_Status |= (1<<4);	// LXa
		} else {
			m_Status |= (1<<5);	// LXm
		}
	}

	if (incr) {
		logerror("incr ");
		if (X == 39) {
			m_Status |= (1<<6);	// Al
			X = 0;
		} else {
			X++;
		}
		if ( (xm == RAW_B8) && (X == 0) ) {
			Y = (Y + 1) & 0x1f;
		}

		Ra = (D<<5) | Y;
		Rb = (B<<6) | X;
		if (aux) {
			m_Reg[4] = Ra;
			m_Reg[5] = Rb;
		} else {
			m_Reg[6] = Ra;
			m_Reg[7] = Rb;
		}
	}

}

UINT16 ts9347_device::log2phy( UINT8 Z, UINT8 Y, UINT8 X ) 
{
	// datasheet WTF
	UINT16 addr;

	addr = ((UINT16)(Z & 0x1f)) << 10; // common part (high) and b0 (bit 10)
	addr |= (X & 0x7); // common part (low)

	if (Y >= 8) {
		if (X & 0x20) {
			addr |= ( (Y & 0x7) << 5 );
			addr |= (Y & 0x18);
		} else {
			addr |= (Y << 5);
			addr |= X;		
		}
	} else {
		addr |= ( (X & 0x38) << 2 ); 
	}

	return addr;
}


void ts9347_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int raw_vpos = m_screen->vpos();
	int vpos = raw_vpos * m_vertical_size / m_screen->height();
	UINT16 *p = &m_tmpbmp.pix16(vpos);
	int y = vpos - m_top_border;

	logerror("ts9347 device_timer raw_vpos=%d vpos=%d\n", raw_vpos, vpos);

	/* Schedule next callback */
	m_line_timer->adjust( m_screen->time_until_pos( ( raw_vpos + 1 ) % m_screen->height() , 0 ) );
}


UINT32 ts9347_device::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	UINT8 ntZ, ntD, ntB, ntY, ntX; // nametable
	UINT8 ptZ, ptY, ptX; // pattern table
	UINT8 C, B, A, D, SN;

	UINT8 x, y;
	UINT16 addr;
	int sx, sy;
	UINT16 *p;
	UINT8 marginCol, bgCol, fgCol;

	logerror("ts9347 screen_update begin\n");

	marginCol = m_MAT & 0x7;
	//marginCol = 0x1;
	// Top margin
	for(sy = TS9347_VERT_DISPLAY_START_PAL - TS9347_TOP_MARGIN; sy < TS9347_VERT_DISPLAY_START_PAL; sy++) {
		p = &m_tmpbmp.pix16(sy);
		for(sx = TS9347_HORZ_DISPLAY_START - TS9347_HORZ_MARGIN; sx < TS9347_HORZ_DISPLAY_START + 320 + TS9347_HORZ_MARGIN; sx++) {
			p[sx] = marginCol;
		}
	}
	// Bottom margin
	for(sy = TS9347_VERT_DISPLAY_START_PAL + 250; sy < TS9347_VERT_DISPLAY_START_PAL + 250 + TS9347_BOTTOM_MARGIN; sy++) {
		p = &m_tmpbmp.pix16(sy);
		for(sx = TS9347_HORZ_DISPLAY_START - TS9347_HORZ_MARGIN; sx < TS9347_HORZ_DISPLAY_START + 320 + TS9347_HORZ_MARGIN; sx++) {
			p[sx] = marginCol;
		}
	}
	// Left and right margins
	for(sy = TS9347_VERT_DISPLAY_START_PAL; sy < TS9347_VERT_DISPLAY_START_PAL + 250; sy++) {
		p = &m_tmpbmp.pix16(sy);
		for(sx = TS9347_HORZ_DISPLAY_START - TS9347_HORZ_MARGIN; sx < TS9347_HORZ_DISPLAY_START; sx++) {
			p[sx] = marginCol;
		}
		for(sx = TS9347_HORZ_DISPLAY_START + 320; sx < TS9347_HORZ_DISPLAY_START + 320 + TS9347_HORZ_MARGIN; sx++) {
			p[sx] = marginCol;
		}
	}


	// Base page address
	ntZ = ((m_DOR & 0x80) >> 3) | ((m_ROR & 0xe0) >> 4);
	ntD = ntZ >> 2;
	// ntB = ntZ & 0x3;
	ntB = ((ntZ>>7)<<1) | ((ntZ>>5)&0x2);

	// Assuming 40 cols and long codes, ditching service row as well 
	for(y = 0; y < 24; y++) {
		ntY = ((m_ROR & 0x1f) + y) & 0x1f;
		for(x = 0; x < 40; x++) {
			ntX = x;
			addr = log2phy( (ntD<<2) | ntB, ntY, ntX);
			C = m_vMem[addr & (m_vram_size-1)] & 0x7f;
			// logerror("(nt) Z=%02X (D=%01X B=%01X) Y=%02X X=%02X %04X=%02X\n", (ntD<<2) | ntB, ntD, ntB, ntY, ntX, addr, C);
			// logerror("%c", (C < 0x20 ? '.' : C));
			addr = log2phy( (ntD<<2) | ((ntB+1)&0x3) , ntY, ntX );
			B = m_vMem[addr & (m_vram_size-1)];
			// logerror("(nt) Z=%02X (D=%01X B=%01X) Y=%02X X=%02X %04X=%02X\n", (ntD<<2) | ((ntB+1)&0x3), ntD, ((ntB+1)&0x3), ntY, ntX, addr, B);
			addr = log2phy( (ntD<<2) | ((ntB+2)&0x3) , ntY, ntX );
			A = m_vMem[addr & (m_vram_size-1)];
			// logerror("(nt) Z=%02X (D=%01X B=%01X) Y=%02X X=%02X %04X=%02X\n", (ntD<<2) | ((ntB+2)&0x3), ntD, ((ntB+2)&0x3), ntY, ntX, addr, A);

			if (A & 0x80) { // Negative
				fgCol = (A & 0x7);
				bgCol = ((A>>4) & 0x7); 
			} else {
				bgCol = (A & 0x7);
				fgCol = ((A>>4) & 0x7); 
			}
			if (B & 0x1) {
				bgCol = 0;
			}


			if (B & 0x80) {
				if (B & 0x40) { // Q0-Q7 character sets
					ptZ = ((m_DOR & 0x80)>>3) | (1<<3) | ((B & 0x20)>>3) | ((B & 0x8)>>2) | ((B & 0x10)>>4);
				} else {
					if (B & 0x20) {
						ptZ = ((m_DOR & 0xf0)>>3) | ((B & 0x10)>>4); 
					} else {
						ptZ = ((m_DOR & 0x80)>>3) | (m_DOR & 0xe); // G'0 character set
					}
				}
				for( SN = 0; SN < 10; SN++) {
					ptX = (SN<<2) | (C & 0x3);
					ptY = (C>>2); 
					addr = log2phy( ptZ, ptY, ptX );
					D = m_vMem[addr & (m_vram_size-1)];
					logerror("  (pt) Z=%02X (D=%01X B=%01X) Y=%02X X=%02X %04X=%02X\n", ptZ, (ptZ>>2), ptZ & 0x3, ptY, ptX, addr, D);
	
					sy = TS9347_VERT_DISPLAY_START_PAL + (y*10) + SN;
					p = &m_tmpbmp.pix16(sy);
					p += TS9347_HORZ_DISPLAY_START + (x*8);
					for( sx = 0; sx < 8; sx++) {
						if (D & 1) {
							*p = fgCol;
						} else {
							*p = bgCol;
						}
						p++;
						D >>= 1;
					}
				}

			} else {
				// Internal ROM, *TODO*
				for( SN = 0; SN < 10; SN++) {
					sy = TS9347_VERT_DISPLAY_START_PAL + (y*10) + SN;
					p = &m_tmpbmp.pix16(sy);
					p += TS9347_HORZ_DISPLAY_START + (x*8);
					for( sx = 0; sx < 8; sx++) {
						if (C != 0x20) {
							*p = fgCol;
						} else {
							*p = bgCol;
						}
						p++;
					}
				}
			}


		} // for(x)
		logerror("\n");
	} /// for(y)

	copybitmap( bitmap, m_tmpbmp, 0, 0, 0, 0, cliprect );

	logerror("ts9347 screen_update end\n");
	return 0;
}


void ts9347_device::device_config_complete()
{
	const ts9347_interface *intf = reinterpret_cast<const ts9347_interface *>(static_config());

	if ( intf != NULL )
	{
		*static_cast<ts9347_interface *>(this) = *intf;
	}
	else
	{
		m_vram_size = 0;
	}
}


void ts9347_device::device_start()
{
	assert_always(((m_vram_size == 0x1000) || (m_vram_size == 0x2000) || (m_vram_size == 0x4000)), "4, 8 or 16 kB vram please");

	m_screen = machine().device<screen_device>( m_screen_tag );
	assert( m_screen != NULL );

	m_top_border = TS9347_VERT_DISPLAY_START_PAL;
	m_vertical_size = TS9347_TOTAL_VERT_PAL;

	/* Video RAM */
	m_vMem = auto_alloc_array_clear(machine(), UINT8, m_vram_size);

	/* back bitmap */
	m_tmpbmp.allocate(TS9347_TOTAL_HORZ, TS9347_TOTAL_VERT_PAL);

	m_line_timer = timer_alloc(TIMER_LINE);
}


void ts9347_device::device_reset()
{
	logerror("ts9347 device_reset\n");

	m_Status = 0;
	for(int i = 0; i < 8; i++) {
		m_Reg[i] = 0;
	}
	m_ROR = 0;
	m_DOR = 0;
	m_PAT = 0;
	m_MAT = 0;
	m_TGS = 0;

	// m_line_timer->adjust( m_screen->time_until_pos( 0, 0 ) );
}

