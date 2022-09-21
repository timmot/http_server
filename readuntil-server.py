import asyncio
import logging

async def handle(r: asyncio.StreamReader, w: asyncio.StreamWriter):
    try:
        await r.readuntil(b'\r\n\r\n')
    except:
        pass
    w.write(b"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 17\r\nConnection: close\r\n\r\npee pee poo poo\r\n")
    w.write_eof()
    w.close()

async def main():
    logging.basicConfig(level=logging.DEBUG)
    server = await asyncio.start_server(handle, "0.0.0.0", 10000)
    logging.info("Server Ready.")
    async with server:
        await server.serve_forever()

if __name__ == "__main__":
    asyncio.run(main())