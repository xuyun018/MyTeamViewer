//---------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------
#ifndef XYTOMATO_SERVER_H
#define XYTOMATO_SERVER_H
//---------------------------------------------------------------------------
#include "../../xyannounce.h"
#include "../../tomato_coder.h"

#include <map>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
#define TOMATO_STATE_CONNECTED												1

#define TOMATO_STAGE_CONNECTED												1
//---------------------------------------------------------------------------
struct tomato_connection
{
	struct tomato_connection *opposite;

	tomato_uid uid;

	unsigned char address[20];

	struct xypagebuffer pb[1];

	SOCKET fd;

	unsigned short remain;

	unsigned char state;
	unsigned char stage;
};

struct tomato_session
{
	struct tomato_connection *connection;
};

struct tomato_server
{
	struct tomato_exports pes[1];

	map<tomato_uid, struct tomato_connection *> *connections;

	//  自增
	tomato_uid uid;

	CRITICAL_SECTION pcs_connection[1];
};
//---------------------------------------------------------------------------
int tomato_server_working(int argc, char *argv[]);
//---------------------------------------------------------------------------
#endif