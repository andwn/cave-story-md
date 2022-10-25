/*
    medloadlx.c
    POSIX C reimplementation of KrikZZ MED loader (which seems to have been written in some foreign proprietary language)
    Copyright Bryan E. Topp ("betopp"), 2013
    <betopp@betopp.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


int main(int argc, const char **argv)
{
    if(argc != 4)
    {
        printf("\nmedloadlx v2 - by betopp, 2013 <betopp@betopp.com>\n");
        printf("Usage: %s <image type> <rom image> <serial port device>\n", argv[0]);
        printf("Supported image types:\n");
        printf("sms\tos\tcd\tm10\tmd\n");
        printf("(use md if you're not sure)\n\n");

        exit(0);

    }



    //find out what kind of run command we are giving to the MED
    const char *run_cmd;
    if(!strcmp(argv[1], "sms"))
        run_cmd = "*rs";
    else if(!strcmp(argv[1], "os"))
        run_cmd = "*ro";
    else if(!strcmp(argv[1], "cd"))
        run_cmd = "*rc";
    else if(!strcmp(argv[1], "m10"))
        run_cmd = "*rM";
    else if(!strcmp(argv[1], "md"))
        run_cmd = "*rm";
    else
    {
        printf("invalid image type specified.\n");
        exit(-1);
    }

    //open ROM image
    FILE *romfile = fopen(argv[2], "rb");
    if(!romfile)
    {
        printf("failed to open ROM image %s\n", argv[2]);
        exit(-1);
    }

    //seek to end, find out how long file is
    fseek(romfile, 0L, SEEK_END);
    long int rom_size = ftell(romfile);

    //limit ROM size to that of the MED RAM
    if(rom_size > 0xF00000)
    {
        printf("ROM file too big for MED!\n");
        exit(-1);
    }

    //we must write 64K blocks at a time to the MED. round up.
    int rom_size_rounded = rom_size;
    if(rom_size_rounded % 65536 != 0)
    {
        rom_size_rounded = (((rom_size_rounded / 65536)+1) * 65536);
    }
    unsigned char rom_size_blocks = rom_size_rounded / 65536;

    printf("ROM size sent will be %d, in %d blocks.\n", rom_size_rounded, rom_size_blocks);


    //seek back to beginning of ROM
    rewind(romfile);

    //allocate and zero memory
    unsigned char *rombuffer = (unsigned char*)malloc(rom_size_rounded);
    if(!rombuffer)
    {
        printf("error allocating memory for ROM file.\n");
        exit(-1);
    }
    memset(rombuffer, 0xFF, rom_size_rounded);

    //read ROM file into buffer
    long int bytes_read = fread(rombuffer, 1, rom_size, romfile);
    if(bytes_read != rom_size)
    {
        printf("read error while loading ROM file from disk.\n");
        exit(-1);
    }

    printf("ROM read successfully.\n");

    //open serial device
    int tty = open(argv[3], O_RDWR | O_NOCTTY);

    if(tty < 0)
    {
        printf("failed to open serial port %s\n", argv[3]);
        exit(-1);
    }

    printf("Serial port opened.\n");

    //flush old data
    if(tcflush(tty, TCIOFLUSH) != 0)
    {
        printf("failed to flush port\n");
        exit(-1);
    }

    printf("Trying to set raw mode...\n");

    //set raw mode
    struct termios term_attr;
    tcgetattr(tty, &term_attr);
    cfmakeraw(&term_attr);
    tcsetattr(tty, 0, &term_attr);

    printf("Hopefully raw mode is set now. Sending reset command...\n");

    //send reset command.
    const char *reset_cmd = "    *T";

    if(write(tty, reset_cmd, 6) != 6)
    {
        printf("failed to send reset command\n");
        exit(-1);
    }

    tcdrain(tty);

    //read response (should be "k")
    unsigned char response_buffer[4];
    if(read(tty, response_buffer, 1) != 1)
    {
        printf("failed to read response to reset command\n");
        exit(-1);
    }
    if(response_buffer[0] != 'k')
    {
        printf("received incorrect response to reset command\n");
        printf("expected 'k', got '%4s'\n", response_buffer);
        exit(-1);
    }
    response_buffer[0] = '\0';
    printf("Reset command sent; response OK.\n");



    //send "game" command and length-in-blocks
    if(write(tty, "*g", 2) != 2)
    {
        printf("failed to send game command\n");
        exit(-1);
    }


    if(write(tty, &rom_size_blocks, 1) != 1)
    {
        printf("failed to send ROM size\n");
        exit(-1);
    }

    tcdrain(tty);



    //read response (should be "k")
    if(read(tty, response_buffer, 1) != 1)
    {
        printf("failed to read response to game command\n");
        exit(-1);
    }
    if(response_buffer[0] != 'k')
    {
        printf("received incorrect response to game command\n");
        printf("expected 'k', got '%4s'\n", response_buffer);
        exit(-1);
    }
    response_buffer[0] = '\0';
    printf("Game and ROM size commands sent; response OK. Sending blocks.\n");



    //send a block at a time
    int data_sent;
    for(data_sent = 0; data_sent < rom_size_rounded; data_sent+=65536)
    {
        printf("Sending block %d of %d...\n", (data_sent/65536)+1, (rom_size_rounded/65536));

        //send the block and flush it

        if(write(tty, rombuffer+data_sent, 65536) != 65536)
        {
            printf("failed to send block\n");
            exit(-1);

        }


        tcdrain(tty);

    }

    printf("done sending blocks...\n");



    //read response (should be "d")
    if(read(tty, response_buffer, 1) != 1)
    {
        printf("failed to read response to blocks\n");
        exit(-1);
    }
    if(response_buffer[0] != 'd')
    {
        printf("received incorrect response to blocks\n");
        printf("expected 'd', got '%4s'\n", response_buffer);
        exit(-1);
    }
    response_buffer[0] = '\0';

    printf("Block data sent, response OK.\n");




    //send "run" command
    if(write(tty, run_cmd, 3) != 3)
    {
        printf("failed to send run command\n");
        exit(-1);
    }

    //read response (should be "k")
    if(read(tty, response_buffer, 1) != 1)
    {
        printf("failed to read response to run command\n");
        exit(-1);
    }
    if(response_buffer[0] != 'k')
    {
        printf("received incorrect response to run command\n");
        printf("expected 'k', got '%4s'\n", response_buffer);
        exit(-1);
    }
    response_buffer[0] = '\0';



    printf("Run command sent.\n");

}
