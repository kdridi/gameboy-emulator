#include <cart.h>

typedef struct
{
    char filename[1024];
    u64 rom_size;
    u8 *rom_data;
    rom_header *header;
} cart_context;

static cart_context ctx;

static const char *ROM_TYPES[0x100] = {
    [0x00] = "ROM ONLY",
    [0x01] = "MBC1",
    [0x02] = "MBC1+RAM",
    [0x03] = "MBC1+RAM+BATTERY",
    [0x05] = "MBC2",
    [0x06] = "MBC2+BATTERY",
    [0x08] = "ROM+RAM",
    [0x09] = "ROM+RAM+BATTERY",
    [0x0B] = "MMM01",
    [0x0C] = "MMM01+RAM",
    [0x0D] = "MMM01+RAM+BATTERY",
    [0x0F] = "MBC3+TIMER+BATTERY",
    [0x10] = "MBC3+TIMER+RAM+BATTERY",
    [0x11] = "MBC3",
    [0x12] = "MBC3+RAM",
    [0x13] = "MBC3+RAM+BATTERY",
    [0x19] = "MBC5",
    [0x1A] = "MBC5+RAM",
    [0x1B] = "MBC5+RAM+BATTERY",
    [0x1C] = "MBC5+RUMBLE",
    [0x1D] = "MBC5+RUMBLE+RAM",
    [0x1E] = "MBC5+RUMBLE+RAM+BATTERY",
    [0x20] = "MBC6",
    [0x22] = "MBC7+SENSOR+RUMBLE+RAM+BATTERY",
    [0xFC] = "POCKET CAMERA",
    [0xFD] = "BANDAI TAMA5",
    [0xFE] = "HuC3",
    [0xFF] = "HuC1+RAM+BATTERY",
};

static const char *LIC_CODE[0xA5] = {
    [0x00] = "None",
    [0x01] = "Nintendo R&D1",
    [0x08] = "Capcom",
    [0x13] = "Electronic Arts",
    [0x18] = "Hudson Soft",
    [0x19] = "b-ai",
    [0x20] = "kss",
    [0x22] = "pow",
    [0x24] = "PCM Complete",
    [0x25] = "san-x",
    [0x28] = "Kemco Japan",
    [0x29] = "seta",
    [0x30] = "Viacom",
    [0x31] = "Nintendo",
    [0x32] = "Bandai",
    [0x33] = "Ocean/Acclaim",
    [0x34] = "Konami",
    [0x35] = "Hector",
    [0x37] = "Taito",
    [0x38] = "Hudson",
    [0x39] = "Banpresto",
    [0x41] = "Ubi Soft",
    [0x42] = "Atlus",
    [0x44] = "Malibu",
    [0x46] = "angel",
    [0x47] = "Bullet-Proof",
    [0x49] = "irem",
    [0x50] = "Absolute",
    [0x51] = "Acclaim",
    [0x52] = "Activision",
    [0x53] = "American sammy",
    [0x54] = "Konami",
    [0x55] = "Hi tech entertainment",
    [0x56] = "LJN",
    [0x57] = "Matchbox",
    [0x58] = "Mattel",
    [0x59] = "Milton Bradley",
    [0x60] = "Titus",
    [0x61] = "Virgin",
    [0x64] = "LucasArts",
    [0x67] = "Ocean",
    [0x69] = "Electronic Arts",
    [0x70] = "Infogrames",
    [0x71] = "Interplay",
    [0x72] = "Broderbund",
    [0x73] = "sculptured",
    [0x75] = "sci",
    [0x78] = "THQ",
    [0x79] = "Accolade",
    [0x80] = "misawa",
    [0x83] = "lozc",
    [0x86] = "Tokuma Shoten Intermedia",
    [0x87] = "Tsukuda Original",
    [0x91] = "Chunsoft",
    [0x92] = "Video system",
    [0x93] = "Ocean/Acclaim",
    [0x95] = "Varie",
    [0x96] = "Yonezawa/s'pal",
    [0x97] = "Kaneko",
    [0x99] = "Pack in soft",
    [0xA4] = "Konami (Yu-Gi-Oh!)",
};

const char *cart_lic_name()
{
    const char *result = NULL;
    if (ctx.header->lic_code < 0xA5)
        result = LIC_CODE[ctx.header->lic_code];
    return result ? result : "Unknown";
}

const char *cart_type_name()
{
    const char *result = ROM_TYPES[ctx.header->type];
    return result ? result : "Unknown";
}

bool cart_load(const char *cart)
{
    snprintf(ctx.filename, sizeof(ctx.filename), "%s", cart);

    FILE *fp = fopen(ctx.filename, "rb");
    if (!fp)
    {
        printf("Failed to open: %s\n", ctx.filename);
        return false;
    }

    printf("Opened: %s\n", ctx.filename);

    fseek(fp, 0, SEEK_END);
    ctx.rom_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    ctx.rom_data = calloc(ctx.rom_size, sizeof(u8));
    assert(ctx.rom_data != NULL);
    size_t read_size = fread(ctx.rom_data, sizeof(u8), ctx.rom_size, fp);
    assert(read_size == ctx.rom_size);

    int err = fclose(fp);
    assert(err == 0);

    ctx.header = (rom_header *)(ctx.rom_data + 0x100);
    ctx.header->title[15] = '\0';

    printf("Cartridge Loaded:\n");
    printf("\t Title        : %s\n", ctx.header->title);
    printf("\t Type         : %2.2X (%s)\n", ctx.header->type, cart_type_name());
    printf("\t ROM Size     : %d KB\n", 32 << ctx.header->rom_size);
    printf("\t RAM Size     : %2.2X\n", ctx.header->ram_size);
    printf("\t License Code : %2.2X (%s)\n", ctx.header->lic_code, cart_lic_name());
    printf("\t ROM Version  : %2.2X\n", ctx.header->version);

    // Calculate checksum using the same algorithm as the bootrom
    // Here's the detailled algorithm: https://gbdev.io/pandocs/#0104-rom-header
    u16 x = 0;
    for (u16 i = 0x0134; i <= 0x014C; i++)
        x = x - ctx.rom_data[i] - 1;

    printf("\t Checksum     : %2.2X (%s)\n", ctx.header->checksum, x & 0xFF ? "PASSED" : "FAILED");

    return true;
}

u8 cart_read(u16 address)
{
    return ctx.rom_data[address];
}

void cart_write(u16 address, u8 value)
{
    printf("UNSUPPORTED cart_write at address 0x%04X\n", address);
    // NO_IMPL();
}
