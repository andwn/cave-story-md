## Contributing

Hi there! I'm glad you're interested in helping with this project.
Here are some things I've been asked how to contribute and a brief explanation of how each work.

### Music

The sound driver used is XGM from SGDK. The only real restrictions for this are:

- The file must be a VGM, created in Deflemask or similar
- The channel FM6 must always be set to DAC mode (or you'll kill sound effects)
- The file should be a reasonable size (depends on how many WAV samples are needed)

Also, please provide the source tracker file if possible.

### Graphics

The Mega Drive does not draw bitmaps. It draws 8x8 4bpp tiles that use one of 4 available palettes.
When adjusting an image in the `res/` folder make sure it stays in "indexed color" mode, and also
that the order of the colors don't change.

### Translations

In terms of just translating the game, you can translate the PC (freeware) version 
and give it to me to convert. There might already be one too.

Getting it into the game gets more complicated based on
- The encoding of the TSC files: I'll have to look it up on Wikipedia and learn how it works
  - So far tscomp supports: ASCII, Shift-JIS, Windows-1252
- The glyphs that need to be rendered in the game
  - European/Latin languages should be pretty much covered
  - For Chinese and Korean I can redo the same thing I did with Japanese (just with different encodings)
  - A font for Russian/Ukranian/Bulgarian is in progress
  - I have no idea what to do for RTL languages
- My level of burnout

In addition to the TSC files, there are the stage names embedded in doukutsu.exe.
If those are translated give me the patched EXE so I can extract them.
