# Cosmos CP1
I found an old computer from the 80s. <br>
It has 128 Words of RAM used by both the code and the data. <br>
It also has a very simplistic reduced instruction set (RISC). <br>

# Instruction Set
| name  | opcode |                        description                        |
|:-----:|:------:|:---------------------------------------------------------:|
|  HLT  | 01.000 |                    Halts the processor                    |
| PRINT | 02.000 |           Prints the contents of the accumulator          |
| SLEEP | 03.xxx | Sleeps for xxx milliseconds                               |
| LDC   | 04.xxx | Loads a constant into the accumulator                     |
| LDA   | 05.xxx | Loads the Contents of [xxx] into the accumulator          |
| STA   | 06.xxx | Stores the Contents of the accumulator to [xxx]           |
| ADD   | 07.xxx | Adds the Contents of [xxx] to the accumulator             |
| SUB   | 08.xxx | Subtracts the Contents of [xxx] from the accumulator      |
| JMP   | 09.xxx | Jumps to the address xxx                                  |
| CMPE  | 10.xxx | Compares the accumulator to [xxx] for equality            |
| JC    | 11.xxx | Jumps to xxx if last compare was true                     |
| CMPGR | 12.xxx | Tests if the accumulator is greather than [xxx]           |
| CMPLE | 13.xxx | Tests if the accumulator is less then [xxx]               |
| NEG   | 14.000 | Negates the Contents of the accumulator                   |
| AND   | 15.xxx | Bitwise AND the accumulator with [xxx]                    |
| POUT  | 18.000 | Writes the Contents of the accumulator to the Output port |
































