# 1 vs 100 matchmaking protocol

- Encodes and decodes the messages a 1 vs 100 client exchanges with the [gameserver](https://github.com/1vs100/gameserver) matchmaking service.
- Used by [1vs100-reborn-x360](https://github.com/1vs100/1vs100-reborn-x360) on the console and by [1vs100-PC](https://github.com/1vs100/1vs100-PC), so both speak the same wire format.

## Usage

Add this repository as a submodule and compile `Protocol.cpp` into your client. Include `Protocol.h`.

The code is plain C++98 on `<stdint.h>` types, because the Xbox 360 XDK compiler builds it too. It does no I/O. Your client owns the socket and passes byte buffers in and out.

```
MmWriter w;
MmWriter_Init(&w, payload, sizeof(payload));
MmEncodeJoin(&w, players, playerCount);
uint32_t frameLen = MmFrame(frame, sizeof(frame), MM_TYPE_JOIN, payload, MmWriter_Length(&w));
```

## Wire format

Every frame is `version u8 | type u8 | length u32 | payload`, big-endian. The message types are listed in `Protocol.h`. The server is the reference implementation.

## Tests

```
cmake -S . -B build && cmake --build build && ctest --test-dir build
```
