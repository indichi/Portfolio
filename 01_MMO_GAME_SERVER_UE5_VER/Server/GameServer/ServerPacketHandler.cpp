#include "pch.h"
#include "ServerPacketHandler.h"
#include "BufferReader.h"
#include "BufferWriter.h"
#include "Protocol.pb.h"
#include "Room.h"
#include "ObjectUtils.h"
#include "Player.h"
#include "GameSession.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	// TODO : Log
	return false;
}

bool Handle_C_TRY_LOGIN(PacketSessionRef& session, Protocol::C_TRY_LOGIN& pkt)
{
	Protocol::S_TRY_LOGIN TryLoginPkt;

	// 계정 정보는 하드 코딩으로 대체..
	if (pkt.id() == "account" && pkt.pw() == "1234")
		TryLoginPkt.set_success(true);
	else
		TryLoginPkt.set_success(false);

	SEND_PACKET(TryLoginPkt);

	return true;
}

bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	// 상용적인 로직은 웹에서 인증 후 토큰 발급 받아 확인 후 게임서버에 접속 시키는 것이지만 생략..
	Protocol::S_LOGIN loginPkt;

	Protocol::ObjectInfo* player = loginPkt.add_players();
	Protocol::PosInfo* posInfo = player->mutable_pos_info();
	posInfo->set_x(Utils::GetRandom(0.f, 100.f));
	posInfo->set_y(Utils::GetRandom(0.f, 100.f));
	posInfo->set_z(Utils::GetRandom(0.f, 100.f));
	posInfo->set_yaw(Utils::GetRandom(0.f, 45.f));

	for (int32 i = 0; i < 3; i++)
	{
		
	}

	loginPkt.set_success(true);

	SEND_PACKET(loginPkt);

	return true;
}

bool Handle_C_ENTER_GAME(PacketSessionRef& session, Protocol::C_ENTER_GAME& pkt)
{
	// 플레이어 생성
	PlayerRef player = ObjectUtils::CreatePlayer(static_pointer_cast<GameSession>(session));

	// 방에 입장
	GRoom->DoAsync(&Room::HandleEnterPlayer, player);
	//GRoom->HandleEnterPlayerLocked(player);

	return true;
}

bool Handle_C_LEAVE_GAME(PacketSessionRef& session, Protocol::C_LEAVE_GAME& pkt)
{
	auto gameSession = static_pointer_cast<GameSession>(session);

	PlayerRef player = gameSession->player.load();
	if (player == nullptr)
		return false;

	RoomRef room = player->room.load().lock();
	if (room == nullptr)
		return false;

	room->HandleLeavePlayer(player);

	return true;
}

bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
	auto gameSession = static_pointer_cast<GameSession>(session);

	PlayerRef player = gameSession->player.load();
	if (player == nullptr)
		return false;

	RoomRef room = player->room.load().lock();
	if (room == nullptr)
		return false;

	room->DoAsync(&Room::HandleMove, pkt);
	//room->HandleMove(pkt);

	return true;
}

bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt)
{
	return true;
}
