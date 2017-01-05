# Powerslaves -- The API strikes back.
_Taking Powersaves as a slave to your will._  
(Name courtesy of Normmatt)

# IF YOU WANT SOMETHING THAT WORKS, GO TO THE MASTER BRANCH

## What is this?
It's a little thing. Datel's Powersaves can do more than just
reading and writing save files, but they won't tell you that.

At the moment, this is just a small library that abstracts away having to deal
with hidapi. It also doesn't work. Again. If you want to see something that
works, go look at the application in the master branch. It also has a couple of
examples that use the API, but again, doesn't work because something is borked.

Once this works, this could be useful to anyone who wants to communicate with a
DS or 3DS cartridge. Personally, a good example of this is ak2itool in the
examples directory.

This is licenced under the MIT licence, contained in LICENCE.

### Why Bother?
I have had (and am still having) many issues implementing this. Some were
stupid mistakes. I'd like to save other people trouble next time they want to
do something similar.

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
SeddiHz, for being the one to test the original application for me before I
could test with my own yet-to-arrive powersaves. >\_>  
hedgeberg, for inspiring me to work on this and similar projects. (NTRPi)  
Everyone in #Cakey(-ot) on freenode.  
The developers of hidapi, so that I didn't have to work up the relevant code.  
Datel, for making a device that was more flexible than they probably intended.  
