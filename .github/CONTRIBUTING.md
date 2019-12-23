## Contributing

Hi there! I'm glad you're interested in helping with this project.
Here are some things I've been asked how to contribute and a brief explanation of how each work.

### Music

The sound driver used is XGM from SGDK. The only real restrictions for this are:

- The file must be a VGM, created in Deflemask or similar
- The file should be a reasonable size (depends on how many WAV samples are needed)

Also, please provide the source tracker file if possible.

### Graphics

The Mega Drive does not draw bitmaps. It draws 8x8 tiles that use one of 4 available palettes.
When adjusting an image in the `res/` folder make sure it stays in "indexed color" mode, and also
that the order of the colors don't change.

### Translations

In terms of just translating the game, you can translate the PC (freeware) version 
and give it to me to convert. There might already be one too.

Getting it into the game is the hard part, cause I need to make a new version of tscomp for
that language, then change the code to handle any extra glyphs that weren't in English.
I'd like to document that process eventually...
