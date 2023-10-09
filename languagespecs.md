# Language Specs
### Here are all of the instructions for the assembly langauge  
(This only lists the proper syntax, for more info on each instruction go to http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)  
Each instruction is made of parts, mostly looking like so:  
`Instruction Arg1 Arg2 Arg3`  
All of the args are optional depending on the instruction.  
Arg rules are as follows:  
Bytes will have the `%` prefix.  
Addresses will have the `@` prefix.  
Registers will be listed as `V0` to `VF`.