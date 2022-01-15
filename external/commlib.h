#ifndef COMMLIB_H_
#define COMMLIB_H_

#pragma pack(1)
typedef union {
 char _far * p;
 struct _tag_farptr {
  unsigned int offset;
  unsigned short selector;
 } s;
} GF_FARPTR32;
#pragma pack()
#define GF_FAR far

typedef enum trigger_level{
 TRIGGER_DISABLE = 0,
 TRIGGER_01 = 1,
 TRIGGER_04 = 0x41,
 TRIGGER_08 = 0x81,
 TRIGGER_14 = 0xc1
} TRIGGER_LEVEL;

#define COM1 0
#define COM2 1
#define COM3 2
#define COM4 3

#define IRQ0 0
#define IRQ1 1
#define IRQ2 2
#define IRQ3 3
#define IRQ4 4
#define IRQ5 5
#define IRQ6 6
#define IRQ7 7
#define IRQ8 8
#define IRQ9 9
#define IRQ10 10
#define IRQ11 11
#define IRQ12 12
#define IRQ13 13
#define IRQ14 14
#define IRQ15 15

#define ASSUCCESS 0
#define ASGENERALERROR -1
#define ASINVPORT -2
#define ASINUSE -3
#define ASINVBUFSIZE -4
#define ASNOMEMORY -5
#define ASNOTSETUP -6
#define ASINVPAR -7
#define ASBUFREMPTY -8
#define ASBUFRFULL -9
#define ASTIMEOUT -10

#define PORT struct _tag_port

struct _tag_port {
    GF_FARPTR32 driver;
    PORT *next_port;
    int handle;
    int status;
    unsigned char driver_type;
    int dialing_method;
    unsigned int count;
    int (*read_char )( PORT *port );
    int (*peek_char )( PORT *port );
    int (*write_char )( PORT *port, int c );
    int (*port_close )( PORT *port );
    int (*port_set )( PORT *port,
                                long baud_rate,
                                char parity,
                                int word_length,
                                int stop_bits );
    int (*use_xon_xoff)( PORT *port, int control );
    int (*use_rts_cts )( PORT *port, int control );
    int (*use_dtr_dsr )( PORT *port, int control );
    int (*set_dtr )( PORT *port, int control );
    int (*set_rts )( PORT *port, int control );
    long (*space_free_in_TX_buffer )( PORT *port );
    long (*space_used_in_TX_buffer )( PORT *port );
    long (*space_free_in_RX_buffer )( PORT *port );
    long (*space_used_in_RX_buffer )( PORT *port );
    int (*clear_TX_buffer )( PORT *port );
    int (*write_buffer )( PORT *port,
                                    char *buffer,
                                    unsigned int count );
    int (*read_buffer )( PORT *port,
                                   char *buffer,
                                   unsigned int count );
    int (*dump_port_status )( PORT *port,
                                        void * printer );
    int (*send_break )( PORT *port,
                                  int milliseconds );
    int (*get_modem_status )( PORT *port );
    int (*get_line_status )( PORT *port );
    int (*clear_line_status)( PORT *port );
    int (*block )( PORT *port, int control );
    void (*clear_error )( PORT *port );
    void GF_FAR *user_data;
    void GF_FAR *lpThis;
};

#undef PORT

typedef struct _tag_port PORT;

#define ReadChar(p)               p->read_char(p)
#define PeekChar(p)               p->peek_char(p)
#define WriteChar(p, c)           p->write_char(p, c)
#define PortClose(p)              p->port_close(p)
#define PortSet( p, b, py, wl, sb ) p->port_set( p, b, py, wl, sb )
#define UseXonXoff(p, c)          p->use_xon_xoff(p, c)
#define UseRtsCts(p, c)           p->use_rts_cts(p, c)
#define UseDtrDsr(p, c)           p->use_dtr_dsr(p, c)
#define DumpPortStatus( p, f )      p->dump_port_status( p, f )
#define SetDtr(p, c)              p->set_dtr(p, c)
#define SetRts(p, c)              p->set_rts(p, c)
#define SpaceFreeInTXBuffer(p)    p->space_free_in_TX_buffer(p)
#define SpaceFreeInRXBuffer(p)    p->space_free_in_RX_buffer(p)
#define SpaceUsedInTXBuffer(p)    p->space_used_in_TX_buffer(p)
#define SpaceUsedInRXBuffer(p)    p->space_used_in_RX_buffer(p)
#define ClearTXBuffer(p)          p->clear_TX_buffer(p)
#define WriteBuffer( p, b, i )      p->write_buffer( p, b, i )
#define ReadBuffer( p, b, i )       p->read_buffer( p, b, i )
#define SendBreak( p, t )           p->send_break( p, t )
#define GetModemStatus(p)         p->get_modem_status(p)
#define GetLineStatus(p)          p->get_line_status(p)
#define ClearLineStatus(p)        p->clear_line_status(p)
#define Block(p, c)               p->block(p, c)
#define ClearError(p)             p->clear_error(p)

int Change8259Priority(int irq);
int ReadCharTimed(PORT *, long);
int GetCd(PORT *);
int ReadBufferTimed(PORT *, char *, unsigned int, long);
int ClearRXBuffer(PORT *);
PORT *PortOpenGreenleafFast(int,long,char,int,int);
#define OFF 0
#define ON 1

// workaround for watcom quirk where global var order depends on number of declarations
void sfill00(void);
void sfill01(void);
void sfill02(void);
void sfill03(void);
void sfill04(void);
void sfill05(void);
void sfill06(void);
void sfill07(void);
void sfill08(void);
void sfill09(void);
void sfill0a(void);
void sfill0b(void);
void sfill0c(void);
void sfill0d(void);
void sfill0e(void);
void sfill0f(void);

#endif
