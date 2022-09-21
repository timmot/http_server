import asyncio
import logging

async def handle(r: asyncio.StreamReader, w: asyncio.StreamWriter):
    while not r.at_eof():
        line = await r.readexactly(19)
        print(line)

async def main():
    logging.basicConfig(level=logging.DEBUG)
    [_, writer] = await asyncio.open_connection("localhost", 10000)
    writer.write(b"""abcd\ndefg\nghij\njklm""")
    await asyncio.sleep(1)
    writer.write(b"""dcba\ngfed\njihg""")
    writer.write_eof()

if __name__ == "__main__":
    asyncio.run(main())