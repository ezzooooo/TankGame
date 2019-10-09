#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 4

void CompressSockets(SOCKET hSockArr[], int idx, int total);
void CompressEvents(WSAEVENT hEventArr[], int idx, int total);
void ErrorHandling(char *msg);

int main(int argc, char *argv[])
{
   WSADATA wsaData;
   SOCKET hServSock, hClntSock;
   SOCKADDR_IN servAdr, clntAdr;

   SOCKET hSockArr[WSA_MAXIMUM_WAIT_EVENTS];
   WSAEVENT hEventArr[WSA_MAXIMUM_WAIT_EVENTS];
   WSAEVENT newEvent;
   WSANETWORKEVENTS netEvents;

   int numOfClntSock = 0;//
   int strLen, i;
   int posInfo, startIdx;
   int clntAdrLen;
   int finish=0;
   char msg[BUF_SIZE];
   int item_x = 0, item_y = 0;//아이템 위치


   srand(time(NULL));
   item_x = rand() % 85 + 8;//8~94까지 랜덤//아이템 시작점 설정
   item_y = rand() % 21 + 4;//4~24까지 랜덤//이이템 시적점 설정

   if (argc != 2) {
      printf("Usage: %s <port>\n", argv[0]);
      exit(1);
   }
   if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
      ErrorHandling("WSAStartup() error!");

   hServSock = socket(PF_INET, SOCK_STREAM, 0);
   servAdr.sin_family = AF_INET;
   servAdr.sin_addr.s_addr = inet_addr("127.0.0.1");
   servAdr.sin_port = htons(atoi(argv[1]));

   if (bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
      ErrorHandling("bind() error");

   if (listen(hServSock, 5) == SOCKET_ERROR)
      ErrorHandling("listen() error");

   newEvent = WSACreateEvent();

   // 1. 소켓에 기다리는 이벤트 등록하기(비동기)
   WSAEventSelect(hServSock, newEvent, FD_ACCEPT);


   // 2. 소켓과 이벤트 객체 저장하기
   hSockArr[numOfClntSock] = hServSock;
   hEventArr[numOfClntSock] = newEvent;

   numOfClntSock++;

   while (1)
   {

      // 3. 이벤트 발생 대기
      posInfo = WSAWaitForMultipleEvents(numOfClntSock, hEventArr, FALSE, WSA_INFINITE, FALSE);

      // 4. 첫 번째 이벤트 위치 계산하기
      startIdx = posInfo - WSA_WAIT_EVENT_0;

      for (i = startIdx; i < numOfClntSock; i++)
      {
         // 5. time_out 또는 실패 확인하기
         int sigEventIdx = WSAWaitForMultipleEvents(1, &hEventArr[i], TRUE, 0, FALSE);

         if ((sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT))
         {
            continue;
         }
         else
         {
            sigEventIdx = i;

            // 6. 발생한 구체적인 이벤트 확인하기
            WSAEnumNetworkEvents(hSockArr[sigEventIdx], hEventArr[sigEventIdx], &netEvents);

            if (netEvents.lNetworkEvents & FD_ACCEPT)
            {
               if (netEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
               {
                  puts("Accept Error");
                  break;
               }

               clntAdrLen = sizeof(clntAdr);
               hClntSock = accept(hSockArr[sigEventIdx], (SOCKADDR*)&clntAdr, &clntAdrLen);
               newEvent = WSACreateEvent();
               WSAEventSelect(hClntSock, newEvent, FD_READ | FD_CLOSE);

               hEventArr[numOfClntSock] = newEvent;
               hSockArr[numOfClntSock] = hClntSock;
               printf("%d 번째 클라이언트 연결 완료\n", numOfClntSock);
               numOfClntSock++;

            }
            //2개의 클라이언트 연결 중 close이벤트 발생할경우 
            if (netEvents.lNetworkEvents & FD_CLOSE)
            {
               numOfClntSock--;
               if (netEvents.iErrorCode[FD_CLOSE_BIT] != 0)
               {
                  printf("%d 번째 클라이언트 Close \n", numOfClntSock);
               }

               WSACloseEvent(hEventArr[sigEventIdx]);
               closesocket(hSockArr[sigEventIdx]);

               CompressSockets(hSockArr, sigEventIdx, numOfClntSock);
               CompressEvents(hEventArr, sigEventIdx, numOfClntSock);

               break;
            }//else end
         }//for end
      }//while end
      if (numOfClntSock == 3)
         break;
   }
   //2개의 클라이언트 연결 완료 end


   recv(hSockArr[1], &msg, BUF_SIZE, 0);
   send(hSockArr[2], msg, sizeof(msg), 0);
   recv(hSockArr[2], &msg, BUF_SIZE, 0);
   send(hSockArr[1], msg, sizeof(msg), 0);

   //아이템
   msg[0] = 4;
   msg[1] = 0;
   msg[2] = item_x;
   msg[3] = item_y;

   send(hSockArr[1], msg, sizeof(msg), 0);
   send(hSockArr[2], msg, sizeof(msg), 0);

   printf("GAME START!\n");

   while (1)
   {
      if (finish == 3) {
         printf("\n엔터키를 누르시면 종료됩니다.\n");
         WSACloseEvent(hEventArr[0]);
         closesocket(hSockArr[0]);
         while (getch() != 13);
         return 0;
      }

      // 3. 이벤트 발생 대기
      posInfo = WSAWaitForMultipleEvents(numOfClntSock, hEventArr, FALSE, WSA_INFINITE, FALSE);

      // 4. 첫 번째 이벤트 위치 계산하기
      startIdx = posInfo - WSA_WAIT_EVENT_0;

      for (i = startIdx; i<numOfClntSock; i++)
      {
         // 5. time_out 또는 실패 확인하기
         int sigEventIdx = WSAWaitForMultipleEvents(1, &hEventArr[i], TRUE, 0, FALSE);

         if ((sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT))
            continue;

         else
         {

            // 6. 발생한 구체적인 이벤트 확인하기
            WSAEnumNetworkEvents(hSockArr[i], hEventArr[i], &netEvents);

            if (netEvents.lNetworkEvents & FD_READ)
            {
               strLen = recv(hSockArr[i], &msg, BUF_SIZE, 0);

               if (posInfo == 1)
                  send(hSockArr[2], msg, sizeof(msg), 0);
               if (posInfo == 2)
                  send(hSockArr[1], msg, sizeof(msg), 0);

               if (msg[0] == 2) {
                  if (posInfo == 1)
                     printf("\n플레이어 2 승리\n\n");
                  else
                     printf("\n플레이어 1 승리\n\n");
                  finish = 1;
               }
            }

            //게임 중 close할 경우
            if (netEvents.lNetworkEvents & FD_CLOSE)
            {
               numOfClntSock--;
               finish++;

               if (netEvents.iErrorCode[FD_CLOSE_BIT] != 0)
               {
                  if (finish == 3)
                     printf("모든 클라이언트 Close\n");
                  else
                     printf("클라이언트 %d개 Close \n", finish-1);
               }

               WSACloseEvent(hEventArr[sigEventIdx]);
               closesocket(hSockArr[sigEventIdx]);

               msg[0] = 3;//상대방이 게임 중 close 했을 경우

               if (posInfo == 1)
                  send(hSockArr[2], msg, sizeof(msg), 0);
               if (posInfo == 2)
                  send(hSockArr[1], msg, sizeof(msg), 0);

               CompressSockets(hSockArr, sigEventIdx, numOfClntSock);
               CompressEvents(hEventArr, sigEventIdx, numOfClntSock);
               break;
            }

         }//else end
      }//for end
   }//while end
}

void CompressSockets(SOCKET hSockArr[], int idx, int total)
{
   int i;
   for (i = idx; i<total; i++)
      hSockArr[i] = hSockArr[i + 1];
}
void CompressEvents(WSAEVENT hEventArr[], int idx, int total)
{
   int i;
   for (i = idx; i<total; i++)
      hEventArr[i] = hEventArr[i + 1];
}
void ErrorHandling(char *msg)
{
   fputs(msg, stderr);
   fputc('\n', stderr);
   exit(1);
}