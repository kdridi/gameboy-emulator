#include <bus.h>
#include <cart.h>
#include <ram.h>
#include <cpu.h>
#include <io.h>

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
    if (address < 0x8000)
    {
        return cart_read(address);
    }
    else if (address < 0xA000)
    {
        // TODO: Character RAM / Video RAM
        printf("UNSUPPORTED bus_read(0x%04X)\n", address);
        // NO_IMPL(0);
        return 0;
    }
    else if (address < 0xC000)
    {
        // TODO: Cartridge RAM
        return cart_read(address);
    }
    else if (address < 0xE000)
    {
        return wram_read(address);
    }
    else if (address < 0xFE00)
    {
        // TODO: Echo RAM
        return 0;
    }
    else if (address < 0xFEA0)
    {
        // TODO: Object Attribute Memory (OAM)
        printf("UNSUPPORTED bus_read(0x%04X)\n", address);
        // NO_IMPL(0);
        return 0;
    }
    else if (address < 0xFF00)
    {
        // TODO: Unusable Memory
        printf("UNSUPPORTED bus_read(0x%04X)\n", address);
        NO_IMPL(0);
    }
    else if (address < 0xFF80)
    {
        return io_read(address);
    }
    else if (address < 0xFFFF)
    {
        // TODO: Zero Page
        return hram_read(address);
    }
    else if (address == 0xFFFF)
    {
        // Interrupt Enable Flag
        return cpu_get_ie_register();
    }
    assert(false);
}

void bus_write(u16 address, u8 value)
{
    if (address < 0x8000)
    {
        cart_write(address, value);
        return;
    }
    else if (address < 0xA000)
    {
        // TODO: Character RAM / Video RAM
        printf("UNSUPPORTED bus_write(%04X)\n", address);
        // NO_IMPL();
        return;
    }
    else if (address < 0xC000)
    {
        // TODO: Cartridge RAM
        cart_write(address, value);
    }
    else if (address < 0xE000)
    {
        wram_write(address, value);
        return;
    }
    else if (address < 0xFE00)
    {
        return;
    }
    else if (address < 0xFEA0)
    {
        // TODO: Object Attribute Memory (OAM)
        printf("UNSUPPORTED bus_write(%04X)\n", address);
        // NO_IMPL();
        return;
    }
    else if (address < 0xFF00)
    {
        // TODO: Unusable Memory
        printf("UNSUPPORTED bus_write(%04X)\n", address);
        NO_IMPL();
    }
    else if (address < 0xFF80)
    {
        io_write(address, value);
        return;
    }
    else if (address < 0xFFFF)
    {
        // TODO: Zero Page
        hram_write(address, value);
        return;
    }
    else if (address == 0xFFFF)
    {
        // Interrupt Enable Flag
        cpu_set_ie_register(value);
        return;
    }
    assert(false);
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
