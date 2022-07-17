#include "dtimer.h"
#include <cstdint>

DTimer *dtimer = nullptr;
void    settimer(void)
{
    dtimer->set_timeout();
}
DTimer::DTimer(int _baseaddr, int _irq_base, IRQ *_gic)
{
    dtimer   = this;
    baseaddr = _baseaddr;
    irq      = _irq_base;
    gic      = _gic;
    bitops   = new BitOps();
    // later later_test1(1, true, &settimer);
    // printf("setTimer1\n");
}
DTimer::~DTimer()
{
    delete bitops;
}
int DTimer::write_Control1(uint32_t word)
{
    auto ctl      = parse_control_register(word);
    Control1_data = ctl;
    if (ctl.En) {
        int64_t val = Value1_data;
        if (val != 0xffffffff)
            timeout_timer1();
    }
    return 1;
}
int DTimer::write_Control2(uint32_t word)
{
    auto ctl      = parse_control_register(word);
    Control2_data = ctl;
    if (ctl.En) {
        int64_t val = Value2_data;
        if (val != 0xffffffff)
            timeout_timer2();
    }
    return 1;
}
int DTimer::set_timeout()
{
    printf("DTimer::set_timeout\n");
    return 1;
}
void DTimer::timeout_timer1()
{
    if (Control1_data.IntEnable)
        gic->send_interrupt(irq);

    Value2_data -= Value1_data;
    if (Value2_data < 0)
        Value2_data = 0xffffffff + Value2_data;

    if (!Control1_data.OneShot) {
        int64_t val = Value1_data;
        if (val != 0xffffffff) {
            printf("setTimer1\n");
            later later_test1(1, true, &settimer);
        }
    }
}
void DTimer::timeout_timer2()
{
    if (Control2_data.IntEnable)
        gic->send_interrupt(irq);

    if (!Control2_data.OneShot) {
        int64_t val = Value1_data;
        if (val != 0xffffffff) {
            printf("setTimer2\n");
            later later_test1(1, true, &settimer);
        }
    }
}
ControllData DTimer::parse_control_register(int value)
{
    ControllData ctl = {.En = 0, .Mode = 0, .IntEnable = 0, .Pre = 0, .Size = 0, .OneShot = 0, .value = 0};
    ctl.En           = bitops->get_bit(value, 7, 0);
    ctl.Mode         = bitops->get_bit(value, 6, 0);
    ctl.IntEnable    = bitops->get_bit(value, 5, 0);
    ctl.Pre          = bitops->get_bits(value, 3, 2);
    ctl.Size         = bitops->get_bit(value, 1, 0);
    ctl.OneShot      = bitops->get_bit(value, 0, 0);
    ctl.value        = value;
    return ctl;
}
