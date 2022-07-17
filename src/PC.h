#ifndef _H_PC
#define _H_PC

#include "IO.h"
#include "IRQ.h"
#include "dtimer.h"
#include "UART.h"
#include "Mem.h"
#include "ARM/ARMv7_cpu.h"
#include <stdexcept>
using namespace std;

class PC {
  private:
    bool    is_booted      = false;
    bool    is_running     = false;
    int64_t tick           = 0;
    int64_t n_instructions = 0;
    int     n_interrupts   = 0;
    int     stop_after     = 0;
    int     irq_base       = 32;

    uint8_t *bin0 = nullptr;
    uint8_t *bin1 = nullptr;
    uint8_t *bin2 = nullptr;

    int mem_size   = 16 * 1024 * 1024;
    int start_addr = 0x10000;
    int steps      = -1;

  public:
    ARMV7_CPU *cpu     = nullptr;
    IRQ       *gic     = nullptr;
    DTimer    *timer0  = nullptr;
    Mem       *memctlr = nullptr;
    UART      *uart0   = nullptr;
    IO        *io      = nullptr;

  public:
    PC();
    ~PC();

    void boot();
    void load_binary(string path, int64_t phyaddr);
    void run();
    void loop();

  private:
};
#endif
