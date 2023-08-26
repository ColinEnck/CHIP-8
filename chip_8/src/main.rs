use std::env;
use std::fs::File;
use std::io::{self, Read};
use pronto_graphics::{Window, Key};
use std::time::Instant;
extern crate ears;
use ears::{Sound, AudioController};
use rand::random;

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
    
    let font = [
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    ];

    for i in 0x50..0x9F {
        mem[i] = font[i-0x50];
    }

    // setting up the rest
    let mut stack: Vec<u16> = vec![];
    let mut pc: u16 = 512; 
    let mut index: u16 = 0;
    let mut delay: f64 = 0.0; // these had to be f64 to hold the decimal data of the delay timer
    let mut sound: f64 = 0.0;
    let mut registers: [u8; 16] = [0; 16];
    let mut instruction: String;
    let mut screen: [[bool; 64]; 32] = [[false; 64]; 32];
    let mut keyboard: [bool; 16] = [false; 16];

    // sets up the grapics window
    let mut window = Window::new(64 * 20, 32 * 20, "CHIP-8!");

    // audio file
    let mut beep = Sound::new("src/beep.ogg").unwrap();

    // event loop
    loop {
        // to measure the time each loop takes
        let start_time = Instant::now();

        // instruction fetch, decode, and execute
        instruction = fetch(&mem, &mut pc);

        // keyboard logic
        if window.key_pressed(Key::ESCAPE) {
            return Ok(());
        }
        let keys = [
            Key::NUM0,
            Key::NUM1,
            Key::NUM2,
            Key::NUM3,
            Key::NUM4,
            Key::NUM5,
            Key::NUM6,
            Key::NUM7,
            Key::NUM8,
            Key::NUM9,
            Key::A,
            Key::B,
            Key::C,
            Key::D,
            Key::E,
            Key::F
        ];
        for i in 0..keys.len() {
            if window.key_pressed(keys[i]) {
                keyboard[i] = true;
            }
        }

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
                pc = hex(&nnn) as u16;
            },
            '2' => {
                stack.push(pc);
                pc = hex(&nnn) as u16;
            },
            '3' => {
                if registers[hex(&x.to_string())] == hex(&nn) as u8 {
                    pc += 2;
                }
            },
            '4' => {
                if registers[hex(&x.to_string())] != hex(&nn) as u8 {
                    pc += 2;
                }
            },
            '5' => {
                if registers[hex(&x.to_string())] == registers[hex(&y.to_string())] {
                    pc += 2;
                }
            },
            '6' => {
                registers[hex(&x.to_string())] = hex(&nn) as u8;
            },
            '7' => {
                registers[hex(&x.to_string())] = registers[hex(&x.to_string())].wrapping_add(hex(&nn) as u8);
            },
            '8' => {
                match n {
                    '0' => {
                        registers[hex(&x.to_string())] = registers[hex(&y.to_string())];
                    },
                    '1' => {
                        registers[hex(&x.to_string())] |= registers[hex(&y.to_string())];
                    },
                    '2' => {
                        registers[hex(&x.to_string())] &= registers[hex(&y.to_string())];
                    },
                    '3' => {
                        registers[hex(&x.to_string())] ^= registers[hex(&y.to_string())];
                    },
                    '4' => {
                        let (sum, overflow) = registers[hex(&x.to_string())].overflowing_add(registers[hex(&y.to_string())]);
                        if overflow {
                            registers[0xF] = 1;
                            registers[hex(&x.to_string())] = sum;
                        } else {
                            registers[0xF] = 0;
                            registers[hex(&x.to_string())] = sum;
                        }
                    },
                    '5' => {
                        if registers[hex(&x.to_string())] > registers[hex(&y.to_string())] {
                            registers[0xF] = 1;
                        }
                        registers[hex(&x.to_string())] = registers[hex(&x.to_string())].wrapping_sub(registers[hex(&y.to_string())]);
                    },
                    '6' => {
                        let og_val = registers[hex(&x.to_string())];
                        registers[hex(&x.to_string())] >>= 1;
                        registers[0xF] = og_val & 1;
                    },
                    '7' => {
                        if registers[hex(&y.to_string())] > registers[hex(&x.to_string())] {
                            registers[0xF] = 1;
                        }
                        registers[hex(&x.to_string())] = registers[hex(&y.to_string())].wrapping_sub(registers[hex(&x.to_string())]);

                    },
                    'E' => {
                        let og_val = registers[hex(&x.to_string())];
                        registers[hex(&x.to_string())] <<= 1;
                        registers[0xF] = (og_val >> (std::mem::size_of_val(&og_val) * 8 - 1)) & 1;
                    }
                    _ => ()
                }
            },
            '9' => {
                if registers[hex(&x.to_string())] != registers[hex(&y.to_string())] {
                    pc += 2;
                }
            },
            'A' => {
                index = hex(&nnn) as u16;
            },
            'B' => {
                pc = registers[hex(&x.to_string())] as u16 + hex(&nn) as u16;
            },
            'C' => {
                registers[hex(&x.to_string())] = random::<u8>() & hex(&nn) as u8;
            },
            'D' => {
                let xcords = registers[hex(&x.to_string()) % 64];
                let ycords = registers[hex(&y.to_string()) % 32];
                registers[0xF] = 0;
                for i in 0..hex(&n.to_string()) {
                    if ycords + (i as u8) >= 32 { break; }
                    let sprite = mem[index as usize + i];
                    let sprite_str = format!("{:08b}", sprite);
                    for (count, bit) in sprite_str.chars().enumerate() {
                        if xcords + (count as u8) >= 64 { break; }
                        if screen[ycords as usize + i][xcords as usize + count] {
                            registers[0xF] = 1;
                            screen[ycords as usize + i][xcords as usize + count] = false;
                        } else if bit == '1' {
                            screen[ycords as usize + i][xcords as usize + count] = true;
                        }
                    }
                }
            },
            'E' => {
                if nn == "9E" {
                    if keyboard[registers[hex(&x.to_string())] as usize] {
                        pc += 2;
                    }
                }
                if nn == "A1" {
                    if !keyboard[registers[hex(&x.to_string())] as usize] {
                        pc += 2;
                    }
                }
            },
            'F' => {
                match y {
                    '0' => {
                        match n {
                            '7' => {
                                registers[hex(&x.to_string())] = delay as u8;
                            },
                            'A' => {
                                let mut key_on = false;
                                for key in keyboard {
                                    if key {
                                        key_on = true;
                                        break;
                                    }
                                };
                                if !key_on {
                                    pc -= 2;
                                }
                            },
                            _ => ()
                        }
                    },
                    '1' => {
                        match n {
                            '5' => {
                                delay = registers[hex(&x.to_string())].into();
                            },
                            '8' => {
                                sound = registers[hex(&x.to_string())].into();
                            },
                            'E' => {
                                index += registers[hex(&x.to_string())] as u16;
                                if index >= 0x1000 {
                                    registers[0xF] = 1;
                                }
                            },
                            _ => ()
                        }
                    },
                    '2' => {
                        index = (registers[hex(&x.to_string())] * 5 + 0x50) as u16;
                    },
                    '3' => {
                        let num = registers[hex(&x.to_string())].to_string();
                        let mut num1 = '\0';
                        let mut num2 = '\0';
                        let mut num3 = '\0';
                        for (index, digit) in num.chars().enumerate() {
                            match index {
                                0 => num1 = digit,
                                1 => num2 = digit,
                                2 => num3 = digit,
                                _ => ()
                            }
                        }
                        mem[index as usize] = hex(&num1.to_string()) as u8;
                        mem[(index+1) as usize] = hex(&num2.to_string()) as u8;
                        mem[(index+2) as usize] = hex(&num3.to_string()) as u8;
                    },
                    '5' => {
                        for i in 0..=hex(&x.to_string()) {
                            mem[(index as usize) + i] = registers[i as usize];
                        }
                    },
                    '6' => {
                        for i in 0..=hex(&x.to_string()) {
                            registers[i as usize] = mem[(index as usize) + i];
                        }
                    },
                    _ => ()
                }
            },
            _ => {
                ()
            }
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

        // to meausre how long the loop took
        let end_time = Instant::now();
        let duration = end_time - start_time;
        let duration_secs = duration.as_secs_f64();
        if delay > 0.0 {
            delay -= 60.0 * duration_secs;
        }
        if sound > 0.0 {
            sound -= 60.0 * duration_secs;
            if !beep.is_playing() {
                beep.play();
            }
        }
    }

}

fn fetch(mem: &[u8; 4096], pc: &mut u16) -> String {
    let ret = String::from(format!("{:02X}{:02X}", mem[*pc as usize], mem[(*pc as usize) + 1]));
    *pc += 2;
    ret
}

// converts a hex number in string to a num
fn hex(num: &str) -> usize {
    usize::from_str_radix(num, 16).unwrap()
}
