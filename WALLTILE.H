// Filename   : WALLTILE.H
// Description: constant for wall tiles
// Ownership  : Gilbert


#ifndef __WALLTILE_H
#define __WALLTILE_H

// North gate, west tower
#define NGATE_WTOWER_NW 0x01
#define NGATE_WTOWER_NE 0x02
#define NGATE_WTOWER_SW 0x03
#define NGATE_WTOWER_SE 0x04

// North gate, east tower
#define NGATE_ETOWER_NW 0x05
#define NGATE_ETOWER_NE 0x06
#define NGATE_ETOWER_SW 0x07
#define NGATE_ETOWER_SE 0x08

// South gate, east tower
#define SGATE_WTOWER_NW 0x09
#define SGATE_WTOWER_NE 0x0a
#define SGATE_WTOWER_SW 0x0b
#define SGATE_WTOWER_SE 0x0c

// South gate, west tower
#define SGATE_ETOWER_NW 0x0d
#define SGATE_ETOWER_NE 0x0e
#define SGATE_ETOWER_SW 0x0f
#define SGATE_ETOWER_SE 0x10

// West gate, north tower
#define WGATE_NTOWER_NW 0x11
#define WGATE_NTOWER_NE 0x12
#define WGATE_NTOWER_SW 0x13
#define WGATE_NTOWER_SE 0x14

// West gate, south tower
#define WGATE_STOWER_NW 0x15
#define WGATE_STOWER_NE 0x16
#define WGATE_STOWER_SW 0x17
#define WGATE_STOWER_SE 0x18

// East gate, north tower
#define EGATE_NTOWER_NW 0x19
#define EGATE_NTOWER_NE 0x1a
#define EGATE_NTOWER_SW 0x1b
#define EGATE_NTOWER_SE 0x1c

// East gate, south tower
#define EGATE_STOWER_NW 0x1d
#define EGATE_STOWER_NE 0x1e
#define EGATE_STOWER_SW 0x1f
#define EGATE_STOWER_SE 0x20

// Guard tower
#define NWTOWER 0x21
#define NETOWER 0x22
#define SWTOWER 0x23
#define SETOWER 0x24
#define NTOWER 0x21
#define STOWER 0x23
#define WTOWER 0x23
#define ETOWER 0x24
#define SINGLE_TOWER 0x23

// Wall tiles
#define NSWALL 0x25
#define NSWALL_SHADOW 0x26
#define EWWALL 0x27
#define EWWALL_SHADOW 0x28

// constructing wall tiles
#define NSWALL_CON1 0x51
#define NSWALL_CON2 0x52
#define NSWALL_CON3 0x53
#define EWWALL_CON1 0x54
#define EWWALL_CON2 0x55
#define EWWALL_CON3 0x56
#define TOWER_CON1 0x57
#define TOWER_CON2 0x58
#define TOWER_CON3 0x59

// destructing wall tiles
#define NSWALL_DES1 0x5a
#define NSWALL_DES2 0x5b
#define NSWALL_DES3 0x5c
#define EWWALL_DES1 0x5d
#define EWWALL_DES2 0x5e
#define EWWALL_DES3 0x5f
#define TOWER_DES1 0x60
#define TOWER_DES2 0x61
#define TOWER_DES3 0x62

inline int is_wall_rubble(int w)
{
	return (w >= 0x51 && w <= 0x62);
}

// Gate tiles

/*inline int is_gate(char wallId)
{
	return wallId > 0x30;
}
*/

// north gate 31-38
#define NGATE_BASE 0x31
#define NGATE_W 0x35
#define NGATE_E 0x38

// south gate 39-40
#define SGATE_BASE 0x39
#define SGATE_W 0x39
#define SGATE_E 0x3c

// west gate 41-48
#define WGATE_BASE 0x41
#define WGATE_N 0x42
#define WGATE_S 0x48

// east gate 49-50
#define EGATE_BASE 0x49
#define EGATE_N 0x49
#define EGATE_S 0x4f


#define GATE_LENGTH 8
#define GATE_WIDTH 2

#endif