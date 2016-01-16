// license:BSD-3-Clause
// copyright-holders:Carl
// TODO: dump 8048 mcu

#include "machine/m20_kbd.h"

m20_keyboard_device::m20_keyboard_device(const machine_config& mconfig, std::string tag, device_t* owner, UINT32 clock) :
	serial_keyboard_device(mconfig, M20_KEYBOARD, "M20 Keyboard", tag, owner, 0, "m20_keyboard", __FILE__)
{
}

void m20_keyboard_device::write(UINT8 data)
{
	switch(data)
	{
		case 0x03:
			send_key(2);
			break;
		case 0x80:
			send_key(0x80);
			break;
	}
	return;
}

UINT8 m20_keyboard_device::keyboard_handler(UINT8 last_code, UINT8 *scan_line)
{
	int i = *scan_line, j;
	UINT8 code = 0, state = m_state[i];

	if (i == 0) code = m_io_kbd0->read();
	else
	if (i == 1) code = m_io_kbd1->read();
	else
	if (i == 2) code = m_io_kbd2->read();
	else
	if (i == 3) code = m_io_kbd3->read();
	else
	if (i == 4) code = m_io_kbd4->read();
	else
	if (i == 5) code = m_io_kbd5->read();
	else
	if (i == 6) code = m_io_kbd6->read();
	else
	if (i == 7) code = m_io_kbd7->read();
	else
	if (i == 8) code = m_io_kbd8->read();

	*scan_line = (*scan_line + 1) % 9;

	if(m_state[i] == code)
		return 0;

	m_state[i] = code;
	code = (state ^ code) & code;

	if(!code)
		return 0;

	for(j = 0; j < 8; j++)
		if((code >> j) == 1)
			break;

	if(i >= 6)
		code = 0xc0 + ((i - 6) << 3) + j;
	else
		code = (i << 3) + j;

	state = m_io_kbd9->read();
	if(state && (i <= 8))
	{
		switch(state)
		{
			case 1:
				code += 0x90;
				break;
			case 2:
				code += 0x60;
				break;
			case 4:
			case 8:
				code += 0x30;
				break;
		}
	}

	return code;
}

// Italian layout
static INPUT_PORTS_START( m20_keyboard )
	PORT_START("TERM_LINE0")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RESET") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("<") PORT_CODE(KEYCODE_LALT) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')

	PORT_START("TERM_LINE1")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('?')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("TERM_LINE2")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("TERM_LINE3")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xc3\xa0 0") PORT_CODE(KEYCODE_0) PORT_CHAR(0x85) PORT_CHAR('0') // a_GRAVE
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xc2\xa3 1") PORT_CODE(KEYCODE_1) PORT_CHAR(0x9c) PORT_CHAR('1') // POUND
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(e_ACUTE " 2") PORT_CODE(KEYCODE_2) PORT_CHAR(0x82) PORT_CHAR('2')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\" 3") PORT_CODE(KEYCODE_3) PORT_CHAR('"') PORT_CHAR('3')

	PORT_START("TERM_LINE4")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("' 4") PORT_CODE(KEYCODE_4) PORT_CHAR('\'') PORT_CHAR('4')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("( 5") PORT_CODE(KEYCODE_5) PORT_CHAR('(') PORT_CHAR('5')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("_ 6") PORT_CODE(KEYCODE_6) PORT_CHAR('_') PORT_CHAR('6')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xc3\xa8 7") PORT_CODE(KEYCODE_7) PORT_CHAR(0x8a) PORT_CHAR('7') // e_GRAVE
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^ 8") PORT_CODE(KEYCODE_8) PORT_CHAR('^') PORT_CHAR('8')
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xc3\xa7 9") PORT_CODE(KEYCODE_9) PORT_CHAR(0x87) PORT_CHAR('9') // c_CEDILLA
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(") \xc2\xb0") PORT_CODE(KEYCODE_MINUS) PORT_CHAR(')') PORT_CHAR(0xa7) // DEGREE
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- +") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('-') PORT_CHAR('+')

	PORT_START("TERM_LINE5")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xc3\xac =") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x8d) PORT_CHAR('=') // i_GRAVE
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("$ &") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('$') PORT_CHAR('&')
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xc3\xb9 %") PORT_CODE(KEYCODE_COLON) PORT_CHAR(0x8a) PORT_CHAR('%') // u_GRAVE
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("* \xc2\xa7") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('*') PORT_CHAR(0xf5) // SECTION
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("; .") PORT_CODE(KEYCODE_STOP) PORT_CHAR(';') PORT_CHAR('.')
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(": /") PORT_CODE(KEYCODE_SLASH) PORT_CHAR(':') PORT_CHAR('/')
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xc3\xb2") PORT_CODE(KEYCODE_RALT) PORT_CHAR(0x95)

	PORT_START("TERM_LINE6")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S1") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S2") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Keypad 00") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))

	PORT_START("TERM_LINE7")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))

	PORT_START("TERM_LINE8")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))

	PORT_START("TERM_LINE9")
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("COMMAND") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)

	PORT_START("TERM_LINEC")

	PORT_START("RS232_TXBAUD")
	PORT_CONFNAME(0xff, RS232_BAUD_1200, "TX Baud") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_BAUD_1200, "1200")

	PORT_START("RS232_STARTBITS")
	PORT_CONFNAME(0xff, RS232_STARTBITS_1, "Start Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_STARTBITS_1, "1")

	PORT_START("RS232_DATABITS")
	PORT_CONFNAME(0xff, RS232_DATABITS_8, "Data Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_DATABITS_8, "8")

	PORT_START("RS232_PARITY")
	PORT_CONFNAME(0xff, RS232_PARITY_NONE, "Parity") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_PARITY_NONE, "None")

	PORT_START("RS232_STOPBITS")
	PORT_CONFNAME(0xff, RS232_STOPBITS_2, "Stop Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, serial_keyboard_device, update_serial)
	PORT_CONFSETTING( RS232_STOPBITS_2, "2")
INPUT_PORTS_END

ioport_constructor m20_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(m20_keyboard);
}

void m20_keyboard_device::device_start()
{
	serial_keyboard_device::device_start();
	memset(m_state, '\0', sizeof(m_state));
	set_rcv_rate(1200);
}

void m20_keyboard_device::rcv_complete()
{
	receive_register_extract();
	write(get_received_char());
}

const device_type M20_KEYBOARD = &device_creator<m20_keyboard_device>;
