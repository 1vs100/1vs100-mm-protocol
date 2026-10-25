#pragma once
#include "MatchmakingTypes.h"

/* Wire protocol as pure functions over byte buffers. Big-endian throughout. */

/* Every frame: version u8 | type u8 | length u32 | payload. Another version is not read further. */
#define MM_PROTOCOL_VERSION 2
#define MM_HEADER_SIZE 6

#define MM_TYPE_JOIN 0x01
#define MM_TYPE_JOIN_ACK 0x02
#define MM_TYPE_HEARTBEAT 0x03
#define MM_TYPE_HEARTBEAT_ACK 0x04
#define MM_TYPE_LEAVE 0x05
#define MM_TYPE_ROSTER 0x06
#define MM_TYPE_ROSTER_REPLY 0x07
#define MM_TYPE_ANNOUNCE 0x08
#define MM_TYPE_AVATAR_PUT 0x09
#define MM_TYPE_AVATAR_ACK 0x0A
#define MM_TYPE_AVATAR_GET 0x0B
#define MM_TYPE_AVATAR_REPLY 0x0C
#define MM_TYPE_RELAY_PUT 0x0D
#define MM_TYPE_RELAY_ACK 0x0E
#define MM_TYPE_RELAY_POLL 0x0F
#define MM_TYPE_RELAY_REPLY 0x10
#define MM_TYPE_REJECT 0x11

/* Server closes after REJECT. */
#define MM_REJECT_VERSION 1
#define MM_REJECT_MALFORMED_JOIN 2
#define MM_REJECT_UNKNOWN_TYPE 3

/* Puts bounds-check; past the end sets overflow. Caller checks once at the end. */
struct MmWriter {
	uint8_t *start;
	uint8_t *p;
	uint8_t *end;
	int overflow;
};
void MmWriter_Init(MmWriter *w, uint8_t *buf, uint32_t cap);
uint32_t MmWriter_Length(const MmWriter *w);
void MmPutU8(MmWriter *w, uint8_t v);
void MmPutU16(MmWriter *w, uint32_t v);
void MmPutU32(MmWriter *w, uint32_t v);
void MmPutU64(MmWriter *w, uint64_t v);
void MmPutBytes(MmWriter *w, const uint8_t *data, uint32_t len);
/* u16 length, no terminator. */
void MmPutStr(MmWriter *w, const char *s);

/* No request names a session; the server keeps it with the connection. */
/* Players, user 0 first; the first XUID identifies the console. */
void MmEncodeJoin(MmWriter *w, const MatchmakingLocalPlayer *players, uint32_t count);
/* First XUID alone. */
void MmEncodeXuid(MmWriter *w, uint64_t xuid);
void MmEncodeAvatarPut(MmWriter *w, uint64_t xuid, const uint8_t *blob);
void MmEncodeAvatarGet(MmWriter *w, uint64_t peer);
void MmEncodeRelayPut(MmWriter *w, uint64_t xuid, const uint64_t *targets, uint32_t targetCount,
	const uint8_t *data, uint32_t len);

/* 0 when it does not fit. */
uint32_t MmFrame(uint8_t *out, uint32_t cap, uint8_t type, const uint8_t *payload, uint32_t len);
/* Header fields. */
void MmParseHeader(const uint8_t *header, uint8_t *version, uint8_t *type, uint32_t *payloadLen);

uint16_t MmGetU16(const uint8_t *p);
uint32_t MmGetU32(const uint8_t *p);
uint64_t MmGetU64(const uint8_t *p);

/* Group consoles with players, then group id. Self console skipped; at most cap. 0 when truncated or a console lists 0 or 5+ players. */
int MmDecodeRoster(const uint8_t *p, uint32_t n, uint64_t self, MatchmakingPeer *out, uint32_t cap,
	uint32_t *count, uint32_t *groupId);
/* Population and group id; 0 when short. */
int MmDecodeJoinAck(const uint8_t *p, uint32_t n, uint32_t *population, uint32_t *groupId);
/* Population, 0 when absent. */
uint32_t MmDecodePopulation(const uint8_t *p, uint32_t n);
/* 1 with *blob on a found, complete reply. */
int MmDecodeAvatarReply(const uint8_t *p, uint32_t n, const uint8_t **blob);
/* Visits each item in order; a bad item ends the reply. */
typedef void (*MmRelayVisitor)(void *ctx, uint64_t from, const uint8_t *data, uint32_t len);
uint32_t MmDecodeRelayReply(const uint8_t *p, uint32_t n, MmRelayVisitor visit, void *ctx);
