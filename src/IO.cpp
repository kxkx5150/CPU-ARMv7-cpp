#include "IO.h"
#include <cstdint>

IO::IO(IRQ *_gic, DTimer *_timer0, UART *_uart0)
{
    gic       = _gic;
    timer0    = _timer0;
    uart0     = _uart0;
    port_data = new uint32_t[100000]{};
}
IO::~IO()
{
    delete[] port_data;
}
u_int8_t IO::ld_byte(int addr)
{
    int rval = check_portl(addr);
    if (rval != -555)
        return rval;
    return 0;
}
u_int16_t IO::ld_halfword(int addr)
{
    int rval = check_portl(addr);
    if (rval != -555)
        return rval;
    return 0;
}
u_int32_t IO::ld_word(int addr)
{
    int rval = check_portl(addr);
    if (rval != -555)
        return rval;
    return 0;
}
u_int8_t IO::st_byte(int addr, u_int8_t onebyte)
{
    int rval = check_ports(addr, onebyte);
    if (rval != -555)
        return rval;
    return 0;
}
u_int16_t IO::st_halfword(int addr, u_int16_t halfword)
{
    int rval = check_ports(addr, halfword);
    if (rval != -555)
        return rval;
    return 0;
}
u_int32_t IO::st_word(int addr, u_int32_t word)
{
    int rval = check_ports(addr, word);
    if (rval != -555)
        return rval;
    return 0;
}
u_int32_t IO::ld_data(int addr)
{
    int naddr = addr - 0xFFF7580;
    return port_data[naddr];
}
u_int32_t IO::st_data(u_int32_t addr, u_int32_t data)
{
    int naddr        = addr - 0xFFF7580;
    port_data[naddr] = data;
    return 0;
}
int IO::check_ports(int addr, int data)
{
    switch (addr) {
        case 0x1E001000:
            return gic->func_sDCR(data);
        case 0x1E001C00:
        case 0x1E001C04:
        case 0x1E001C08:
        case 0x1E001C0C:
            return gic->func_sICFR(data, (addr - 0x1E001C00) / 4);
        case 0x1E001800:
        case 0x1E001804:
        case 0x1E001808:
        case 0x1E00180C:
        case 0x1E001810:
        case 0x1E001814:
        case 0x1E001818:
        case 0x1E00181C:
        case 0x1E001820:
        case 0x1E001824:
        case 0x1E001828:
        case 0x1E00182C:
        case 0x1E001830:
        case 0x1E001834:
        case 0x1E001838:
        case 0x1E00183C:
            return gic->func_sIPTR(data, (addr - 0x1E001800) / 4);
        case 0x1E001400:
        case 0x1E001404:
        case 0x1E001408:
        case 0x1E00140C:
        case 0x1E001410:
        case 0x1E001414:
        case 0x1E001418:
        case 0x1E00141C:
        case 0x1E001420:
        case 0x1E001424:
        case 0x1E001428:
        case 0x1E00142C:
        case 0x1E001430:
        case 0x1E001434:
        case 0x1E001438:
        case 0x1E00143C:
            return gic->func_sIPR(data, (addr - 0x1E001400) / 4);
        case 0x1E001180:
        case 0x1E001184:
            return gic->func_sICER(data, (addr - 0x1E001180) / 4);
        case 0x1E001100:
        case 0x1E001104:
            return gic->func_sISER(data, (addr - 0x1E001100) / 4);
        case 0x1E000100:
            return gic->func_sCICR(data);
        case 0x1E000104:
            return gic->func_sIPMR(data);
        case 0x1E000110:
            return gic->func_sEOIR(data);
        case 0x1E000800:
            return gic->func_sICDIPTR(data);

        case 0x10011008:
            return timer0->write_Control1(data);
        case 0x10011028:
            return timer0->write_Control2(data);
        case 0x10009000:
            return uart0->func_sDR(data);
        case 0x10009030:
            return uart0->func_sCR(data);
        case 0x1000902C:
            return uart0->func_sLCR_H(data);
        case 0x10009034:
            return uart0->func_sIFLS(data);
        case 0x10009038:
            return uart0->func_sIMSC(data);
        case 0x10009044:
            return uart0->func_sICR(data);
    }
    return -555;
}
int IO::check_portl(int addr)
{
    switch (addr) {
        case 0x1E001000:
            return gic->func_lDCR();
        case 0x1E001004:
            return gic->func_ICTR();
        case 0x1E001C00:
        case 0x1E001C04:
        case 0x1E001C08:
        case 0x1E001C0C:
            return gic->func_ICFR((addr - 0x1E001C00) / 4);
        case 0x1E001800:
        case 0x1E001804:
        case 0x1E001808:
        case 0x1E00180C:
        case 0x1E001810:
        case 0x1E001814:
        case 0x1E001818:
        case 0x1E00181C:
        case 0x1E001820:
        case 0x1E001824:
        case 0x1E001828:
        case 0x1E00182C:
        case 0x1E001830:
        case 0x1E001834:
        case 0x1E001838:
        case 0x1E00183C:
            return gic->func_IIPTR((addr - 0x1E001800) / 4);
        case 0x1E001400:
        case 0x1E001404:
        case 0x1E001408:
        case 0x1E00140C:
        case 0x1E001410:
        case 0x1E001414:
        case 0x1E001418:
        case 0x1E00141C:
        case 0x1E001420:
        case 0x1E001424:
        case 0x1E001428:
        case 0x1E00142C:
        case 0x1E001430:
        case 0x1E001434:
        case 0x1E001438:
        case 0x1E00143C:
            return gic->func_sIPR((addr - 0x1E001400) / 4, 0);
        case 0x1E001180:
        case 0x1E001184:
            return gic->func_IICER((addr - 0x1E001180) / 4);
        case 0x1E001100:
        case 0x1E001104:
            return gic->func_IISER((addr - 0x1E001100) / 4);
        case 0x1E000100:
            return gic->func_lCICR();
        case 0x1E000104:
            return gic->func_lIPMR();
        case 0x1E00010C:
            return gic->func_lIAR();
        case 0x1E000800:
            return gic->func_lICDIPTR();
        case 0x10009FE0:
            return 0x11;
        case 0x10009FE4:
            return 0x10;
        case 0x10009FE8:
            return 0x34;
        case 0x10009FEC:
            return 0x00;
        case 0x10009FF0:
            return 0x0d;
        case 0x10009FF4:
            return 0xf0;
        case 0x10009FF8:
            return 0x05;
        case 0x10009FFC:
            return 0xb1;
        case 0x10009000:
            return uart0->func_lDR();
        case 0x10009018:
            return uart0->func_IFR();
        case 0x10009030:
            return uart0->func_lCR();
        case 0x1000902C:
            return uart0->func_lLCR_H();
        case 0x10009034:
            return uart0->func_lIFLS();
        case 0x10009038:
            return uart0->func_lIMSC();
        case 0x10009040:
            return uart0->func_IMIS();
        case 0x100000A8:
            return 0xff;
    }
    return -555;
}
