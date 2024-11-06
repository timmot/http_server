use rust_http::ThreadPool;
use std::{
    fs,
    io::{prelude::*, BufReader},
    net::{TcpListener, TcpStream},
    process::ExitCode,
    thread,
    time::{Duration, Instant, SystemTime},
};

fn main() -> ExitCode {
    if std::env::args().len() > 3 {
        eprintln!(
            "Usage: {} [port] [host]",
            std::env::args().next().unwrap_or("http".to_owned())
        );
        return ExitCode::FAILURE;
    }

    let mut args = std::env::args();
    args.next();
    let port = args
        .next()
        .map(|port: String| port.parse::<u16>())
        .map_or(8000, |port| port.expect("Did not provide number for port"));
    let host = args.next().unwrap_or("localhost".to_owned());

    let listener = TcpListener::bind((host, port)).expect("Could not bind to addr");
    println!("Listening on {}", listener.local_addr().unwrap());

    // TODO: Replace threadpool with crate
    // TODO: Use async instead of multithreading
    let pool = ThreadPool::new(4);

    for stream in listener.incoming() {
        match stream {
            Ok(s) => {
                pool.execute(|| {
                    handle_connection(s);
                });
            }
            Err(e) => panic!("encountered IO error: {e}"),
        }
    }

    // TODO: https://doc.rust-lang.org/book/ch20-03-graceful-shutdown-and-cleanup.html
    ExitCode::SUCCESS
}

// TODO: Bubble up errors to here to get rid of ugly error handling logic
fn handle_connection(mut stream: TcpStream) {
    let instant = Instant::now();

    // FIXME: Replace with real http parser
    let buf_reader = BufReader::new(&mut stream);

    let request_line_result = match buf_reader.lines().next() {
        Some(line_result) => line_result,
        None => return,
    };

    let request_line = match request_line_result {
        Ok(request_line) => request_line,
        Err(http_error) => {
            eprintln!("HTTP Parse Error: {}", http_error);
            return;
        }
    };

    let (status_line, filename) = match &request_line[..] {
        "GET / HTTP/1.1" => ("HTTP/1.1 200 OK", "hello.html"),
        "GET /sleep HTTP/1.1" => {
            thread::sleep(Duration::from_secs(5));
            ("HTTP/1.1 200 OK", "hello.html")
        }
        _ => ("HTTP/1.1 404 NOT FOUND", "404.html"),
    };

    let contents = fs::read_to_string(filename).unwrap();
    let length = contents.len();

    let response = format!("{status_line}\r\nContent-Length: {length}\r\n\r\n{contents}");

    // TODO: Parse the request line first
    eprintln!(
        "{:?} {}. Took {}ns",
        SystemTime::now(),
        request_line,
        instant.elapsed().as_nanos()
    );

    stream.write_all(response.as_bytes()).unwrap();
}
