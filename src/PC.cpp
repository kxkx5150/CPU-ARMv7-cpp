#include "PC.h"
#include <cstdint>
#include <string>

PC::PC()
{
}
PC::~PC()
{
    delete memctlr;
}
void PC::boot()
{
    gic     = new IRQ(0x1e000000);
    timer0  = new DTimer(0x10011000, irq_base + 2, gic);
    uart0   = new UART(0, 0x10009000, irq_base + 5, gic);
    io      = new IO(gic, timer0, uart0);
    memctlr = new Mem(io);
    cpu     = new ARMV7_CPU(memctlr);

    load_binary("bin/vexpress-armjs.dtb", 0x100);
    load_binary("bin/initramfs.cpio.lzo", 0x00800000);
    load_binary("bin/Image-3.10.29", 0x00008000);
    printf("load_binary\n");

    cpu->regs[0]  = 0;
    cpu->regs[1]  = 2272;
    cpu->regs[2]  = 0x100;
    cpu->regs[15] = 0x00008000;
    cpu->cpsr.m   = 0x13;
    is_booted     = true;
    is_running    = true;

    printf("\n\n************************\n");
    printf("************************\n");
    printf("****** Boot Linux ******\n");
    printf("************************\n");
    printf("************************\n\n\n");
}
void PC::load_binary(string path, int64_t phyaddr)
{
    FILE *f = fopen(path.c_str(), "rb");
    fseek(f, 0, SEEK_END);
    const int size = ftell(f);
    fseek(f, 0, SEEK_SET);
    auto buffer = new uint8_t[size];
    fread(buffer, size, 1, f);

    for (int64_t i = 0; i < size; i++) {
        memctlr->st_byte(phyaddr + i, buffer[i]);
    }

    delete[] buffer;
    fclose(f);
}
void PC::run()
{
    loop();
}
void PC::loop()
{
    int64_t timeslice  = 100000;
    int64_t n_executed = 0;
    int64_t inst       = 0;
    string  inst_name  = "";

    do {
        if (!cpu->cpsr.i && gic->is_pending()) {
            int64_t irq = gic->pick_interrupt();
            if (irq != 0) {
                n_interrupts += 1;
                cpu->is_halted = false;
                cpu->interrupt();
            }
        }

        cpu->branch_to = 0;
        int64_t pc     = cpu->regs[15];
        try {
            inst = cpu->fetch_instruction(pc);
        } catch (string e) {
            if (e == "PF") {
                cpu->prefetch_abort();
                continue;
            } else {
                throw e;
            }
        }

        if (cpu->is_valid(inst)) {
            try {
                inst_name = cpu->decode(inst, pc);
            } catch (string e) {
                if (e == "UND") {
                    cpu->undefined_instruction();
                    continue;
                } else {
                    throw e;
                }
            }

            if (cpu->cond(inst)) {
                try {
                    cpu->exec(inst_name, inst, pc);
                } catch (string e) {
                    if (e == "PF") {
                        cpu->data_abort();
                        continue;
                    } else if (e == "SUPERVISOR") {
                        cpu->supervisor();
                        continue;
                    } else {
                        throw e;
                    }
                }
            }
        }

        if (cpu->branch_to) {
            cpu->regs[15] = cpu->branch_to;
        } else {
            cpu->regs[15] = pc + 4;
        }

        n_instructions += 1;
        n_executed += 1;
    } while (--timeslice > 0);

    tick += n_executed;
}
