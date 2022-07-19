#include "UART.h"
#include <cstdio>
#include <string>

UART::UART(int _id, int _baseaddr, int _irq_base, IRQ *_gic)
{
    gic    = _gic;
    bitops = new BitOps();
    irq    = _irq_base;
    DR     = baseaddr + 0x00;
    FR     = baseaddr + 0x18;
    IBRD   = baseaddr + 0x24;
    FBRD   = baseaddr + 0x28;
    LCR_H  = baseaddr + 0x2c;
    CR     = baseaddr + 0x30;
    IFLS   = baseaddr + 0x34;
    IMSC   = baseaddr + 0x38;
    MIS    = baseaddr + 0x40;
    ICR    = baseaddr + 0x44;
}
UART::~UART()
{
    delete bitops;
}
int64_t UART::func_sDR(int64_t onebyte)
{
    if (!(onebyte >= 0 && onebyte <= 127))
        throw "Invalid char: ";
    auto str = (char)(onebyte);
    output_char(str);
    return 0;
};
int64_t UART::func_lDR()
{
    if (rx_fifo_idx > 0) {
        int64_t val = rx_fifo[rx_fifo_idx];
        rx_fifo_idx--;
        return val;
    } else
        return 0;
};
int64_t UART::func_IFR()
{
    int64_t ret = 0;
    if (tx_fifo_idx == 0)
        ret += (1 << 7);
    if (rx_fifo_idx >= fifo_length)
        ret += (1 << 6);
    if (rx_fifo_idx == 0)
        ret += (1 << 4);
    return ret;
};
int64_t UART::func_sCR(int64_t halfword)
{
    bool old = (CR_data & 1) ? true : false;
    enabled  = (halfword & 1) ? true : false;
    CR_data  = halfword;
    if (!old && enabled) {
        enable();
    }
    if (halfword & 0x200)
        rx_enabled = true;
    else
        rx_enabled = false;
    if (halfword & 0x100)
        tx_enabled = true;
    else
        tx_enabled = false;
    return 0;
};
int64_t UART::func_lCR()
{
    return CR_data;
};
int64_t UART::func_sLCR_H(int64_t onebyte)
{
    update_fifo_onoff(onebyte);
    LCR_H_data = onebyte;
    return 0;
};
int64_t UART::func_lLCR_H()
{
    return LCR_H_data;
};

int64_t UART::func_sIFLS(int64_t halfword)
{
    update_fifo_level(halfword);
    IFLS_data = halfword;
    return 0;
};
int64_t UART::func_lIFLS()
{
    return IFLS_data;
};
int64_t UART::func_sIMSC(int64_t halfword)
{
    tx_int_enabled = (halfword & 0x20) ? true : false;
    rx_int_enabled = (halfword & 0x10) ? true : false;
    if (tx_int_enabled && tx_fifo_idx == 0)
        gic->send_interrupt(irq);
    IMSC_data = halfword;
    return 0;
};
int64_t UART::func_lIMSC()
{
    return IMSC_data;
};
int64_t UART::func_IMIS()
{
    int64_t ret = 0;
    if (!tx_fifo_idx)
        ret += (1 << 5);
    if (rx_fifo_idx)
        ret += (1 << 4);
    return ret;
};
int64_t UART::func_sICR(int64_t halfword)
{
    ICR_data = halfword;
    return 0;
};
int64_t UART::enable()
{
    while (tx_fifo_idx > 0)
        tx_fifo_idx--;
    int64_t rval = tx_fifo[tx_fifo_idx];
    store_char(rval);
    return 0;
};

int64_t UART::output_char(char str)
{
    tx_fifo[tx_fifo_idx] = (str);
    tx_fifo_idx++;
    while (tx_fifo_idx > 0) {
        tx_fifo_idx--;
        int64_t rval = tx_fifo[tx_fifo_idx];
        store_char(rval);
    }
    return 0;
};
int64_t UART::input_char(char str)
{
    rx_fifo[rx_fifo_idx] = (str);
    rx_fifo_idx++;
    if (rx_int_enabled) {
        if (!fifo_enabled || rx_fifo_idx > rx_fifo_level) {
            gic->send_interrupt(irq);
        } else {
            if (rx_fifo_idx > 0)
                gic->send_interrupt(irq);
        }
    }
    return 0;
};
int64_t UART::update_fifo_onoff(int64_t halfword)
{
    fifo_enabled = (bitops->get_bit(halfword, 4, 0) ? true : false);
    return 0;
};
int64_t UART::update_fifo_level(int64_t halfword)
{
    switch (bitops->get_bits(halfword, 5, 3)) {
        case 0:
            rx_fifo_level = fifo_length / 8;
            break;
        case 1:
            rx_fifo_level = fifo_length / 4;
            break;
        case 2:
            rx_fifo_level = fifo_length / 2;
            break;
        case 3:
            rx_fifo_level = fifo_length * 3 / 4;
            break;
        case 4:
            rx_fifo_level = fifo_length * 7 / 8;
            break;
        default:
            throw "UART: unknown RX FIFO level: ";
    }
    switch (bitops->get_bits(halfword, 2, 0)) {
        case 0:
            tx_fifo_level = fifo_length / 8;
            break;
        case 1:
            tx_fifo_level = fifo_length / 4;
            break;
        case 2:
            tx_fifo_level = fifo_length / 2;
            break;
        case 3:
            tx_fifo_level = fifo_length * 3 / 4;
            break;
        case 4:
            tx_fifo_level = fifo_length * 7 / 8;
            break;
        default:
            throw "UART: unknown TX FIFO level: ";
    }
    return 0;
};
void deleteNl2(std::string &targetStr)
{
    const char  LF = '\n';
    const char  RF = '\r';
    std::string destStr;
    for (const auto c : targetStr) {
        if (c != LF && c != RF) {
            destStr += c;
        }
    }
    targetStr = std::move(destStr);
}
void UART::store_char(int x)
{
    if (x == 13) {
        deleteNl2(strbuf);
        if (1 < strbuf.length()) {
            strbufs[strbufs_idx] = strbuf;
            strbufs_idx++;
        }
        strbuf = "";
        if (strbufs_idx == 1000) {
            strbufs_idx = 0;
        }
    } else if (x == '#') {
        strbuf += x;
        strbufs[strbufs_idx] = strbuf;
        strbufs_idx++;
        strbuf = "";
        if (strbufs_idx == 1000) {
            strbufs_idx = 0;
        }
    } else if (0x1f < x && x < 0x7f) {
        strbuf += x;
    }
    printf("%c", x);
}
