#include <bus.h>
#include <cart.h>
#include <ram.h>
#include <cpu.h>
#include <io.h>
#include <ppu.h>
#include <dma.h>

// 0x0000 - 0x3FFF  :   16 KiB ROM bank 00                  :   From cartridge, usually a fixed bank
// 0x4000 - 0x7FFF  :   16 KiB ROM Bank 01~NN               :   From cartridge, switchable bank via mapper (if any)
// 0x8000 - 0x9FFF  :   8 KiB Video RAM (VRAM)              :   In CGB mode, switchable bank 0/1
// 0xA000 - 0xBFFF  :   8 KiB External RAM                  :   From cartridge, switchable bank if any
// 0xC000 - 0xCFFF  :   4 KiB Work RAM (WRAM)               :
// 0xD000 - 0xDFFF  :   4 KiB Work RAM (WRAM)               :   In CGB mode, switchable bank 1~7
// 0xE000 - 0xFDFF  :   Mirror of C000~DDFF (ECHO RAM)      :   Nintendo says use of this area is prohibited.
// 0xFE00 - 0xFE9F  :   Object attribute memory (OAM)       :
// 0xFEA0 - 0xFEFF  :   Not Usable                          :   Nintendo says use of this area is prohibited
// 0xFF00 - 0xFF7F  :   I/O Registers                       :
// 0xFF80 - 0xFFFE  :   High RAM (HRAM)                     :
// 0xFFFF - 0xFFFF  :   Interrupt Enable register (IE)      :

// $0000 - $00FF    Restart and Interrupt Vectors
// $0100 - $014F    Cartridge Header Area
// $0150 - $3FFF    Cartridge ROM - Bank 0 (fixed)
// $4000 - $7FFF    Cartridge ROM - Switchable Banks 1-xx
// $8000 - $97FF    Character RAM
// $9800 - $9BFF    BG Map Data 1
// $9C00 - $9FFF    BG Map Data 2
// $A000 - $BFFF    Cartridge RAM (If Available)
// $C000 - $CFFF    Internal RAM - Bank 0 (fixed)
// $D000 - $DFFF    Internal RAM - Bank 1-7 (switchable - CGB only)
// $E000 - $FDFF    Echo RAM - Reserved, Do Not Use
// $FE00 - $FE9F    OAM - Object Attribute Memory
// $FEA0 - $FEFF    Unusable Memory
// $FF00 - $FF7F    Hardware I/O Registers
// $FF80 - $FFFE    Zero Page - 127 bytes
// $FFFF            Interrupt Enable Flag

u8 bus_read(u16 address)
{
    assert(address >= 0x0000);
    if (BETWEEN(address, 0x0000, 0x7FFF))
        return cart_read(address);

    assert(address >= ADDR_VRAM_START);
    if (BETWEEN(address, ADDR_VRAM_START, ADDR_VRAM_END))
        return ppu_vram_read(address);

    assert(address >= 0xA000);
    if (BETWEEN(address, 0xA000, 0xBFFF))
        return cart_read(address);

    assert(address >= 0xC000);
    if (BETWEEN(address, 0xC000, 0xDFFF))
        return wram_read(address);

    assert(address >= 0xE000);
    if (BETWEEN(address, 0xE000, 0xFDFF))
        return 0;

    assert(address >= ADDR_OAM_START);
    if (BETWEEN(address, ADDR_OAM_START, ADDR_OAM_END) && dma_transfering())
        return 0xFF;

    assert(address >= ADDR_OAM_START);
    if (BETWEEN(address, ADDR_OAM_START, ADDR_OAM_END))
        return ppu_oam_read(address);

    assert(address >= 0xFEA0);
    if (BETWEEN(address, 0xFEA0, 0xFEFF))
        // NO_IMPL(0);
        return 0;

    assert(address >= 0xFF00);
    if (BETWEEN(address, 0xFF00, 0xFF7F))
        return io_read(address);

    assert(address >= 0xFF80);
    if (BETWEEN(address, 0xFF80, 0xFFFE))
        return hram_read(address);

    assert(address == 0xFFFF);
    return cpu_get_ie_register();
}

void bus_write(u16 address, u8 value)
{
    assert(address >= 0x0000);
    if (BETWEEN(address, 0x0000, 0x7FFF))
        return cart_write(address, value);

    assert(address >= ADDR_VRAM_START);
    if (BETWEEN(address, ADDR_VRAM_START, ADDR_VRAM_END))
        return ppu_vram_write(address, value);

    assert(address >= 0xA000);
    if (BETWEEN(address, 0xA000, 0xBFFF))
        return cart_write(address, value);

    assert(address >= 0xC000);
    if (BETWEEN(address, 0xC000, 0xDFFF))
        return wram_write(address, value);

    assert(address >= 0xE000);
    if (BETWEEN(address, 0xE000, 0xFDFF))
        return;

    assert(address >= ADDR_OAM_START);
    if (BETWEEN(address, ADDR_OAM_START, ADDR_OAM_END) && dma_transfering())
        return;

    assert(address >= ADDR_OAM_START);
    if (BETWEEN(address, ADDR_OAM_START, ADDR_OAM_END))
        return ppu_oam_write(address, value);

    assert(address >= 0xFEA0);
    if (BETWEEN(address, 0xFEA0, 0xFEFF))
        // NO_IMPL();
        return;

    assert(address >= 0xFF00);
    if (BETWEEN(address, 0xFF00, 0xFF7F))
        return io_write(address, value);

    assert(address >= 0xFF80);
    if (BETWEEN(address, 0xFF80, 0xFFFE))
        return hram_write(address, value);

    assert(address == 0xFFFF);
    return cpu_set_ie_register(value);
}

u16 bus_read16(u16 address)
{
    u16 lo = bus_read(address);
    u16 hi = bus_read(address + 1);

    return lo | (hi << 8);
}

void bus_write16(u16 address, u16 value)
{
    bus_write(address + 1, (value >> 8) & 0xFF);
    bus_write(address, value & 0xFF);
}
