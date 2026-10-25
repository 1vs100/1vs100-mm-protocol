#pragma once
#include <stddef.h>
#include <stdint.h>

/* Types and limits of the gameserver matchmaking protocol, shared by every client. */

/* 16-bit text unit: wchar_t where it is 16 bits wide, so callers pass it to the platform's wide APIs. */
#if defined(_XBOX) || defined(_WIN32)
typedef wchar_t MmChar16;
#else
typedef uint16_t MmChar16;
#endif

/* A group holds four players; this console at least one. */
#define MATCHMAKING_MAX_REMOTE_PEERS 3
/* JOIN players, user 0 first. */
#define MATCHMAKING_MAX_LOCAL_PLAYERS 4

struct MatchmakingPeer {
	uint64_t xuid;
	MmChar16 gamertag[32];
};

struct MatchmakingLocalPlayer {
	uint64_t xuid;
	char gamertag[32]; /* at most 31 chars */
	uint32_t user;     /* XAM index 0-3, not sent */
};

/* Relayed so consoles build each other avatars without a QNet data plane. */
#define AVATAR_METADATA_BYTES 1000

/* Relayed title events. */
#define MATCHMAKING_RELAY_MAX_BYTES 8192
/* Empty list means everyone. */
#define MATCHMAKING_RELAY_MAX_TARGETS 4
