#include "MMORPGdbClientObject.h"
#include "Log/MMORPGGateServerLog.h"
#include "Protocol/HallProtocol.h"
#include "ServerList.h"
#include "MMORPGType.h"

void UMMORPGdbClientObject::Init()
{
	Super::Init();
}

void UMMORPGdbClientObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void UMMORPGdbClientObject::Close()
{
	Super::Close();
}

void UMMORPGdbClientObject::RecvProtocol(uint32 InProtocol)
{
	Super::RecvProtocol(InProtocol);

	switch (InProtocol)
	{
	case SP_CharacterAppearanceResponses:
	{
		//收到dbServer返回的角色信息回调
		FString String;//dbServer发送回来的角色Json数据
		FSimpleAddrInfo AddrInfo;
		//接收角色数据
		SIMPLE_PROTOCOLS_RECEIVE(SP_CharacterAppearanceResponses, AddrInfo, String);

		//向本地服务器转发协议,
		//SIMPLE_SERVER_SEND：客户端受到消息后，由服务器发送到另外一端
		//SIMPLE_CLIENT_SEND: 服务器通过客户端发送到对方的服务器
		SIMPLE_SERVER_SEND(GateServer, SP_CharacterAppearanceResponses, AddrInfo, String);

		UE_LOG(LogMMORPGGateServer, Display, TEXT("[SP_CharacterAppearanceResponses]"));
		break;
	}
	case SP_CheckCharacterNameResponses:
	{
		//收到dbServer返回的检查角色名字回调
		ECheckNameType CheckNameType = ECheckNameType::UNKNOWN_ERROR;
		FSimpleAddrInfo AddrInfo;
		SIMPLE_PROTOCOLS_RECEIVE(SP_CheckCharacterNameResponses, CheckNameType, AddrInfo);

		//向本地服务器转发协议
		SIMPLE_SERVER_SEND(GateServer, SP_CheckCharacterNameResponses, AddrInfo, CheckNameType);

		UE_LOG(LogMMORPGGateServer, Display, TEXT("[SP_CheckCharacterNameResponses]"));
		break;
	}
	case SP_CreateCharacterResponses:
	{
		//收到dbServer返回的创建角色回调
		FSimpleAddrInfo AddrInfo;
		ECheckNameType CheckNameType = ECheckNameType::UNKNOWN_ERROR;
		bool bCreateCharacter = false;//是否创角成功
		FString JsonString;
		//接收角色数据
		SIMPLE_PROTOCOLS_RECEIVE(SP_CreateCharacterResponses, CheckNameType, bCreateCharacter, JsonString, AddrInfo);

		//向本地服务器转发协议
		SIMPLE_SERVER_SEND(GateServer, SP_CreateCharacterResponses, AddrInfo, CheckNameType, bCreateCharacter, JsonString);

		UE_LOG(LogMMORPGGateServer, Display, TEXT("[SP_CreateCharacterResponses]"));
		break;
	}
	case SP_DeleteCharacterResponses:
	{
		//收到dbServer返回的删除角色回调
		FSimpleAddrInfo AddrInfo;
		int32 InUserID = INDEX_NONE;
		int32 SlotID = INDEX_NONE;
		int32 SuccessDeleteCount = 0;
		//接收角色数据
		SIMPLE_PROTOCOLS_RECEIVE(SP_DeleteCharacterResponses, InUserID, SlotID, SuccessDeleteCount, AddrInfo);

		//向本地服务器转发协议
		SIMPLE_SERVER_SEND(GateServer, SP_DeleteCharacterResponses, AddrInfo, InUserID, SlotID, SuccessDeleteCount);

		UE_LOG(LogMMORPGGateServer, Display, TEXT("[SP_DeleteCharacterResponses]"));
		break;
	}
	}
}

