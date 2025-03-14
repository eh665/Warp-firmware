/*
 *	See https://github.com/adafruit/Adafruit-SSD1331-OLED-Driver-Library-for-Arduino for the Arduino driver.
 */

typedef enum
{
	kSSD1331ColororderRGB		= 1, // could alternatively choose to do: #define SSD1331_COLORORDER_BGR  but can't do both

	kSSD1331DelaysHWFILL		= 3, //!< Fill delay
	kSSD1331DelaysHWLINE		= 1, //!< Line delay
} SSD1331Constants;

typedef enum
{
	kSSD1331CommandDRAWLINE			= 0x21, //!< Draw line
	kSSD1331CommandDRAWRECT			= 0x22, //!< Draw rectangle
	kSSD1331CommandCLEAR			= 0x25,
	kSSD1331CommandFILL				= 0x26, //!< Fill enable/disable
	kSSD1331CommandSETCOLUMN		= 0x15, //!< Set column address
	kSSD1331CommandSETROW			= 0x75, //!< Set row adress
	kSSD1331CommandCONTRASTA		= 0x81, //!< Set contrast for color A
	kSSD1331CommandCONTRASTB		= 0x82, //!< Set contrast for color B
	kSSD1331CommandCONTRASTC		= 0x83, //!< Set contrast for color C
	kSSD1331CommandMASTERCURRENT	= 0x87, //!< Master current control
	kSSD1331CommandSETREMAP			= 0xA0, //!< Set re-map & data format
	kSSD1331CommandSTARTLINE		= 0xA1, //!< Set display start line
	kSSD1331CommandDISPLAYOFFSET	= 0xA2, //!< Set display offset
	kSSD1331CommandNORMALDISPLAY	= 0xA4, //!< Set display to normal mode
	kSSD1331CommandDISPLAYALLON		= 0xA5, //!< Set entire display ON
	kSSD1331CommandDISPLAYALLOFF	= 0xA6, //!< Set entire display OFF
	kSSD1331CommandINVERTDISPLAY	= 0xA7, //!< Invert display
	kSSD1331CommandSETMULTIPLEX		= 0xA8, //!< Set multiplex ratio
	kSSD1331CommandSETMASTER		= 0xAD, //!< Set master configuration
	kSSD1331CommandDISPLAYOFF		= 0xAE, //!< Display OFF (sleep mode)
	kSSD1331CommandDISPLAYON		= 0xAF, //!< Normal Brightness Display ON
	kSSD1331CommandPOWERMODE		= 0xB0, //!< Power save mode
	kSSD1331CommandPRECHARGE		= 0xB1, //!< Phase 1 and 2 period adjustment
	kSSD1331CommandCLOCKDIV			= 0xB3, //!< Set display clock divide ratio/oscillator frequency
	kSSD1331CommandPRECHARGEA		= 0x8A, //!< Set second pre-charge speed for color A
	kSSD1331CommandPRECHARGEB		= 0x8B, //!< Set second pre-charge speed for color B
	kSSD1331CommandPRECHARGEC		= 0x8C, //!< Set second pre-charge speed for color C
	kSSD1331CommandPRECHARGELEVEL	= 0xBB, //!< Set pre-charge voltage
	kSSD1331CommandVCOMH			= 0xBE, //!< Set Vcomh voltge
} SSD1331Commands;

int	devSSD1331init(void);
