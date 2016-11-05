# PowerSlaves
_Taking Powersaves as a slave to your will._  
(Name courtesy of Normmatt)

## What is this?
It's a little thing. Datel's Powersaves can do more than just
reading and writing save files, but they won't tell you that.

At the moment, all this does is dump the header of a cartridge.
Or, rather, the 0x4000 bytes that you can get up to when
communicating with a cartridge and asking for a header.

As it currently stands, this shouldn't really be useful to most
people. It may be useful to me in debuging NTRPi in it's respective
current form.

This is licenced under the MIT licence, contained in LICENCE.

### TODO
 - Make sure this is cross platform. This was developed and tested
   on linux based distros, so the BSDs should be tested, along
   with Windows and Mac OS.
 - Do *more*.
 - Add behavior profiles, so that it can act as if it's a
   DS, DSi, or 3DS, as they all have slight differences in the
   order of the commands that they send to the cartridge, and some
   things take note of these differences to send different
   information. (Ahem, Datel.)

## Compiling
Grab and install hidapi. It's a requirement for this.
Run make. Done. That was easy. Or, at least, it should've been.
Let me know if it wasn't as easy as it should've been in the issues.

### Usage
Run `./powerslaves` to dump the header of a cartridge to a file.
By default, it dumps 0x1000 bytes to 'header.bin'.
You can change the file it outputs to with `-o` and
change the number of bytes it reads with `-l`.

## Credits
Normmatt, for the C# code this was based off of.  
SeddiHz, for being the one to test for me before I could test with
my own yet-to-arrive powersaves. >\_>  
hedgeberg, for inspiring me to work on this and similar projects. (NTRPi)  
Everyone in #Cakey(-ot) on freenode.  
The developers of hidapi, so that I didn't have to work up the
relevant code.  
Datel, for making a device that was more flexible than they
probably intended.  
