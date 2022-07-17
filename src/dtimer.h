#ifndef _H_DTIMER
#define _H_DTIMER
#include "IRQ.h"
#include "utils/bitopts.h"
#include "utils/timer.h"

#include <cstddef>
#include <cstdint>

using namespace std;

typedef struct ControllData
{
    int En        = 0;
    int Mode      = 0;
    int IntEnable = 0;
    int Pre       = 0;
    int Size      = 0;
    int OneShot   = 0;
    int value     = 0;
} ControllData;

class DTimer {

  public:
    IRQ    *gic    = nullptr;
    BitOps *bitops = nullptr;

  private:
    int64_t baseaddr    = 0;
    int64_t irq         = 0;
    int64_t TIMCLK      = 100000;
    int     Value1_data = 0;
    int     Value2_data = 0;

    ControllData Control1_data = {.En = 0, .Mode = 0, .IntEnable = 1, .Pre = 0, .Size = 0, .OneShot = 0, .value = 0};
    ControllData Control2_data = {.En = 0, .Mode = 0, .IntEnable = 1, .Pre = 0, .Size = 0, .OneShot = 0, .value = 0};

  public:
    DTimer(int _baseaddr, int _irq_base, IRQ *_gic);
    ~DTimer();

    int write_Control1(uint32_t word);
    int write_Control2(uint32_t word);

    void timeout_timer1();
    void timeout_timer2();
    int  set_timeout();

    ControllData parse_control_register(int value);
};
#endif
