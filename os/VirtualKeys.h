#ifndef VIRTUALKEYS_H
#define VIRTUALKEYS_H

#include "../CommonTypes.h"

#define XL_LBUTTON        0x01
#define XL_RBUTTON        0x02
#define XL_CANCEL         0x03
#define XL_MBUTTON        0x04    /* NOT contiguous with L & RBUTTON */

#define XL_XBUTTON1       0x05    /* NOT contiguous with L & RBUTTON */
#define XL_XBUTTON2       0x06    /* NOT contiguous with L & RBUTTON */

#define XL_BACK           0x08
#define XL_TAB            0x09

#define XL_CLEAR          0x0C
#define XL_RETURN         0x0D

#define XL_SHIFT          0x10
#define XL_CONTROL        0x11
#define XL_MENU           0x12
#define XL_PAUSE          0x13
#define XL_CAPITAL        0x14

#define XL_KANA           0x15
#define XL_JUNJA          0x17
#define XL_FINAL          0x18
#define XL_HANJA          0x19
#define XL_KANJI          0x19

#define XL_ESCAPE         0x1B

#define XL_CONVERT        0x1C
#define XL_NONCONVERT     0x1D
#define XL_ACCEPT         0x1E
#define XL_MODECHANGE     0x1F

#define XL_SPACE          0x20
#define XL_PRIOR          0x21
#define XL_NEXT           0x22
#define XL_END            0x23
#define XL_HOME           0x24
#define XL_LEFT           0x25
#define XL_UP             0x26
#define XL_RIGHT          0x27
#define XL_DOWN           0x28
#define XL_SELECT         0x29
#define XL_PRINT          0x2A
#define XL_EXECUTE        0x2B
#define XL_SNAPSHOT       0x2C
#define XL_INSERT         0x2D
#define XL_DELETE         0x2E
#define XL_HELP           0x2F

#define XL_0			  0x30
#define XL_1			  0x31
#define XL_2			  0x32
#define XL_3			  0x33
#define XL_4			  0x34
#define XL_5			  0x35
#define XL_6			  0x36
#define XL_7			  0x37
#define XL_8			  0x38
#define XL_9			  0x39

#define XL_A			  0x41
#define XL_B			  0x42
#define XL_C			  0x43
#define XL_D			  0x44
#define XL_E			  0x45
#define XL_F			  0x46
#define XL_G			  0x47
#define XL_H			  0x48
#define XL_I			  0x49
#define XL_J			  0x4A
#define XL_K			  0x4B
#define XL_L			  0x4C
#define XL_M			  0x4D
#define XL_N			  0x4E
#define XL_O			  0x4F
#define XL_P			  0x50
#define XL_Q			  0x51
#define XL_R			  0x52
#define XL_S			  0x53
#define XL_T			  0x54
#define XL_U			  0x55
#define XL_V			  0x56
#define XL_W			  0x57
#define XL_X			  0x58
#define XL_Y			  0x59
#define XL_Z			  0x5A
#define XL_SLEEP          0x5F
#define XL_NUMPAD0        0x60
#define XL_NUMPAD1        0x61
#define XL_NUMPAD2        0x62
#define XL_NUMPAD3        0x63
#define XL_NUMPAD4        0x64
#define XL_NUMPAD5        0x65
#define XL_NUMPAD6        0x66
#define XL_NUMPAD7        0x67
#define XL_NUMPAD8        0x68
#define XL_NUMPAD9        0x69
#define XL_MULTIPLY       0x6A
#define XL_ADD            0x6B
#define XL_SEPARATOR      0x6C
#define XL_SUBTRACT       0x6D
#define XL_DECIMAL        0x6E
#define XL_DIVIDE         0x6F
#define XL_F1             0x70
#define XL_F2             0x71
#define XL_F3             0x72
#define XL_F4             0x73
#define XL_F5             0x74
#define XL_F6             0x75
#define XL_F7             0x76
#define XL_F8             0x77
#define XL_F9             0x78
#define XL_F10            0x79
#define XL_F11            0x7A
#define XL_F12            0x7B
#define XL_F13            0x7C
#define XL_F14            0x7D
#define XL_F15            0x7E
#define XL_F16            0x7F
#define XL_F17            0x80
#define XL_F18            0x81
#define XL_F19            0x82
#define XL_F20            0x83
#define XL_F21            0x84
#define XL_F22            0x85
#define XL_F23            0x86
#define XL_F24            0x87
#define XL_NUMLOCK        0x90
#define XL_SCROLL         0x91
#define XL_OEM_NEC_EQUAL  0x92   // '=' key on numpad
#define XL_OEM_FJ_JISHO   0x92   // 'Dictionary' key
#define XL_OEM_FJ_MASSHOU 0x93   // 'Unregister word' key
#define XL_OEM_FJ_TOUROKU 0x94   // 'Register word' key
#define XL_OEM_FJ_LOYA    0x95   // 'Left OYAYUBI' key
#define XL_OEM_FJ_ROYA    0x96   // 'Right OYAYUBI' key
#define XL_LSHIFT         0xA0
#define XL_RSHIFT         0xA1
#define XL_LCONTROL       0xA2
#define XL_RCONTROL       0xA3
#define XL_LMENU          0xA4
#define XL_RMENU          0xA5

#define XL_OEM_1          0xBA   // ';:' for US
#define XL_OEM_PLUS       0xBB   // '+' any country
#define XL_OEM_COMMA      0xBC   // ',' any country
#define XL_OEM_MINUS      0xBD   // '-' any country
#define XL_OEM_PERIOD     0xBE   // '.' any country
#define XL_OEM_2          0xBF   // '/?' for US
#define XL_TILDE          0xC0   // '`~' for US

#define XL_OEM_4          0xDB  //  '[{' for US
#define XL_OEM_5          0xDC  //  '\|' for US
#define XL_OEM_6          0xDD  //  ']}' for US
#define XL_OEM_7          0xDE  //  ''"' for US
#define XL_OEM_8          0xDF

#endif	//VIRTUALKEYS_H
