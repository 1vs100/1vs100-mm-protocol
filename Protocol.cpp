#include "Protocol.h"

void MmWriter_Init(MmWriter *w, uint8_t *buf, uint32_t cap)
{
	w->start = buf;
	w->p = buf;
	w->end = buf + cap;
	w->overflow = 0;
}

uint32_t MmWriter_Length(const MmWriter *w)
{
	return (uint32_t)(w->p - w->start);
}

void MmPutBytes(MmWriter *w, const uint8_t *data, uint32_t len)
{
	uint32_t i = 0;
	if (w->overflow || (uint32_t)(w->end - w->p) < len) {
		w->overflow = 1;
		return;
	}
	while (i < len) {
		w->p[i] = data[i];
		i++;
	}
	w->p += len;
}

void MmPutU8(MmWriter *w, uint8_t v)
{
	MmPutBytes(w, &v, 1);
}

void MmPutU16(MmWriter *w, uint32_t v)
{
	uint8_t b[2];
	b[0] = (uint8_t)(v >> 8);
	b[1] = (uint8_t)v;
	MmPutBytes(w, b, 2);
}

void MmPutU32(MmWriter *w, uint32_t v)
{
	uint8_t b[4];
	b[0] = (uint8_t)(v >> 24);
	b[1] = (uint8_t)(v >> 16);
	b[2] = (uint8_t)(v >> 8);
	b[3] = (uint8_t)v;
	MmPutBytes(w, b, 4);
}

void MmPutU64(MmWriter *w, uint64_t v)
{
	MmPutU32(w, (uint32_t)(v >> 32));
	MmPutU32(w, (uint32_t)v);
}

void MmPutStr(MmWriter *w, const char *s)
{
	uint32_t len = 0;
	while (len < 0xFFFF && s[len])
		len++;
	MmPutU16(w, len);
	MmPutBytes(w, (const uint8_t *)s, len);
}

void MmEncodeJoin(MmWriter *w, const MatchmakingLocalPlayer *players, uint32_t count)
{
	uint32_t i = 0;
	MmPutU8(w, (uint8_t)count);
	while (i < count) {
		MmPutU64(w, players[i].xuid);
		MmPutStr(w, players[i].gamertag);
		i++;
	}
}

void MmEncodeXuid(MmWriter *w, uint64_t xuid)
{
	MmPutU64(w, xuid);
}

void MmEncodeAvatarPut(MmWriter *w, uint64_t xuid, const uint8_t *blob)
{
	MmPutU64(w, xuid);
	MmPutU16(w, AVATAR_METADATA_BYTES);
	MmPutBytes(w, blob, AVATAR_METADATA_BYTES);
}

void MmEncodeAvatarGet(MmWriter *w, uint64_t peer)
{
	MmPutU64(w, peer);
}

void MmEncodeRelayPut(MmWriter *w, uint64_t xuid, const uint64_t *targets, uint32_t targetCount,
	const uint8_t *data, uint32_t len)
{
	uint32_t i = 0;
	MmPutU64(w, xuid);
	MmPutU16(w, targetCount);
	while (i < targetCount) {
		MmPutU64(w, targets[i]);
		i++;
	}
	MmPutU16(w, len);
	MmPutBytes(w, data, len);
}

uint32_t MmFrame(uint8_t *out, uint32_t cap, uint8_t type, const uint8_t *payload, uint32_t len)
{
	MmWriter w;
	MmWriter_Init(&w, out, cap);
	MmPutU8(&w, MM_PROTOCOL_VERSION);
	MmPutU8(&w, type);
	MmPutU32(&w, len);
	MmPutBytes(&w, payload, len);
	return w.overflow ? 0 : MmWriter_Length(&w);
}

void MmParseHeader(const uint8_t *header, uint8_t *version, uint8_t *type, uint32_t *payloadLen)
{
	*version = header[0];
	*type = header[1];
	*payloadLen = MmGetU32(header + 2);
}

uint16_t MmGetU16(const uint8_t *p)
{
	return (uint16_t)(((uint32_t)p[0] << 8) | (uint32_t)p[1]);
}

uint32_t MmGetU32(const uint8_t *p)
{
	return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

uint64_t MmGetU64(const uint8_t *p)
{
	return ((uint64_t)MmGetU32(p) << 32) | MmGetU32(p + 4);
}

/* Single-byte chars, at most 31 kept. */
static void CopyGamertag(MmChar16 *out, const uint8_t *in, uint32_t len)
{
	uint32_t i = 0;
	if (len >= 32)
		len = 31;
	while (i < len) {
		out[i] = (MmChar16)in[i];
		i++;
	}
	out[len] = 0;
}

int MmDecodeRoster(const uint8_t *p, uint32_t n, uint64_t self, MatchmakingPeer *out, uint32_t cap,
	uint32_t *count, uint32_t *groupId)
{
	uint32_t pos = 2, kept = 0;
	uint32_t consoles;
	if (n < 2)
		return 0;
	consoles = MmGetU16(p);
	while (consoles != 0) {
		uint32_t players, consoleStart = kept;
		int isSelf = 0;
		consoles--;
		if (pos + 1 > n)
			return 0;
		players = p[pos];
		pos++;
		if (players == 0 || players > MATCHMAKING_MAX_LOCAL_PLAYERS)
			return 0;
		/* Kept while there is room; this console taken back out once read. */
		while (players != 0) {
			uint64_t xuid;
			uint32_t nameLen;
			players--;
			if (pos + 10 > n)
				return 0;
			xuid = MmGetU64(p + pos);
			nameLen = MmGetU16(p + pos + 8);
			pos += 10;
			if (pos + nameLen > n)
				return 0;
			if (xuid != 0 && xuid == self)
				isSelf = 1;
			if (kept < cap) {
				out[kept].xuid = xuid;
				CopyGamertag(out[kept].gamertag, p + pos, nameLen);
				kept++;
			}
			pos += nameLen;
		}
		if (isSelf)
			kept = consoleStart;
	}
	if (pos + 4 > n)
		return 0;
	*groupId = MmGetU32(p + pos);
	*count = kept;
	return 1;
}

int MmDecodeJoinAck(const uint8_t *p, uint32_t n, uint32_t *population, uint32_t *groupId)
{
	if (n < 8)
		return 0;
	*population = MmGetU32(p);
	*groupId = MmGetU32(p + 4);
	return 1;
}

uint32_t MmDecodePopulation(const uint8_t *p, uint32_t n)
{
	return n >= 4 ? MmGetU32(p) : 0;
}

int MmDecodeAvatarReply(const uint8_t *p, uint32_t n, const uint8_t **blob)
{
	if (n < 3 + AVATAR_METADATA_BYTES || p[0] != 1 || MmGetU16(p + 1) != AVATAR_METADATA_BYTES)
		return 0;
	*blob = p + 3;
	return 1;
}

uint32_t MmDecodeRelayReply(const uint8_t *p, uint32_t n, MmRelayVisitor visit, void *ctx)
{
	uint32_t pos = 2, visited = 0, count;
	if (n < 2)
		return 0;
	count = MmGetU16(p);
	while (count != 0) {
		uint64_t from;
		uint32_t len;
		count--;
		if (pos + 10 > n)
			break;
		from = MmGetU64(p + pos);
		len = MmGetU16(p + pos + 8);
		pos += 10;
		if (pos + len > n || len > MATCHMAKING_RELAY_MAX_BYTES)
			break;
		visit(ctx, from, p + pos, len);
		visited++;
		pos += len;
	}
	return visited;
}
