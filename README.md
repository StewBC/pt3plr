# Vince Weaver (deater)'s PT3_Player for ProDOS
  
This project is based on the work of deater.  Specifically https://github.com/deater/dos33fsprogs/tree/master/music/pt3_player inside of https://github.com/deater/dos33fsprogs  
  
## Changes
  
1. I modified this to work with ProDOS so that I can add more .pt3 files to the PT3 folder and have the player just play those files.  This version allows for up to 255 .PT3 files.  
2. With so many files, skipping around is more tedious so the "," key will now skip down by 10 files and the "." key will skip up by 10 files.  
3. Pressing 'S' will shuffle all the songs and reset the current song to 1 so the newly shuffled list
plays from start.  
  
## Prerequisites:  
* cc65 is needed tp compile the code.  
* GNU Make.  
* Cadius - https://github.com/mach-kernel/cadius/releases - V1.4.5 is what I am using - This version understands the #nnXXXX appended to file names.  
  
## Assumptions:  
This was only tested on Windows, so Makefile-po.mk has the line:  
CA ?= cadius-145.exe  
Also, this make of a po does not work if there is an sh.exe in the path - Cadius gives this error:  
cadius-145.exe addfile pt3plr.po /pt3plr pt3plr.system#FF2000  
C:\users\swessels\Apps\bin\cadius-145.exe v 1.4.5 (c) Brutal Deluxe 2011-2013.  
  - Add file 'pt3plr.system#FF2000' :  
  Error : Invalid Prodos Volume name : C:  
  
## Building  
  
the command:  
make po  
will make a file called pt3plr.po in this folder.  
  
If Cadius is not an option, just:  
make  
add all of the files in pt3/ to a disk in a folder /pt3/  
The *.pt3 files don't need a type and aux, it's not used  
  
Add pt3plr.apple2 as pt3plr with type BIN (06) and AUX $1000  
run cl65 --print-target-path  
Append to the output the following "\apple2\util\loader.system" and that's the loader for pt3plr  
Add that file, i.e. on Unix echo $(cl65 --print-target-path)/apple2/util/loader.system  
as pt3plr.system with type SYS ($FF) and aux $2000  
  
If using Windows and AppleWin, GNU sed can be used to generate a symbol file.  
Make sure the path to sed.exe is set up as an environment variable named SED.  
The environment variable APPLEWIN_HOME needs to contain the full path to AppleWin.exe  
Uncomment:  
PREEMUCMD := $(SED) "s/^al \([[0-9A-F]\+\)\ \./\1 /g" $(NAME).apple2.lbl > $(APPLEWIN_HOME)\A2_USER1.SYM  
  
With that, make test (or make po test) will also launch AppleWin.  
Make sure to set ptrplr.po as a hard disk in AppleWin.  F7 will show code with symbols.  
  
Stefan Wessels  
swessels@email.com  
5 June 2023  
