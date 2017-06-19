#include <stdint.h>

__attribute__((externally_visible))
const struct {
    char console[16];               /* Console Name (16) */
    char copyright[16];             /* Copyright Information (16) */
    char title_local[48];           /* Domestic Name (48) */
    char title_int[48];             /* Overseas Name (48) */
    char serial[14];                /* Serial Number (2, 12) */
    uint16_t checksum;                   /* Checksum (2) */
    char IOSupport[16];             /* I/O Support (16) */
    uint32_t rom_start;                  /* ROM Start Address (4) */
    uint32_t rom_end;                    /* ROM End Address (4) */
    uint32_t ram_start;                  /* Start of Backup RAM (4) */
    uint32_t ram_end;                    /* End of Backup RAM (4) */
    char sram_sig[2];               /* "RA" for save ram (2) */
    uint16_t sram_type;                  /* 0xF820 for save ram on odd bytes (2) */
    uint32_t sram_start;                 /* SRAM start address - normally 0x200001 (4) */
    uint32_t sram_end;                   /* SRAM end address - start + 2*sram_size (4) */
    char modem_support[12];         /* Modem Support (24) */
    char notes[40];                 /* Memo (40) */
    char region[16];                /* Country Support (16) */
} rom_header = {
    "SEGA MEGA DRIVE ",
    "Andwn   2017.JUN",
    "Doukutsu Monogatari MD                          ",
    "Cave Story MD                                   ",
    "GM ANDYG002-A4",
    0x0000,
    "JD              ",
    0x00000000,
    0x00400000,
    0x00FF0000,
    0x00FFFFFF,
    "RA",
    0xF820,
    0x00200001,
    0x0020FFFF,
    "            ",
    "https://github.com/andwn/cave-story-md\0\0",
#ifdef PAL
	"E               "
#else
    "JU              "
#endif
};
