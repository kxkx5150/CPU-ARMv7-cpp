#ifndef _H_IO
#define _H_IO
#include "IRQ.h"
#include "dtimer.h"
#include "UART.h"
#include <sys/types.h>

class IO {

  public:
    IRQ    *gic    = nullptr;
    DTimer *timer0 = nullptr;
    UART   *uart0  = nullptr;

    uint32_t *port_data;

  public:
    IO(IRQ *_gic, DTimer *_timer0, UART *_uart0);
    ~IO();

    u_int8_t  ld_byte(int addr);
    u_int16_t ld_halfword(int addr);
    u_int32_t ld_word(int addr);

    u_int8_t  st_byte(int addr, u_int8_t onebyte);
    u_int16_t st_halfword(int addr, u_int16_t halfword);
    u_int32_t st_word(int addr, u_int32_t word);
    int       check_ports(int addr, int data);
    int       check_portl(int addr);

    u_int32_t ld_data(int addr);
    u_int32_t st_data(u_int32_t addr, u_int32_t data);

  private:
};
#endif
