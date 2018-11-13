#include <termios.h>

typedef signed short s16;
typedef unsigned short u16;

#define	TTY_NAME			"/dev/ttyAMA1"
#define FILE_NAME			"./uart_test.txt"
#define	TTY_BAUDRATE		9600
#define	TTY_W_BUFFER		256
#define	TTY_NC_CHARNUM		1
#define	TTY_NC_TIMEOUT		0

#define CANOICAL			1
#define NON_CANOICAL		0

#define READ_MODE			0
#define WRITE_MODE			1

#define TEST_MASTER			2
#define TEST_SLAVE			3

#define KBYTE		1024
#define TEST_SIZE	256	

#define CMD_CHECK_READY			0x01
#define CMD_READY				0x02
#define CMD_CHANGE_BAUD			0x03
#define CMD_CHANGE_BAUD_OK		0X05
#define CMD_SET_CRC				0x07
#define CRC_CHECK_OK			0x08

#define CMD_WRITE_OK			0x09


#define FALSE					0x00
#define TRUE					0x01


#define TEST_OK					0x01
#define TEST_NOK				0x00
#define NOT_TESTED				-1

#define READ_OK					2
#define READING					1

#define WRITING					1
#define WRITE_OK 				0

#define _CRLF_(c, p, l)	{ if (c) { p[l] = '\r', p[l+1] = '\n', l += 1; } }

struct sig_param {
	int			   fd;
	struct termios tio;
};



/*----------------------------------------------------------------------------
 *	struct termios
 *	{
 *	    tcflag_t c_iflag;       // input modes
 *	    tcflag_t c_oflag;      	// output modes
 *	    tcflag_t c_cflag;      	// control modes
 *	    tcflag_t c_lflag;       // local modes
 *	    cc_t c_cc[NCCS];    	// control chars
 *	}
 *
 * [arch/arm/include/asm/termbits.h]
 *
 * [ c_iflag bits ]
 * #define IGNBRK	0000001
 * #define BRKINT	0000002
 * #define IGNPAR	0000004
 * #define PARMRK	0000010
 * #define INPCK	0000020
 * #define ISTRIP	0000040
 * #define INLCR	0000100
 * #define IGNCR	0000200
 * #define ICRNL	0000400
 * #define IUCLC	0001000
 * #define IXON		0002000
 * #define IXANY	0004000
 * #define IXOFF	0010000
 * #define IMAXBEL	0020000
 * #define IUTF8	0040000
 *
 * [ c_oflag bits ]
 * #define OPOST	0000001
 * #define OLCUC	0000002
 * #define ONLCR	0000004
 * #define OCRNL	0000010
 * #define ONOCR	0000020
 * #define ONLRET	0000040
 * #define OFILL	0000100
 * #define OFDEL	0000200
 * #define NLDLY	0000400
 * #define   NL0	0000000
 * #define   NL1	0000400
 * #define CRDLY	0003000
 * #define   CR0	0000000
 * #define   CR1	0001000
 * #define   CR2	0002000
 * #define   CR3	0003000
 * #define TABDLY	0014000
 * #define   TAB0	0000000
 * #define   TAB1	0004000
 * #define   TAB2	0010000
 * #define   TAB3	0014000
 * #define   XTABS	0014000
 * #define BSDLY	0020000
 * #define   BS0	0000000
 * #define   BS1	0020000
 * #define VTDLY	0040000
 * #define   VT0	0000000
 * #define   VT1	0040000
 * #define FFDLY	0100000
 * #define   FF0	0000000
 * #define   FF1	0100000
 *
 * [ c_cflag bit meaning ]
 * #define CBAUD		0010017
 * #define  B0			0000000		// hang up
 * #define  B50			0000001
 * #define  B75			0000002
 * #define  B110		0000003
 * #define  B134		0000004
 * #define  B150		0000005
 * #define  B200		0000006
 * #define  B300		0000007
 * #define  B600		0000010
 * #define  B1200		0000011
 * #define  B1800		0000012
 * #define  B2400		0000013
 * #define  B4800		0000014
 * #define  B9600		0000015
 * #define  B19200		0000016
 * #define  B38400		0000017
 * #define EXTA 		B19200
 * #define EXTB 		B38400
 * #define CSIZE		0000060
 * #define   CS5		0000000
 * #define   CS6		0000020
 * #define   CS7		0000040
 * #define   CS8		0000060
 * #define CSTOPB		0000100
 * #define CREAD		0000200
 * #define PARENB		0000400
 * #define PARODD		0001000
 * #define HUPCL		0002000
 * #define CLOCAL		0004000
 * #define CBAUDEX 		0010000
 * #define    BOTHER 	0010000
 * #define    B57600 	0010001
 * #define   B115200 	0010002
 * #define   B230400 	0010003
 * #define   B460800 	0010004
 * #define   B500000 	0010005
 * #define   B576000 	0010006
 * #define   B921600 	0010007
 * #define  B1000000 	0010010
 * #define  B1152000 	0010011
 * #define  B1500000 	0010012
 * #define  B2000000 	0010013
 * #define  B2500000 	0010014
 * #define  B3000000 	0010015
 * #define  B3500000 	0010016
 * #define  B4000000 	0010017
 * #define CIBAUD	 	002003600000		// input baud rate
 * #define CMSPAR    	010000000000		// mark or space (stick) parity
 * #define CRTSCTS	  	020000000000			// flow control
 *
 * #define IBSHIFT	   16
 *
 * [ c_lflag bits ]
 * #define ISIG		0000001
 * #define ICANON	0000002
 * #define XCASE	0000004
 * #define ECHO		0000010
 * #define ECHOE	0000020
 * #define ECHOK	0000040
 * #define ECHONL	0000100
 * #define NOFLSH	0000200
 * #define TOSTOP	0000400
 * #define ECHOCTL	0001000
 * #define ECHOPRT	0002000
 * #define ECHOKE	0004000
 * #define FLUSHO	0010000
 * #define PENDIN	0040000
 * #define IEXTEN	0100000
 * #define EXTPROC	0200000
 *----------------------------------------------------------------------------*/

