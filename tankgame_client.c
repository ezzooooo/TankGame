#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <stdlib.h>
#include <winsock2.h>
#include <time.h>

#define BUF_SIZE 4
#define B_UP 0
#define B_LEFT 1
#define B_RIGHT 2
#define B_DOWN 3
#define Width 100
#define Height 30

void Cursor_INVI(void);
void Gotoxy(int, int);
void Check(int*, int*, int*, int*, int*, int*, int);
void Draw(int, int, int);
void UP(int, int);
void LEFT(int, int);
void RIGHT(int, int);
void DOWN(int, int);
void E_UP(int, int);
void E_LEFT(int, int);
void E_RIGHT(int, int);
void E_DOWN(int, int);
int DrawBullet(int*, int, int*, int*, int*, int);
void draw_outline();
int CheckItem(int, int, int, int, int);
void Draw_Enemy(int, int, int);
void ErrorHandling(char *message);
void game_defeat(void);
void game_win(void);
void game_intro(void);


static int i;
int start = 0;
char tank_message[BUF_SIZE] = { 0,0,0,0 };
char bullet_message[BUF_SIZE] = { 1,0,0,0 };
char item_message[BUF_SIZE] = { 4,0,0,0 };
char recv_message[BUF_SIZE];
char item = 'F';
int item_re = 0;


int main(int argc, char *argv[]) {

   //udp 변수
   WSADATA wsaData;
   SOCKET hSocket;
   //char message[BUF_SIZE];
   int strLen;
   SOCKADDR_IN servAdr;
   DWORD recvTimeout = 5;

   //대포게임 변수
   int bx, by, bd = B_RIGHT;
   int bullet_exist = 0;
   int enemy = 1;
   int send_bullet = 0;
   int finish = 0;

   int e_last_key = 0, e_x = 110, e_y = 25;
   int e_bd = 0, e_bx = 116, e_by = 25, e_bullet_exist = 0;
   int last_x = 0, last_y = 0; // 마지막 위치
   int x = 0, y = 0;//시작점

   int last_key = 77;//마지막 INPUT값
   int item_time = 0;


   if (argc != 3) {
      printf("Usage : %s <IP> <port>\n", argv[0]);
      exit(1);
   }

   if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
      ErrorHandling("WSAStartup() error!");

   hSocket = socket(PF_INET, SOCK_STREAM, 0);
   if (hSocket == INVALID_SOCKET)
      ErrorHandling("socket() error");

   memset(&servAdr, 0, sizeof(servAdr));
   servAdr.sin_family = AF_INET;
   servAdr.sin_addr.s_addr = inet_addr(argv[1]);
   servAdr.sin_port = htons(atoi(argv[2]));

   if (connect(hSocket, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
      ErrorHandling("connect() error!");
   else
      puts("Connected...........");


   system("mode con cols=150 lines=30");
   Cursor_INVI();

   srand(time(NULL));
   x = rand() % 90 + 6;//6~90까지 랜덤//탱크 시작점 설정
   y = rand() % 24 + 4;//4~24까지 랜덤//탱크 시적점 설정


   bx = x + 5;
   by = y;

   tank_message[1] = last_key;
   tank_message[2] = x;
   tank_message[3] = y;

   draw_outline();
   Gotoxy(35, 14);
   printf("상대 접속을 기다리고 있습니다.");

   //탱크 좌표 서버에게 보냄
   send(hSocket, tank_message, sizeof(tank_message), 0);
   //상대방의 탱크 좌표 서버로부터 받음
   recv(hSocket, &recv_message, BUF_SIZE, 0);

   setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeout, sizeof(recvTimeout));
   system("cls");
   draw_outline();


   //아이템
   recv(hSocket, &item_message, BUF_SIZE, 0);
   Gotoxy(item_message[2], item_message[3]);
   printf("★");

   game_intro();

   while (1) {

      last_x = x, last_y = y;

      Check(&last_key, &bd, &x, &y, &bx, &by, bullet_exist);

      //자기자신 대포
      SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11); //색정함-연한옥색
      Draw(last_key, x, y);

      //아이템 먹었는지 안먹었는지 0;안먹음 1:내가먹음 2:상대방먹음
      if (item_re == 0) {
         item_re = CheckItem(item_message[2], item_message[3], x, y, recv_message[0]);
      }

      //자기자신 총알
      send_bullet = DrawBullet(&last_key, bd, &bx, &by, &bullet_exist, 3);


      if (last_x != x || last_y != y)
         send(hSocket, tank_message, sizeof(tank_message), 0);

      //아이템 먹었을때
      if (item_re == 1) {
         bullet_message[0] = 5;
         item_message[0] = 6;
         send(hSocket, item_message, sizeof(bullet_message), 0);
         item_re = 3;
      }

      if (send_bullet == 1) {
         send(hSocket, bullet_message, sizeof(bullet_message), 0);//상대방에게 총알 보냄
         send_bullet = 0;
      }

      //상대방의 탱크,총알 좌표 서버로부터 받음
      recv(hSocket, &recv_message, BUF_SIZE, 0);

      if (recv_message[1] != 0 && recv_message[0] == 0) { // 탱크
         e_last_key = recv_message[1];//방향
         e_x = recv_message[2];//x좌표
         e_y = recv_message[3];//y좌표
      }
      else if (recv_message[0] == 1 || recv_message[0] == 5) {// 총알 || 아이템총알
         recv_message[0] = -1;
         e_bullet_exist = 1;
         e_bd = recv_message[1];//총알 방향
         e_bx = recv_message[2];//총알 x축
         e_by = recv_message[3];//총알 y축
      }
      else if (recv_message[0] == 6) { //상대방이 아이템 먹음
         item_re = 2;
      }
      else if (recv_message[0] == 2) {
         break;
      }
      else if (recv_message[0] == 3) {//상대방이  게임중 close 했을경우
         game_win();
         break;
      }

      if (enemy == 1) {
         //상대방 대포
         SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 13); //색정함-연한자주색
         Draw_Enemy(e_last_key, e_x, e_y);
      }
      //상대방 총알
      DrawBullet(&e_last_key, e_bd, &e_bx, &e_by, &e_bullet_exist, 2);

      //총알 맞았는지 확인 
      finish = CheckBullet(e_bd, e_bx, e_by, &e_bullet_exist, x, y, &enemy);

      if (finish == 1) {
         tank_message[0] = 2;
         send(hSocket, tank_message, sizeof(tank_message), 0);
         break;
      }

      Sleep(20);

   }
   if (finish == 1) 
      game_defeat();
   
   else if (recv_message[0] == 2) 
      game_win();

   return 0;
}

void Cursor_INVI() {
   CONSOLE_CURSOR_INFO Curinfo;
   Curinfo.bVisible = FALSE;
   Curinfo.dwSize = 1;
   SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &Curinfo);
}

void Gotoxy(int x, int y) {
   COORD Pos = { x , y };
   SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}

void Check(int* last_key, int* bd, int* x, int* y, int* bx, int* by, int bullet_exist) {


   if (GetConsoleWindow() == GetForegroundWindow()) {


      if (bullet_exist == 1) {
         if (GetAsyncKeyState(VK_UP) < 0 && *y > 4) {
            (*y)--;
            *last_key = 72;
         }
         else if (GetAsyncKeyState(VK_LEFT) < 0 && *x > 8) {
            (*x)--;
            *last_key = 75;
         }
         else if (GetAsyncKeyState(VK_RIGHT) < 0 && *x < 92) {
            (*x)++;
            *last_key = 77;
         }
         else if (GetAsyncKeyState(VK_DOWN) < 0 && *y < 25) {
            (*y)++;
            *last_key = 80;
         }
         else if (GetAsyncKeyState(VK_SPACE) < 0) {
            *last_key = 32;
         }
      }
      else {
         if (GetAsyncKeyState(VK_UP) < 0 && *y > 4) {
            (*y)--;
            *bx = *x;
            *by = *y - 3;
            *last_key = 72;
            *bd = B_UP;
            bullet_message[1] = 0;
         }
         else if (GetAsyncKeyState(VK_LEFT) < 0 && *x > 8) {
            (*x)--;
            *bx = *x - 5;
            *by = *y;
            *bd = B_LEFT;
            *last_key = 75;
            bullet_message[1] = 1;
         }
         else if (GetAsyncKeyState(VK_RIGHT) < 0 && *x < 92) {
            (*x)++;
            *bx = *x + 5;
            *by = *y;
            *last_key = 77;
            *bd = B_RIGHT;
            bullet_message[1] = 2;
         }
         else if (GetAsyncKeyState(VK_DOWN) < 0 && *y < 25) {
            (*y)++;
            *bx = *x;
            *by = *y + 3;
            *last_key = 80;
            *bd = B_DOWN;

         }
         else if (GetAsyncKeyState(VK_SPACE) < 0) {
            *last_key = 32;
            bullet_message[1] = *bd;
            bullet_message[2] = *bx;
            bullet_message[3] = *by;
         }

      }
   }
}



//대포 방향 제어
void Draw(int last_key, int x, int y) {

   tank_message[1] = last_key;
   tank_message[2] = x;
   tank_message[3] = y;

   switch (last_key) {
   case 72:
      UP(x, y);
      break;

   case 75:
      LEFT(x, y);
      break;

   case 77:
      RIGHT(x, y);
      break;

   case 80:
      DOWN(x, y);
      break;
   }
}

void UP(int x, int y) {

   Gotoxy(x, y + 2);
   printf(" ");
   Gotoxy(x, y + 1);
   printf(" ");
   Gotoxy(x - 2, y + 1);
   printf("      ");

   Gotoxy(x, y);
   printf("▣");
   Gotoxy(x, y - 1);
   printf("∥");
}

void LEFT(int x, int y) {

   Gotoxy(x + 1, y);
   printf("   ");
   Gotoxy(x + 2, y - 1);
   printf("  ");
   Gotoxy(x + 2, y + 1);
   printf("  ");

   Gotoxy(x, y);
   printf("▣");
   Gotoxy(x - 2, y);
   printf("〓");
}

void RIGHT(int x, int y) {
   Gotoxy(x - 3, y);
   printf("   ");
   Gotoxy(x - 1, y - 1);
   printf("  ");
   Gotoxy(x - 1, y + 1);
   printf("  ");

   Gotoxy(x, y);
   printf("▣");
   Gotoxy(x + 2, y);
   printf("〓");
}

void DOWN(int x, int y) {
   Gotoxy(x, y - 2);
   printf(" ");
   Gotoxy(x, y - 1);
   printf(" ");
   Gotoxy(x - 2, y - 1);
   printf("      ");

   Gotoxy(x, y);
   printf("▣");
   Gotoxy(x, y + 1);
   printf("∥");
}


//총알 그리기 //enemy_check:1 자기자신 enemy_check: 2 상대방
int DrawBullet(int* last_key, int bullet_direct, int* bx, int* by, int* bullet_exist, int enemy_check) {

   if (*bx < 4 || *bx > 95 || *by < 2 || *by > 27) {
      *bullet_exist = 0;
      if (bullet_direct == B_LEFT || bullet_direct == B_RIGHT) {
         Gotoxy(*bx - 1, *by);
         printf("   ");
         Gotoxy(*bx - 1, *by + 1);
         printf("   ");
         Gotoxy(*bx - 1, *by - 1);
         printf("   ");


      }
      if (bullet_direct == B_UP) {
         Gotoxy(*bx, *by + 1);
         printf("  ");
         Gotoxy(*bx + 2, *by + 1);
         printf("  ");
         Gotoxy(*bx - 2, *by + 1);
         printf("  ");

      }
      if (bullet_direct == B_DOWN) {
         Gotoxy(*bx + 2, *by - 1);
         printf("  ");
         Gotoxy(*bx, *by - 1);
         printf("  ");
         Gotoxy(*bx - 2, *by - 1);
         printf("  ");
      }

      *last_key = 0;

   }

   if (*last_key == 32) { //마지막 누를때가 스페이바일때

      if (*bullet_exist == 1) {
         if (bullet_direct == B_UP) {
            Gotoxy(*bx, (*by) + 1);
            printf(" ");
            Gotoxy(*bx, *by);
            printf("＊");

            if (item_re == enemy_check) {
               Gotoxy(*bx + 2, (*by) + 1);
               printf(" ");
               Gotoxy(*bx - 2, (*by) + 1);
               printf(" ");
               Gotoxy(*bx + 2, *by);
               printf("＊");
               Gotoxy(*bx - 2, *by);
               printf("＊");

            }

            *by = *by - 1;
         }

         else if (bullet_direct == B_LEFT) {
            Gotoxy((*bx) + 1, *by);
            printf(" ");
            Gotoxy(*bx, *by);
            printf("＊");

            if (item_re == enemy_check) {
               Gotoxy((*bx) + 1, *by + 1);
               printf(" ");
               Gotoxy((*bx) + 1, *by - 1);
               printf(" ");
               Gotoxy(*bx, *by + 1);
               printf("＊");
               Gotoxy(*bx, *by - 1);
               printf("＊");
            }
            *bx = *bx - 1;
         }

         else if (bullet_direct == B_RIGHT) {
            Gotoxy((*bx) - 1, *by);
            printf(" ");
            Gotoxy(*bx, *by);
            printf("＊");

            if (item_re == enemy_check) {
               Gotoxy((*bx) - 1, *by + 1);
               printf(" ");
               Gotoxy(*bx, *by + 1);
               printf("＊");
               Gotoxy((*bx) - 1, *by - 1);
               printf(" ");
               Gotoxy(*bx, *by - 1);
               printf("＊");
            }
            *bx = *bx + 1;
         }
         else if (bullet_direct == B_DOWN) {
            Gotoxy(*bx, (*by) - 1);
            printf(" ");
            Gotoxy(*bx, *by);
            printf("＊");
            if (item_re == enemy_check) {
               Gotoxy(*bx + 2, (*by) - 1);
               printf(" ");
               Gotoxy(*bx + 2, *by);
               printf("＊");
               Gotoxy(*bx - 2, (*by) - 1);
               printf(" ");
               Gotoxy(*bx - 2, *by);
               printf("＊");
            }
            *by = *by + 1;
         }
         return 0;
      }

      else {

         if (bullet_direct == B_UP) {
            Gotoxy(*bx, *by);
            printf("＊");

            if (item_re == enemy_check) {
               Gotoxy(*bx + 2, (*by) + 1);
               printf(" ");
               Gotoxy(*bx + 2, *by);
               printf("＊");
               Gotoxy(*bx - 2, (*by) + 1);
               printf(" ");
               Gotoxy(*bx - 2, *by);
               printf("＊");

            }
            Gotoxy(40, 10);

            *by = *by - 1;
         }

         else if (bullet_direct == B_LEFT) {
            Gotoxy(*bx, *by);
            printf("＊");

            if (item_re == enemy_check) {
               Gotoxy((*bx) + 1, *by + 1);
               printf(" ");
               Gotoxy(*bx, *by + 1);
               printf("＊");
               Gotoxy((*bx) + 1, *by - 1);
               printf(" ");
               Gotoxy(*bx, *by - 1);
               printf("＊");
            }
            *bx = *bx - 1;
         }

         else if (bullet_direct == B_RIGHT) {
            Gotoxy(*bx, *by);
            printf("＊");
            if (item_re == enemy_check) {
               Gotoxy((*bx) - 1, *by + 1);
               printf(" ");
               Gotoxy(*bx, *by + 1);
               printf("＊");
               Gotoxy((*bx) - 1, *by - 1);
               printf(" ");
               Gotoxy(*bx, *by - 1);
               printf("＊");
            }
            *bx = *bx + 1;
         }
         else if (bullet_direct == B_DOWN) {
            Gotoxy(*bx, *by);
            printf("＊");
            if (item_re == enemy_check) {
               Gotoxy(*bx + 2, (*by) - 1);
               printf(" ");
               Gotoxy(*bx + 2, *by);
               printf("＊");
               Gotoxy(*bx - 2, (*by) - 1);
               printf(" ");
               Gotoxy(*bx - 2, *by);
               printf("＊");
            }
            *by = *by + 1;
         }
      }
      *bullet_exist = 1;
      return 1;
   }

   else {//스페이스바를 누르지않음

      if (*bullet_exist == 1) {//총알이 존재함
         if (bullet_direct == B_UP || bullet_direct == 0) {
            Gotoxy(*bx, (*by) + 1);
            printf(" ");
            Gotoxy(*bx, *by);
            printf("＊");


            if (item_re == enemy_check) {

               Gotoxy(*bx + 2, (*by) + 1);
               printf(" ");
               Gotoxy(*bx - 2, (*by) + 1);
               printf(" ");
               Gotoxy(*bx + 2, *by);
               printf("＊");
               Gotoxy(*bx - 2, *by);
               printf("＊");

            }
            *by = *by - 1;
         }

         else if (bullet_direct == B_LEFT || bullet_direct == 1) {
            Gotoxy((*bx) + 1, *by);
            printf(" ");
            Gotoxy(*bx, *by);
            printf("＊");

            if (item_re == enemy_check) {
               Gotoxy((*bx) + 1, *by + 1);
               printf(" ");
               Gotoxy(*bx, *by + 1);
               printf("＊");
               Gotoxy((*bx) + 1, *by - 1);
               printf(" ");
               Gotoxy(*bx, *by - 1);
               printf("＊");
            }
            *bx = *bx - 1;
         }

         else if (bullet_direct == B_RIGHT || bullet_direct == 2) {
            Gotoxy((*bx) - 1, *by);
            printf(" ");
            Gotoxy(*bx, *by);
            printf("＊");
            if (item_re == enemy_check) {
               Gotoxy((*bx) - 1, *by + 1);
               printf(" ");
               Gotoxy(*bx, *by + 1);
               printf("＊");
               Gotoxy((*bx) - 1, *by - 1);
               printf(" ");
               Gotoxy(*bx, *by - 1);
               printf("＊");
            }
            *bx = *bx + 1;
         }
         else if (bullet_direct == B_DOWN || bullet_direct == 3) {
            Gotoxy(*bx, (*by) - 1);
            printf(" ");
            Gotoxy(*bx, *by);
            printf("＊");
            if (item_re == enemy_check) {
               Gotoxy(*bx + 2, (*by) - 1);
               printf(" ");
               Gotoxy(*bx + 2, *by);
               printf("＊");
               Gotoxy(*bx - 2, (*by) - 1);
               printf(" ");
               Gotoxy(*bx - 2, *by);
               printf("＊");
            }
            *by = *by + 1;
         }
      }
      return 0;
   }
}

void draw_outline()
{
   int i;
   SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14); //색정함-노랑색
   for (i = 0; i < Width / 2; i++) {
      printf("▦");
   }
   for (i = 0; i < Height; i++) {
      Gotoxy(0, i); printf("▦");
   }
   for (i = 0; i < Height; i++) {
      Gotoxy(Width, i);
      printf("▦");
   }
   Gotoxy(0, Height - 1);
   for (i = 0; i < Width / 2; i++) {
      printf("▦");
   }

}

//총알 맞았는지 확인
int CheckBullet(int e_bd, int bx, int by, int* e_bullet_exist, int ex, int ey, int* enemy) {

   if (*e_bullet_exist == 1) {//상대방의 총알이 존재 할때

      if (item_re == 2) {//아이템 먹었을때

         if (e_bd == 0 || e_bd == 3) {
            if ((bx - 1 == ex && by == ey) || (bx + 1 == ex && by == ey) || (bx == ex && by == ey) ||
               (bx - 2 == ex && by == ey) || (bx + 2 == ex && by == ey) ||
               (bx - 3 == ex && by == ey) || (bx + 3 == ex && by == ey)) {
               Gotoxy(bx, by);
               printf("  ");
               e_bd = 0;
               *e_bullet_exist = 0;
               *enemy = 0;
               Gotoxy(115, 20);
               printf("게임끝");

               return 1;
            }
         }
         else if (e_bd == 1 || e_bd == 2) {
            if ((bx == ex && by == ey) || (bx == ex && by == ey + 1) || (bx == ex && by == ey - 1) ||
               (bx == ex && by == ey + 2) || (bx == ex && by == ey - 2)) {
               Gotoxy(bx, by);
               printf("  ");
               e_bd = 0;
               *e_bullet_exist = 0;
               *enemy = 0;
               Gotoxy(115, 20);
               printf("게임끝");

               return 1;
            }
         }//if end

         return 0;

      }
      else {//아이템 안먹었을때

         if ((bx == ex && by == ey)) {
            Gotoxy(bx, by);
            printf("  ");
            e_bd = 0;
            *e_bullet_exist = 0;
            *enemy = 0;
            Gotoxy(115, 20);
            printf("게임끝");

            return 1;
         }//if end
         return 0;
      }

      return 0;
   }
}

void game_intro() {
   Gotoxy(123, 3);
   printf("조작법");
   Gotoxy(112, 6);
   printf("위");
   Gotoxy(112, 7);
   printf("↑");
   Gotoxy(106, 8);
   printf("왼쪽←↓→오른쪽     Space : 총알발사");
   Gotoxy(111, 9);
   printf("아래");
   Gotoxy(104, 11);
   printf("===========================================");
   Gotoxy(123, 13);
   printf("게임설명");
   Gotoxy(106, 15);
   printf("내 탱크 : ▣〓             적 탱크 : ◎〓");
   Gotoxy(106, 18);
   printf("1. 내 탱크가 적 총알에 맞았을 시 패배");
   Gotoxy(106, 20);
   printf("2. 총알은 한발만 발사 가능");
   Gotoxy(106, 22);
   printf("3. ★을 획득할 시 총알이 3발씩 발사됨");
}

//아이템 획득했는지
int CheckItem(int ix, int iy, int x, int y, int check) {

   if (ix == x && iy == y && check == 0) {
      Gotoxy(x, y);
      printf(" ");
      Gotoxy(118, 25);
      printf("★아이템 획득★");
      item = 'Y';
      check = 1;
      return 1;
   }
   else {
      Gotoxy(ix, iy);
      printf("★");
   }
   return 0;
}

void Draw_Enemy(int last_key, int x, int y) {
   switch (last_key) {
   case 72:
      E_UP(x, y);
      break;

   case 75:
      E_LEFT(x, y);
      break;

   case 77:
      E_RIGHT(x, y);
      break;

   case 80:
      E_DOWN(x, y);
      break;
   }
}


void ErrorHandling(char *message)
{
   fputs(message, stderr);
   fputc('\n', stderr);
   exit(1);
}

void E_UP(int x, int y) {

   Gotoxy(x, y + 2);
   printf(" ");
   Gotoxy(x, y + 1);
   printf(" ");
   Gotoxy(x - 2, y + 1);
   printf("      ");

   Gotoxy(x, y);
   printf("◎");
   Gotoxy(x, y - 1);
   printf("∥");
}

void E_LEFT(int x, int y) {

   Gotoxy(x + 1, y);
   printf("   ");
   Gotoxy(x + 2, y - 1);
   printf("  ");
   Gotoxy(x + 2, y + 1);
   printf("  ");

   Gotoxy(x, y);
   printf("◎");
   Gotoxy(x - 2, y);
   printf("〓");
}

void E_RIGHT(int x, int y) {
   Gotoxy(x - 3, y);
   printf("   ");
   Gotoxy(x - 1, y - 1);
   printf("  ");
   Gotoxy(x - 1, y + 1);
   printf("  ");

   Gotoxy(x, y);
   printf("◎");
   Gotoxy(x + 2, y);
   printf("〓");
}

void E_DOWN(int x, int y) {
   Gotoxy(x, y - 2);
   printf(" ");
   Gotoxy(x, y - 1);
   printf(" ");
   Gotoxy(x - 2, y - 1);
   printf("      ");

   Gotoxy(x, y);
   printf("◎");
   Gotoxy(x, y + 1);
   printf("∥");
}

void game_defeat() {

   system("cls");
   draw_outline();
   Gotoxy(46, 13);
   printf("게임패배");
   Gotoxy(36, 15);
   printf("엔터키를 누르시면 종료됩니다.");
   while (getch() != 13);
}

void game_win() {

   system("cls");
   draw_outline();
   Gotoxy(46, 13);
   printf("게임승리");
   Gotoxy(36, 15);
   printf("엔터키를 누르시면 종료됩니다.");
   while (getch() != 13);

}