# Awassembler
Translates Awassembly to Awabytecode

Assembler for AWA5.0
https://github.com/TempTempai/AWA5.0

## Instructions

Read the specification for details.
AWA5.0 is a stack based language.

| tism  | argument                                              | description                                                   |
|-------|-------------------------------------------------------|---------------------------------------------------------------|
| nop   |                                                       | do nothing                                                    |
| prn   |                                                       | pop and print char(s)                                         |
| pr1   |                                                       | pop and print number(s)                                       |
| red   |                                                       | read and push char(s)                                         |
| r3d   |                                                       | read and push number                                          |
| blo   | 8 bit signed integer or AwaSCII char in single quotes | push                                                          |
| sbm   | 5 bit unsigned integer                                | move top element down the stack n positions                   |
| pop   |                                                       | pop                                                           |
| dpl   |                                                       | duplicate top element                                         |
| srn   | 5 bit unsigned integer                                | surround n elements with a bubble                             |
| mrg   |                                                       | merge top two bubbles                                         |
| 4dd   |                                                       | add top two bubbles                                           |
| sub   |                                                       | subtract top two bubbles                                      |
| mul   |                                                       | multiply top two bubbles                                      |
| div   |                                                       | divide top two bubbles (result and remainder)                 |
| cnt   |                                                       | push count of top bubble elements                             |
| lbl   | 5 bit unsigned integer                                | set label                                                     |
| jmp   | 5 bit unsigned integer                                | jump to label                                                 |
| eql   |                                                       | execute next tism if top two bubbles are equal                |
| lss   |                                                       | execute next tism if top bubble is less than second bubble    |
| gr8   |                                                       | execute next tism if top bubble is greater than second bubble |
| trm   |                                                       | terminate                                                     |
| PRINT | String of chars in double quotes                      | PRINTS a string contained in double quotes (not standard)     |

The PRINT macro also converts some chars automatically to AwaSCII. For example:
```
PRINT "This is a very short example: My knuckles crack."
```
turns into
```
This is a bery short e ample; My cnuccles cracc.
```
