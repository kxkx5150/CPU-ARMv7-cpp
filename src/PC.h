#ifndef _H_PC
#define _H_PC

#include "IO.h"
#include "IRQ.h"
#include "dtimer.h"
#include "UART.h"
#include "Mem.h"
#include "ARM/ARMv7_cpu.h"
#include <stdexcept>
#include <stdexcept>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstddef>

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

    TTF_Font *font = nullptr;

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

    void init();
    void start();
    void load_binary(std::string path, int64_t phyaddr);
    void run_cpu();
    void loop();

    void paint(SDL_Renderer *render, int widht, int height);

  private:
};
#endif
