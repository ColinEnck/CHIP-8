// NOT DONE YET

use std::env;
use std::fs::File;
use std::io::prelude::*;

struct Instruction<'a> {
    op: &'a str,
    arg1: &'a str,
    arg2: &'a str,
    arg3: &'a str,
}

struct EncodedInstruction {
    msb: u8,
    lsb: u8,
}

impl Instruction<'_> {
    fn new() -> Instruction<'static> {
        Instruction {
            op: "",
            arg1: "",
            arg2: "",
            arg3: "",
        }
    }
}

impl EncodedInstruction {
    fn new(msb: u8, lsb: u8) -> EncodedInstruction {
        EncodedInstruction { msb: msb, lsb: lsb }
    }
}

fn main() -> std::io::Result<()> {
    let args: Vec<String> = env::args().collect();
    let filename: &str;
    let outputfilename: &str;
    let mut buffer = String::new();
    let mut code: Vec<&str> = vec![];
    match args.len() {
        2 => {
            filename = &args[1];
            outputfilename = "out.ch8"
        }
        3 => {
            filename = &args[1];
            outputfilename = &args[2];
        }
        _ => {
            println!("Usage:\n[PROGRAM] [INPUT FILENAME] [OUTPUT FILENAME (optional)]");
            return Ok(());
        }
    };
    let mut infile = File::open(filename)?;
    let mut outfile = File::create(outputfilename)?;
    infile.read_to_string(&mut buffer)?;
    for line in buffer.lines() {
        code.push(line);
    }
    outfile.write_all(&assemble(code))?;
    Ok(())
}

fn assemble(code: Vec<&str>) -> Vec<u8> {
    let mut compiled: Vec<u8> = vec![];
    let mut instruction: Instruction;
    let mut encoded: EncodedInstruction;
    for line in code {
        instruction = Instruction::new();
        // writes out the instruction to the variable
        for (argc, word) in line.split_whitespace().enumerate() {
            match argc {
                0 => {
                    instruction.op = word;
                },
                1 => {
                    instruction.arg1 = word;
                },
                2 => {
                    instruction.arg2 = word;
                },
                3 => {
                    instruction.arg3 = word;
                },
                _ => {
                    ()
                },
            }
        }
        encoded = encode(instruction);
        println!("{:?}, {:?}", encoded.msb, encoded.lsb);
        // must push the lsb first for each 16-bit block(?)
        compiled.push(encoded.lsb);
        compiled.push(encoded.msb);
    }
    compiled
}

fn encode(instruction: Instruction) -> EncodedInstruction {
    let ret = match instruction.op {
        "CLR" => EncodedInstruction::new(hex("00") as u8, hex("E0") as u8),
        "RET" => EncodedInstruction::new(hex("00") as u8, hex("EE") as u8),
        "JP" => hex_split(0x1000 + hex(instruction.arg1)),
        _ => EncodedInstruction::new(0, 0),
    };

    ret
}

// extracts the 'V', '@', or '%' from the arg 
// (removes first character from string)
fn extract(arg: &str) -> str {

}

// converts hex str to u16 (instead of u8 to account for 12-bit addresses)
fn hex(num: &str) -> u16 {
    u16::from_str_radix(num, 16).unwrap()
}

// splits a u16 into two u8's in an EncodedInstruction
fn hex_split(num: u16) -> EncodedInstruction {
    EncodedInstruction::new((num / 256) as u8, (num % 256) as u8)
}
