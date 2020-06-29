#include "../../xyannounce.h"

#include "tomato_server.h"

int main(int argc, char *argv[])
{
	WSADATA wsad;

	WSAStartup(MAKEWORD(2, 2), &wsad);

	tomato_server_working(argc, argv);

	WSACleanup();

	return(0);
}