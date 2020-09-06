# FAQ

## Tech Support / Common Issues

### I'm stuck
Take a look at one of [these walkthroughs](https://www.cavestory.org/guides-and-faqs/list.php).

### The game feels slow
The original freeware Cave Story runs at 50 FPS. Cave Story+ runs at 60 FPS, but
the speeds and timings are unchanged causing the game to be faster.
This port DOES make adjustments to those timings so it feels like the original even on NTSC.
If you prefer the faster speed of the Nicalis ports, enable the "CS+ Speed (NTSC Only)"
setting in the config menu.

### The save disks are missing. What's going on?
Your cart or emulator does not support the type of SRAM used by the game.
Check [COMPATIBILITY.md](/andwn/cave-story-md/blob/master/doc/COMPATIBILITY.md)
for a list of tested carts and emulators.

Technical explanation: The following sequence of events occurred when the game booted.
- Read 4 bytes from a specific spot in SRAM and compare it to `CSMD` and `TEST`
- It's not `CSMD`, so there's no save data yet
- It's not `TEST`, so SRAM hasn't been validated yet
- Write the 4 bytes `TEST` to the same spot in SRAM and read it back
- What was read back was not `TEST`, SRAM is malfunctioning

### I can only pick English or Japanese, where are the other languages?
In order to support additional languages, I had to make the ROM larger than 4MB.
If your cart or emulator does not support the Sega SSF mapper, then the game will not be
able to access the data for those new languages. 

Consider using the BlastEm emulator or Mega Everdrive X3/5/7 cart. There's sure to be more that
have proper support, these are just what I test on. If this turns out to be a really pervasive
issue I will consider splitting the languages into multiple ROMs instead of using the mapper.

### You got something wrong/inaccurate
Because this is a "port", the moment someone finds out about it and plays it they hyper-focus
on finding inaccuracies. On multiple occasions people have bombarded this repository
with nearly 100 issues in a short timeframe with various inaccuracies they found.
A good number of them are helpful, and things I can actually fix.
However, there have been times where people point out intentional changes,
listed in [DIFFERENCES.md](/andwn/cave-story-md/blob/master/doc/DIFFERENCES.md),
or something really nitpicky. One time, someone got so trigger happy with the "New Issue"
button that they reported multiple issues twice because they forgot they already reported it.

I'm thankful that people are willing to go out of their way to inform me about inaccuracies,
but would prefer it to be done in moderation. Try to keep it to 1 or 2 issues per day. 
If you find multiple problems in a single area, combine them into one issue. 
If Curly is a pixel too far to the left when she pulls her gun out, consider that I might not care.

### I don't like the music
This isn't an issue. Don't open an issue to say this.

### Can you add new languages and game modes? What about Wind Fortress?
I am not accepting feature requests at this time.
Whether I add something depends on if I feel like it or not.
The Nicalis additions are unlikely. I don't want to give them a reason to come after me.


## General Questions

### Is this an official port?
No. Pixel probably doesn't even know it exists.

### Why did you make it?
I was learning MD dev and my first project was a simple music player.
At the time I was also playing some Cave Story mods. I thought it would be cool to make
one myself, so I started reading about it and how the game works under the hood.
I started noticing things about the game resources, like most graphics had 16 color palettes,
internal resolution was 320x240, it used fixed point integer math, etc.
My music player got turned into a Cave Story map viewer that played a few BGM from the game.
I fell down a slippery slope and added objects, the player class, a TSC interpreter,
and eventually the whole damn game.

### Why not SNES?
I'm not very interested in developing for that console. However, I'd really like to write an ORG
player for the SPC700 someday. I think you could get it to sound really close to the original.
It's almost like Organya was made for that chip.

### When will it be "finished"?
When there are few enough bugs that it doesn't bother me anymore.

### Is there or will there be physical release?
There is no physical release. I do not endorse selling carts with this game on it,
but feel free to make them for yourself or friends if you want.

### Can I mod this?
If you're just asking for permission: Yes.

If you're asking whether it's possible to try and gauge the difficulty:
![you're gonna have a bad time.](badtime.png)

### Can I use your code in my commercial game?
Short answer is yes. Check the license. Note that the TSC scripts containing game dialogue,
while technically "code", are considered assets owned by Studio Pixel. If you decide to use
TSC syntax in your game because you like it or something, make sure you write your own scripts.

### I have an idea for a Genesis game can you code it for free?
No.
