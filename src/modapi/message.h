#ifndef MODAPI_H
#define MODAPI_H

class IServer;
class CGameContext;

class CModAPI_Msg
{
private:
	IServer* m_pServer;
	CGameContext* m_pGameServer;
	
public:
	CModAPI_Msg(CGameContext* pGameServer);
	
	IServer* Server();
	CGameContext* GameServer();
};

/* BROADCAST **********************************************************/

enum
{
	MODAPIALT_BROADCAST_NONE = 0,
	MODAPIALT_BROADCAST_CHAT
};

class CModAPI_Msg_Broadcast : public CModAPI_Msg
{
protected:
	int m_Alternative;
	
public:
	CModAPI_Msg_Broadcast(CGameContext* pGameServer, int Alternative);
	void Send(int ClientID, const char* pMessage);
};

#endif
