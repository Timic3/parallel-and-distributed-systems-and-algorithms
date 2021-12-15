/*H********************************************************************************
* Ime datoteke: serverLinux.cpp
*
* Opis:
*		Enostaven streznik, ki zmore sprejeti le enega klienta naenkrat.
*		Streznik sprejme klientove podatke in jih v nespremenjeni obliki poslje
*		nazaj klientu - odmev.
*
*H*/

//Vkljucimo ustrezna zaglavja
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
/*
Definiramo vrata (port) na katerem bo streznik poslusal
in velikost medponilnika za sprejemanje in posiljanje podatkov
*/
#define PORT 1053
#define BUFFER_SIZE 256

int main(int argc, char **argv){

	//Spremenjlivka za preverjane izhodnega statusa funkcij
	int iResult;

	/*
	Ustvarimo nov vtic, ki bo poslusal
	in sprejemal nove kliente preko TCP/IP protokola
	*/
	int listener=socket(AF_INET, SOCK_STREAM, 0);
	if (listener == -1) {
		printf("Error creating socket\n");
		return 1;
	}

	//Nastavimo vrata in mrezni naslov vtica
	sockaddr_in  listenerConf;
	listenerConf.sin_port=htons(PORT);
	listenerConf.sin_family=AF_INET;
	listenerConf.sin_addr.s_addr=INADDR_ANY;

	//Vtic povezemo z ustreznimi vrati
	iResult = bind( listener, (sockaddr *)&listenerConf, sizeof(listenerConf));
	if (iResult == -1) {
		printf("Bind failed\n");
		close(listener);
		return 1;
    }

	//Zacnemo poslusati
	if ( listen( listener, 5 ) == -1 ) {
		printf( "Listen failed\n");
		close(listener);
		return 1;
	}

	//Definiramo nov vtic in medpomnilik
	int clientSock;
	char buff[BUFFER_SIZE];
	
	/*
	V zanki sprejemamo nove povezave
	in jih strezemo (najvec eno naenkrat)
	*/
	while (1)
	{
		//Sprejmi povezavo in ustvari nov vtic
		clientSock = accept(listener,NULL,NULL);
		if (clientSock == -1) {
			printf("Accept failed\n");
			close(listener);
			return 1;
		}

		//Postrezi povezanemu klientu
		do{

			//Sprejmi podatke
			iResult = recv(clientSock, buff, BUFFER_SIZE, 0);
			if (iResult > 0) {
				printf("Bytes received: %d\n", iResult);

				//Vrni prejete podatke posiljatelju
				iResult = send(clientSock, buff, iResult, 0 );
				if (iResult == -1) {
					printf("send failed!\n");
					close(clientSock);
					break;
				}
				printf("Bytes sent: %d\n", iResult);
			}
			else if (iResult == 0)
				printf("Connection closing...\n");
			else{
				printf("recv failed!\n");
				close(clientSock);
				break;
			}

		} while (iResult > 0);

		close(clientSock);
	}

	//Pocistimo vse vtice
	close(listener);

	return 0;
}
