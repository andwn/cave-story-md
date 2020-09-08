## Flash Cart and Emulator Compatibility
This is to keep track of what emulators or hardware CSMD works on.

### Official Consoles
All Mega Drive and Genesis consoles manufactured by Sega should work 100%.
Of course this depends on your flash cart, or whatever you burn the ROM to.

### Flash Carts
| Name                     | Startup  | Beatable | Saving   | Notes                            |
| ------------------------ | -------- | -------- | -------- | -------------------------------- |
| Mega Everdrive X3/5/7/Pro| Yes      | Yes      | Yes      |                                  |
| Everdrive MD             | Yes      | Yes      | Yes      |                                  |
| RetroStage 32MB Cart     | Yes      | Yes      | Yes      | Need firmware update             |
| UMDKv2                   | Yes      | Yes      | ???      | Occasional pixel color corruption |
| Oerg's Cart              | ???      | ???      | ???      | Untested, probably works         |

### Emulator/FPGA in a Box
| Name                     | Startup  | Beatable | Saving   | Notes                            |
| ------------------------ | -------- | -------- | -------- | -------------------------------- |
| Sega Mega Drive Mini     | Yes      | Yes      | Yes      | With Project Lunar               |
| Atgames Consoles         | ???      | ???      | ???      | Untested, probably sounds bad    |

### Emulators
| Name                     | Startup  | Beatable | Saving   | Notes                            |
| ------------------------ | -------- | -------- | -------- | -------------------------------- |
| BlastEm                  | Yes      | Yes      | Yes      | Most tested                      |
| Kega Fusion              | Yes      | Yes      | Yes      |                                  |
| Genesis Plus GX          | Yes\*    | Yes      | Yes      | Outdated downstream emulators may crash |
| PicoDrive                | Yes\*    | Yes      | Yes      | Outdated downstream emulators may crash |
| Gens GS                  | Yes      | Yes      | Yes      | A couple instruments sound funky |
| Regen                    | Yes      | Yes      | No       |                                  |
| Higan core               | ???      | ???      | ???      | Untested, probably works         |
| Exodus                   | ???      | ???      | ???      | Untested                         |
| Dgen                     | ???      | ???      | ???      | Untested                         |
| sega(1)                  | ???      | ???      | ???      | Untested                         |

### Note About SRAM
CSMD writes 8KB of SRAM mapped at 0x200000 using odd bytes.
The ROM header specifies a 32KB range for compatibility purposes.

This is the same mapper used by SGDK. If your emulator/cart can't save in CSMD,
you will also be unable to save properly in other homebrew with >2MB ROM size.
