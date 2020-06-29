#include "tomato_server.h"
//---------------------------------------------------------------------------
void tomato_connection_intialize(struct tomato_connection *connection)
{
	connection->opposite = NULL;

	xypagebuffer_initialize(connection->pb, NULL, 0, 4096);

	connection->uid = 0;

	connection->fd = -1;

	connection->remain = 0;

	connection->stage = 0;
	connection->state = 0;
}
void tomato_connection_unintialize(struct tomato_exports *pes, struct tomato_connection *connection)
{
	xypagebuffer_uninitialize(pes, connection->pb);
}

int CALLBACK SocketProcedure(LPVOID parameter, LPVOID **pointer, LPVOID context, SOCKET s, BYTE type, BYTE number, SOCKADDR *psa, int *salength, const char *buffer, int length)
{
	PXYSOCKET ps = (PXYSOCKET)parameter;
	PXYSOCKET_CONTEXT psc = (PXYSOCKET_CONTEXT)context;
	struct tomato_server *pserver = (struct tomato_server *)ps->parameter0;
	struct tomato_exports *pes = pserver->pes;
	struct tomato_connection *connection;
	struct tomato_connection *opposite;
	struct xypagebuffer *pb;
	unsigned char *p;
	map<tomato_uid, struct tomato_connection *> *connections;
	map<tomato_uid, struct tomato_connection *>::iterator it;
	tomato_uid uid;
	int result = 0;
	unsigned int l0;
	unsigned int l1;
	unsigned int l;
	unsigned int bufferlength;
	unsigned char address[20];
	PSOCKADDR_IN psai;
	unsigned char command;

	switch (number)
	{
	case XYSOCKET_CLOSE:
		switch (type)
		{
		case XYSOCKET_TYPE_TCP:
			break;
		case XYSOCKET_TYPE_TCP0:
			break;
		case XYSOCKET_TYPE_TCP1:
			printf("Server socket close %d\r\n", s);
			connection = (struct tomato_connection *)psc->context;
			if (connection)
			{
				if (connection->uid)
				{
					opposite = connection->opposite;

					connections = pserver->connections;
					EnterCriticalSection(pserver->pcs_connection);
					it = connections->find(connection->uid);
					if (it != connections->end())
					{
						connections->erase(it);
					}
					LeaveCriticalSection(pserver->pcs_connection);

					if (opposite)
					{
						opposite->opposite = NULL;

						//
					}
				}

				tomato_connection_unintialize(pes, connection);

				FREE(connection);

				psc->context = NULL;
			}
			break;
		default:
			break;
		}
		break;
	case XYSOCKET_CONNECT:
		switch (type)
		{
		case XYSOCKET_TYPE_TCP0:
			switch (length)
			{
			case 0:
				// 成功
				break;
			case XYSOCKET_ERROR_FAILED:
			case XYSOCKET_ERROR_REFUSED:
			case XYSOCKET_ERROR_OVERFLOW:
			default:
				break;
			}
			break;
		case XYSOCKET_TYPE_TCP1:
			switch (length)
			{
			case XYSOCKET_ERROR_ACCEPT:
				psai = (PSOCKADDR_IN)psa;

				psai->sin_family = AF_INET;

				*salength = sizeof(SOCKADDR_IN);
				break;
			case XYSOCKET_ERROR_ACCEPTED:
				{
					switch (psa->sa_family)
					{
					case AF_INET:
						ipv4_2_buffer((const SOCKADDR_IN *)psa, address);
						break;
					case AF_INET6:
						break;
					default:
						break;
					}

					connection = (struct tomato_connection *)MALLOC(sizeof(struct tomato_connection));
					if (connection)
					{
						tomato_connection_intialize(connection);

						connection->fd = s;

						memcpy(connection->address, address, sizeof(connection->address));

						psc->context = connection;

						printf("Server accept ok %d\r\n", s);
					}
				}
				break;
			case XYSOCKET_ERROR_OVERFLOW:
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;
	case XYSOCKET_RECV:
		switch (type)
		{
		case XYSOCKET_TYPE_TCP0:
			// 这里是 client 的 socket
			break;
		case XYSOCKET_TYPE_TCP1:
			// 这里是 server 的 socket
			if (pointer == NULL)
			{
				//OutputDebugString(_T("server recv buffer\r\n"));
				//_tprintf(_T("server recv buffer %d\r\n"), length);

				connection = (struct tomato_connection *)psc->context;
				if (connection)
				{
					pb = connection->pb;

					l0 = xypagebuffer_write(pes, pb, (const unsigned char *)buffer, length);
					if (l0)
					{
						while (pb->offset)
						{
							p = decode_buffer(pes, NULL, pb->buffer1, pb->offset,
								&l1, &bufferlength,
								NULL, 0, &command);
							if (p)
							{
								switch (command)
								{
								case TOMATO_SERVER_CONNECTED:
									if (bufferlength == 0)
									{
										pserver->uid++;
										connection->uid = pserver->uid;

										p = address;
										writevalue8(p + 4, connection->uid);
										l = encode_buffer(pes, p, sizeof(tomato_uid), 0, NULL, 0, TOMATO_SERVER_UID, NULL, 0, 0);

										// 发送
										XYTCPSend(s, (const char *)p, l, 5);
									}
									else if (bufferlength == sizeof(tomato_uid))
									{
										connection->uid = readvalue8(p);
									}
									else
									{
										result = 1;
									}

									if (result == 0)
									{
										connections = pserver->connections;
										EnterCriticalSection(pserver->pcs_connection);
										connections->insert(pair<tomato_uid, struct tomato_connection *>(connection->uid, connection));
										LeaveCriticalSection(pserver->pcs_connection);
									}
									break;
								case TOMATO_SERVER_UID:
									// 要连接此 UID
									if (bufferlength == sizeof(tomato_uid))
									{
										uid = readvalue8(p);

										if (uid)
										{
											connections = pserver->connections;
											EnterCriticalSection(pserver->pcs_connection);
											it = connections->find(uid);
											if (it != connections->end())
											{
												connection->opposite = it->second;
												it->second->opposite = connection;
											}
											else
											{
												uid = 0;
											}
											LeaveCriticalSection(pserver->pcs_connection);
										}

										if (uid == 0)
										{
											result = 1;
										}
										else
										{
											p = address;
											//writevalue8(p + 4, connection->uid);
											// sizeof(tomato_uid)
											l = encode_buffer(pes, p, 0, 0, NULL, 0, TOMATO_SERVER_LINK, NULL, 0, 0);

											// 发送
											XYTCPSend(s, (const char *)p, l, 5);
										}
									}
									else
									{
										result = 1;
									}
									break;
								case TOMATO_SERVER_DISCONNECTED:
									break;
								case TOMATO_SERVER_BAD:
									break;
								case TOMATO_SERVER_TRANSMIT:
									// 直接转发
									opposite = connection->opposite;
									if (opposite)
									{
										XYTCPSend(opposite->fd, (const char *)p, bufferlength, 5);
									}
									break;
								case TOMATO_SERVER_LINK:
									break;
								case TOMATO_SERVER_P2P:
									break;
								default:
									result = 1;
									break;
								}

								xypagebuffer_read(pb, NULL, l1);
							}
							else
							{
								break;
							}
						}
					}
				}
			}
			break;
		default:
			break;
		}
		break;
	case XYSOCKET_SEND:
		break;
	case XYSOCKET_TIMEOUT:
		switch (type)
		{
		case XYSOCKET_TYPE_TCP:
			//OutputDebugString(_T("listener timeout\r\n"));
			break;
		case XYSOCKET_TYPE_TCP0:
			break;
		case XYSOCKET_TYPE_TCP1:
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return(result);
}

void tomato_connections_clear(struct tomato_exports *pes, map<tomato_uid, struct tomato_connection *> *connections)
{
	map<tomato_uid, struct tomato_connection *>::iterator it;

	for (it = connections->begin(); it != connections->end(); ++it)
	{
		tomato_connection_unintialize(pes, it->second);

		FREE(it->second);
	}
	connections->clear();
}

void tomato_server_initialize(struct tomato_server *pserver)
{
	struct tomato_exports *pes = pserver->pes;

	tomato_exports_load(pes);

	pserver->connections = new map<tomato_uid, struct tomato_connection *>;

	pserver->uid = 0;

	InitializeCriticalSection(pserver->pcs_connection);
}
void tomato_server_uninitialize(struct tomato_server *pserver)
{
	struct tomato_exports *pes = pserver->pes;

	tomato_connections_clear(pes, pserver->connections);
	delete pserver->connections;

	DeleteCriticalSection(pserver->pcs_connection);
}

int tomato_server_working(int argc, char *argv[])
{
	XYSOCKET ps[1];
	SOCKET s;
	SOCKADDR_IN sai;

	struct tomato_server pserver[1];

	tomato_server_initialize(pserver);

	XYSocketsStartup(ps, pserver, NULL, SocketProcedure);

	XYSocketLaunchThread(ps, XYSOCKET_THREAD_LISTEN, 64);
	XYSocketLaunchThread(ps, XYSOCKET_THREAD_SERVER, 1024);

	sai.sin_family = AF_INET;
	sai.sin_port = htons(1024);
	sai.sin_addr.s_addr = htonl(INADDR_ANY);

	s = XYTCPListen(ps, NULL, (LPVOID)&sai, (const SOCKADDR *)&sai, sizeof(sai));
	if (s != INVALID_SOCKET)
	{
		getchar();
	}

	XYSocketsCleanup(ps);

	tomato_server_uninitialize(pserver);

	return(0);
}
//---------------------------------------------------------------------------