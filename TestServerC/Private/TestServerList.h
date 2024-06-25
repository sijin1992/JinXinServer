//服务器列表，负责管理各个服务器对象ServerObject
#pragma once

#include "CoreMinimal.h"

class FSimpleNetManage;

FSimpleNetManage* LocalServer = NULL;	//本地服务器
FSimpleNetManage* AClient = NULL;		//连接B服务器的客户端
FSimpleNetManage* BClient = NULL;		//连接C服务器的客户端
