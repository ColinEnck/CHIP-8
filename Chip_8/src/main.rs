use std::env;
use std::fs::File;
use std::io::{self, Read};
use pronto_graphics::{Window, Key};
use std::thread;
use std::time::Duration;

fn main() -> io::Result<()> {
    let args: Vec<String> = env::args().collect();
    if args.len() != 2 {
        println!("Please supply the chip-8 program's filename as the first command line argument");
        return Ok(());
    }
    // reads the bin file
    let mut file = File::open(&args[1])?;
    let mut buffer = Vec::new();
    file.read_to_end(&mut buffer)?;

    // stores the code in mem (starting at 512)
    let mut mem: [u8; 4096] = [0; 4096];
    for i in 512..(512+buffer.len()) {
        mem[i] = buffer[i-512];
    }
    
    // setting up the rest
    let mut stack: Vec<u16>;
    let mut pc: usize = 512; // had to be usize bc of fetch(), indexing in mem only worked with usize
    let mut index: u16 = 0;
    let mut sp: u8 = 0;
    let mut delay: u8 = 0;
    let mut sound: u8 = 0;
    let mut registers: [u8; 16] = [0; 16];
    let mut instruction: String;

    let mut screen: [[bool; 64]; 32] = [[false; 64]; 32];

    // sets up the grapics window
    let mut window = Window::new(64 * 20, 32 * 20, "CHIP-8!");

    // event loop
    loop {
        // instruction fetch, decode, and execute
        instruction = fetch(&mem, &mut pc);
        
        let mut first_char: char = '\0';
        let mut x: char = '\0';
        let mut y: char = '\0';
        let mut n: char = '\0';
        let mut count = 0;
        for i in instruction.chars() {
            match count {
                0 => first_char = i,
                1 => x = i,
                2 => y = i,
                3 => n = i,
                _ => break,
            }
            count += 1;
        }
        let nn = format!("{}{}", y, n);
        let nnn = format!("{}{}", x, nn);

        match first_char {
            '0' => {
                if instruction == "00E0" {
                    screen = [[false; 64]; 32];
                } else if instruction == "00EE" {
                    if stack.len() == 0 { panic!("Stack is size 0 when attempting to pop"); }
                    pc = stack.pop().unwrap();
                }
            },
            '1' => {
                pc = usize::from_str_radix(&nnn, 16).unwrap();
            },
            '2' => {
                ()
            },
            '3' => {
                ()
            },
            '4' => {
                ()
            },
            '5' => {
                ()
            },
            '6' => {
                registers[usize::from_str_radix(&x.to_string(), 16).unwrap()] = u8::from_str_radix(&nn, 16).unwrap();
            },
            '7' => {
                registers[usize::from_str_radix(&x.to_string(), 16).unwrap()] += u8::from_str_radix(&nn, 16).unwrap();
            },
            '8' => {
                ()
            },
            '9' => {
                ()
            },
            'A' => {
                index = u16::from_str_radix(&nnn, 16).unwrap();
            },
            'B' => {
                ()
            },
            'C' => {
                ()
            },
            'D' => {
                let xcords = registers[usize::from_str_radix(&x.to_string(), 16).unwrap() & 63];
                let ycord = registers[usize::from_str_radix(&y.to_string(), 16).unwrap() & 31];
                registers[0xF] = 0;
                for i in 0..usize::from_str_radix(&n.to_string(), 16).unwrap() {
                    let sprite = mem[index as usize + i];
                    let sprite_str = format!("{:08b}", sprite);
                    let mut count = 0;
                    for bit in sprite_str.chars() {
                        if screen[ycord as usize + i][xcords as usize + count] {
                            registers[0xF] = 1;
                            screen[ycord as usize + i][xcords as usize + count] = false;
                        } else if bit == '1' {
                            screen[ycord as usize + i][xcords as usize + count] = true;
                        }
                        count += 1;
                    }
                }
            },
            'E' => {
                ()
            },
            'F' => {
                ()
            },
            _ => {
                ()
            }
        }

        if window.key_pressed(Key::ESCAPE) {
            return Ok(());
        }
        // drawing to the screen
        for i in 0..32 {
            for j in 0..64 {
                if screen[i][j] {
                    window.square(((j*20) as f32, (i*20) as f32), 20 as f32);
                }
            }
        }
        window.update();
    }

}

fn fetch(mem: &[u8; 4096], pc: &mut usize) -> String {
    let ret = String::from(format!("{:02X}{:02X}", mem[*pc], mem[*pc+1]));
    *pc += 2;
    ret
}
