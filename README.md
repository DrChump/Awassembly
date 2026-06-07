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

Note: macros expand into multiple instructions which do not act as a single statement

| macro    | argument                                              | description                                                               |
|----------|-------------------------------------------------------|---------------------------------------------------------------------------|
| PRINT    | String of chars in double quotes                      | PRINTS a string contained in double quotes                                |
| OVER     | 5 bit unsigned integer [0,30]                         | copies value from specified depth on top of the stack (OVER 0 == dpl)     |
| SURF     | 5 bit unsigned integer [0,31]                         | move the element at specified depth to the surface (SURF 0 has no effect) |
| REPEAT   | two integers making up the start and end of the range | start of a REPEAT/END block which is used for code generation             |
| ITERATOR |                                                       | can be used in a REPEAT/END block to get the current index in [start,end] |
| END      |                                                       | marks the end of REPEAT/END block                                         |

The PRINT macro also converts some chars automatically to AwaSCII. For example:
```
PRINT "This is a very short example: My knuckles crack."
```
turns into
```
This is a bery short e ample; My cnuccles cracc.
```
