## Contributing

Hi there! I'm glad you're interested in helping with this project.
Here are some things I've been asked how to contribute and a brief explanation of how each work.

### Music

The sound driver used is XGM from SGDK, though I am currently considering migrating to another.
(This means that even if your contribution is accepted, it may eventually be discarded or
transcribed to another format)

For the time being, the only real restrictions for music in the XGM driver are:

- The file must be a VGM, created in Deflemask or similar
- The channel FM6 must always be set to DAC mode (effect 1701)
- The file should be a reasonable size (keep samples to a minimum)

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
  - So far tscomp supports: ASCII, Shift-JIS, Windows-1252 (Europe), Windows-1251 (RU/UA)
- The glyphs that need to be rendered in the game
  - European/Latin languages should be pretty much covered
  - CJK is mostly covered and I have a workflow for adding new glyphs
  - A font for Russian/Ukrainian/Bulgarian is implemented
  - I have no idea what to do for RTL languages, or ligatures
- My level of burnout

In addition to the TSC files, there are some graphics that need to be extracted.
I put those in `res/sprite_loc` and `res/tiles_loc`. The patch files in `res/patch`
need to be edited to refer to these graphics if a localized one is available.

And then there are localized strings in `res/strings`. Much like the graphics, the address
tables in `res/patch` refer to these files per language if available:
- xx_stagename.txt: Displayed on screen after entering a new map (also used by stage select)
- xx_music.txt: Names for music tracks used by Sound Test
- xx_menus.txt: Title Screen, Save Select, Sound Test, and Config menus (hookups still WIP)
- xx_credits.txt: Staff roll shown after clearing the game
