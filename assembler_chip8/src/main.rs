use std::env;
use std::fs::File;
use std::io::prelude::*;

fn main() -> std::io::Result<()> {
    let args: Vec<String> = env::args().collect();
    let filename: &str;
    let outputfilename: &str;
    let mut code = String::new();
    let mut compiled: Vec<u8> = vec![]; // will hold the compiled code
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
    infile.read_to_string(&mut code)?;
    let mut outfile = File::create(outputfilename)?;

    outfile.write_all(&compiled)?;
    Ok(())
}
