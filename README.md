# Powerslaves -- The API strikes back.
_Taking Powersaves as a slave to your will._  
(Name courtesy of Normmatt)

## What is this?
It's a little thing. Datel's Powersaves can do more than just
reading and writing save files, but they won't tell you that.

At the moment, this is just a small library that abstracts away having to deal
with hidapi, aong with a few examples that use the API. This could be useful to
anyone who wants to communicate with a DS or 3DS cartridge. Personally, a good
usecase of this can be seen in the ak2itool example.

This is licenced under the MIT licence, contained in LICENCE.

### Why bother?
I have had some issues implementing this. Some were stupid mistakes.
I'd like to save other people trouble in case they want to do anything similar.

~~I've also never made a library before, so, experience?~~

### TODO
 - Make absolutely sure this is cross platform. This was developed and tested
   on linux based distros, so Windows and Mac OS should definately be tested.

## Compiling
Grab and install hidapi. It's a requirement for this. Run make. Done.
That was easy. Or, at least, it should've been.
Let me know if it wasn't as easy as it should've been in the issues.

Yes, it's a quicky made Makefile. If you have suggestions for a good build
system that handles every platform, let me know.

### Usage
Link with your project, and (hopefully) never have to think about hid devices
ever again.

#### header 'tool'
`Usage: ./header [-ntc] [-l length] [-o filename]`

`-n` for NTR mode. `-t` for TWL mode. `-c` for CTR mode.

#### ak2itool
`Usage: ./ak2itool [-d] [-l length] [-f filename]`

`-d` dumps ak2i flash to `ak2i_flash.bin`, `-f` takes a filename and writes the
contents of that file to ak2i flash.

## Credits
Normmatt, for the C# code this was based off of.  
TobiX, for sharing some small details about commands that appearantly exist but
are undocumented. And for reminding me about this once again. <\_<  
SeddiHz, for being the one to test the original application for me before I
could test with my own yet-to-arrive powersaves. >\_>  
hedgeberg, for inspiring me to work on this and similar projects. (NTRPi)  
Everyone in #Cakey(-ot) on freenode.  
The developers of hidapi, so that I didn't have to create the relevant code.  
Datel, for making a device that was more flexible than they probably intended.  
