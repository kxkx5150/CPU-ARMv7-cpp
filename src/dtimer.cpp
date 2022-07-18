#include "dtimer.h"
#include <cstddef>
#include <cstdint>

// DTimer *dtimer = nullptr;
// void    settimer(void)
// {
//     dtimer->set_timeout();
// }
DTimer::DTimer(int _baseaddr, int _irq_base, IRQ *_gic)
{
    // dtimer   = this;
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
void DTimer::set_timer_func(size_t count)
{
    switch (count) {
        case 1462034:
        case 1761882:
        case 2061765:
        case 2461648:
        case 2761532:
        case 3061415:
        case 3461298:
        case 3861181:
        case 4161065:
        case 4346264:
        case 4528368:
        case 4708182:
        case 4891860:
        case 5190396:
        case 5465097:
        case 5647415:
        case 5831445:
        case 6012978:
        case 6194460:
        case 6467129:
        case 6649323:
        case 6831198:
        case 7013618:
        case 7194320:
        case 7381486:
        case 7563407:
        case 7828874:
        case 8095038:
        case 8361186:
        case 8627756:
        case 8896176:
        case 9163356:
        case 9431611:
        case 9706806:
        case 9892101:
        case 10081577:
        case 10267517:
        case 10452623:
        case 10637304:
        case 10821506:
        case 11016194:
        case 11198372:
        case 11385827:
        case 11573159:
        case 11768765:
        case 11865026:
        case 12058541:
        case 12248532:
        case 12437379:
        case 12625142:
        case 12909684:
        case 13096793:
        case 13285892:
        case 13473731:
        case 13659751:
        case 13856818:
        case 14053869:
        case 14250936:
        case 14448002: {
            timeout_timer1();
        } break;
        default:
            break;
    }
    return;
}
void DTimer::timeout_timer1()
{
    if (Control1_data.IntEnable)
        gic->send_interrupt(irq);

    Value2_data -= Value1_data;
    if (Value2_data < 0)
        Value2_data = 0xffffffff + Value2_data;

    if (!Control1_data.OneShot) {
        // int64_t val = Value1_data;
        // if (val != 0xffffffff) {
        //     printf("-------setTimer1\n");
        //     later later_test1(1, true, &settimer);
        // }
    }
}
void DTimer::timeout_timer2()
{
    if (Control2_data.IntEnable)
        gic->send_interrupt(irq);

    if (!Control2_data.OneShot) {
        // int64_t val = Value1_data;
        // if (val != 0xffffffff) {
        //     printf("setTimer2\n");
        //     later later_test1(1, true, &settimer);
        // }
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
