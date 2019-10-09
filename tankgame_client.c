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

   //udp ����
   WSADATA wsaData;
   SOCKET hSocket;
   //char message[BUF_SIZE];
   int strLen;
   SOCKADDR_IN servAdr;
   DWORD recvTimeout = 5;

   //�������� ����
   int bx, by, bd = B_RIGHT;
   int bullet_exist = 0;
   int enemy = 1;
   int send_bullet = 0;
   int finish = 0;

   int e_last_key = 0, e_x = 110, e_y = 25;
   int e_bd = 0, e_bx = 116, e_by = 25, e_bullet_exist = 0;
   int last_x = 0, last_y = 0; // ������ ��ġ
   int x = 0, y = 0;//������

   int last_key = 77;//������ INPUT��
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
   x = rand() % 90 + 6;//6~90���� ����//��ũ ������ ����
   y = rand() % 24 + 4;//4~24���� ����//��ũ ������ ����


   bx = x + 5;
   by = y;

   tank_message[1] = last_key;
   tank_message[2] = x;
   tank_message[3] = y;

   draw_outline();
   Gotoxy(35, 14);
   printf("��� ������ ��ٸ��� �ֽ��ϴ�.");

   //��ũ ��ǥ �������� ����
   send(hSocket, tank_message, sizeof(tank_message), 0);
   //������ ��ũ ��ǥ �����κ��� ����
   recv(hSocket, &recv_message, BUF_SIZE, 0);

   setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeout, sizeof(recvTimeout));
   system("cls");
   draw_outline();


   //������
   recv(hSocket, &item_message, BUF_SIZE, 0);
   Gotoxy(item_message[2], item_message[3]);
   printf("��");

   game_intro();

   while (1) {

      last_x = x, last_y = y;

      Check(&last_key, &bd, &x, &y, &bx, &by, bullet_exist);

      //�ڱ��ڽ� ����
      SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11); //������-���ѿ���
      Draw(last_key, x, y);

      //������ �Ծ����� �ȸԾ����� 0;�ȸ��� 1:�������� 2:�������
      if (item_re == 0) {
         item_re = CheckItem(item_message[2], item_message[3], x, y, recv_message[0]);
      }

      //�ڱ��ڽ� �Ѿ�
      send_bullet = DrawBullet(&last_key, bd, &bx, &by, &bullet_exist, 3);


      if (last_x != x || last_y != y)
         send(hSocket, tank_message, sizeof(tank_message), 0);

      //������ �Ծ�����
      if (item_re == 1) {
         bullet_message[0] = 5;
         item_message[0] = 6;
         send(hSocket, item_message, sizeof(bullet_message), 0);
         item_re = 3;
      }

      if (send_bullet == 1) {
         send(hSocket, bullet_message, sizeof(bullet_message), 0);//���濡�� �Ѿ� ����
         send_bullet = 0;
      }

      //������ ��ũ,�Ѿ� ��ǥ �����κ��� ����
      recv(hSocket, &recv_message, BUF_SIZE, 0);

      if (recv_message[1] != 0 && recv_message[0] == 0) { // ��ũ
         e_last_key = recv_message[1];//����
         e_x = recv_message[2];//x��ǥ
         e_y = recv_message[3];//y��ǥ
      }
      else if (recv_message[0] == 1 || recv_message[0] == 5) {// �Ѿ� || �������Ѿ�
         recv_message[0] = -1;
         e_bullet_exist = 1;
         e_bd = recv_message[1];//�Ѿ� ����
         e_bx = recv_message[2];//�Ѿ� x��
         e_by = recv_message[3];//�Ѿ� y��
      }
      else if (recv_message[0] == 6) { //������ ������ ����
         item_re = 2;
      }
      else if (recv_message[0] == 2) {
         break;
      }
      else if (recv_message[0] == 3) {//������  ������ close �������
         game_win();
         break;
      }

      if (enemy == 1) {
         //���� ����
         SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 13); //������-�������ֻ�
         Draw_Enemy(e_last_key, e_x, e_y);
      }
      //���� �Ѿ�
      DrawBullet(&e_last_key, e_bd, &e_bx, &e_by, &e_bullet_exist, 2);

      //�Ѿ� �¾Ҵ��� Ȯ�� 
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



//���� ���� ����
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
   printf("��");
   Gotoxy(x, y - 1);
   printf("��");
}

void LEFT(int x, int y) {

   Gotoxy(x + 1, y);
   printf("   ");
   Gotoxy(x + 2, y - 1);
   printf("  ");
   Gotoxy(x + 2, y + 1);
   printf("  ");

   Gotoxy(x, y);
   printf("��");
   Gotoxy(x - 2, y);
   printf("��");
}

void RIGHT(int x, int y) {
   Gotoxy(x - 3, y);
   printf("   ");
   Gotoxy(x - 1, y - 1);
   printf("  ");
   Gotoxy(x - 1, y + 1);
   printf("  ");

   Gotoxy(x, y);
   printf("��");
   Gotoxy(x + 2, y);
   printf("��");
}

void DOWN(int x, int y) {
   Gotoxy(x, y - 2);
   printf(" ");
   Gotoxy(x, y - 1);
   printf(" ");
   Gotoxy(x - 2, y - 1);
   printf("      ");

   Gotoxy(x, y);
   printf("��");
   Gotoxy(x, y + 1);
   printf("��");
}


//�Ѿ� �׸��� //enemy_check:1 �ڱ��ڽ� enemy_check: 2 ����
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

   if (*last_key == 32) { //������ �������� �����̹��϶�

      if (*bullet_exist == 1) {
         if (bullet_direct == B_UP) {
            Gotoxy(*bx, (*by) + 1);
            printf(" ");
            Gotoxy(*bx, *by);
            printf("��");

            if (item_re == enemy_check) {
               Gotoxy(*bx + 2, (*by) + 1);
               printf(" ");
               Gotoxy(*bx - 2, (*by) + 1);
               printf(" ");
               Gotoxy(*bx + 2, *by);
               printf("��");
               Gotoxy(*bx - 2, *by);
               printf("��");

            }

            *by = *by - 1;
         }

         else if (bullet_direct == B_LEFT) {
            Gotoxy((*bx) + 1, *by);
            printf(" ");
            Gotoxy(*bx, *by);
            printf("��");

            if (item_re == enemy_check) {
               Gotoxy((*bx) + 1, *by + 1);
               printf(" ");
               Gotoxy((*bx) + 1, *by - 1);
               printf(" ");
               Gotoxy(*bx, *by + 1);
               printf("��");
               Gotoxy(*bx, *by - 1);
               printf("��");
            }
            *bx = *bx - 1;
         }

         else if (bullet_direct == B_RIGHT) {
            Gotoxy((*bx) - 1, *by);
            printf(" ");
            Gotoxy(*bx, *by);
            printf("��");

            if (item_re == enemy_check) {
               Gotoxy((*bx) - 1, *by + 1);
               printf(" ");
               Gotoxy(*bx, *by + 1);
               printf("��");
               Gotoxy((*bx) - 1, *by - 1);
               printf(" ");
               Gotoxy(*bx, *by - 1);
               printf("��");
            }
            *bx = *bx + 1;
         }
         else if (bullet_direct == B_DOWN) {
            Gotoxy(*bx, (*by) - 1);
            printf(" ");
            Gotoxy(*bx, *by);
            printf("��");
            if (item_re == enemy_check) {
               Gotoxy(*bx + 2, (*by) - 1);
               printf(" ");
               Gotoxy(*bx + 2, *by);
               printf("��");
               Gotoxy(*bx - 2, (*by) - 1);
               printf(" ");
               Gotoxy(*bx - 2, *by);
               printf("��");
            }
            *by = *by + 1;
         }
         return 0;
      }

      else {

         if (bullet_direct == B_UP) {
            Gotoxy(*bx, *by);
            printf("��");

            if (item_re == enemy_check) {
               Gotoxy(*bx + 2, (*by) + 1);
               printf(" ");
               Gotoxy(*bx + 2, *by);
               printf("��");
               Gotoxy(*bx - 2, (*by) + 1);
               printf(" ");
               Gotoxy(*bx - 2, *by);
               printf("��");

            }
            Gotoxy(40, 10);

            *by = *by - 1;
         }

         else if (bullet_direct == B_LEFT) {
            Gotoxy(*bx, *by);
            printf("��");

            if (item_re == enemy_check) {
               Gotoxy((*bx) + 1, *by + 1);
               printf(" ");
               Gotoxy(*bx, *by + 1);
               printf("��");
               Gotoxy((*bx) + 1, *by - 1);
               printf(" ");
               Gotoxy(*bx, *by - 1);
               printf("��");
            }
            *bx = *bx - 1;
         }

         else if (bullet_direct == B_RIGHT) {
            Gotoxy(*bx, *by);
            printf("��");
            if (item_re == enemy_check) {
               Gotoxy((*bx) - 1, *by + 1);
               printf(" ");
               Gotoxy(*bx, *by + 1);
               printf("��");
               Gotoxy((*bx) - 1, *by - 1);
               printf(" ");
               Gotoxy(*bx, *by - 1);
               printf("��");
            }
            *bx = *bx + 1;
         }
         else if (bullet_direct == B_DOWN) {
            Gotoxy(*bx, *by);
            printf("��");
            if (item_re == enemy_check) {
               Gotoxy(*bx + 2, (*by) - 1);
               printf(" ");
               Gotoxy(*bx + 2, *by);
               printf("��");
               Gotoxy(*bx - 2, (*by) - 1);
               printf(" ");
               Gotoxy(*bx - 2, *by);
               printf("��");
            }
            *by = *by + 1;
         }
      }
      *bullet_exist = 1;
      return 1;
   }

   else {//�����̽��ٸ� ����������

      if (*bullet_exist == 1) {//�Ѿ��� ������
         if (bullet_direct == B_UP || bullet_direct == 0) {
            Gotoxy(*bx, (*by) + 1);
            printf(" ");
            Gotoxy(*bx, *by);
            printf("��");


            if (item_re == enemy_check) {

               Gotoxy(*bx + 2, (*by) + 1);
               printf(" ");
               Gotoxy(*bx - 2, (*by) + 1);
               printf(" ");
               Gotoxy(*bx + 2, *by);
               printf("��");
               Gotoxy(*bx - 2, *by);
               printf("��");

            }
            *by = *by - 1;
         }

         else if (bullet_direct == B_LEFT || bullet_direct == 1) {
            Gotoxy((*bx) + 1, *by);
            printf(" ");
            Gotoxy(*bx, *by);
            printf("��");

            if (item_re == enemy_check) {
               Gotoxy((*bx) + 1, *by + 1);
               printf(" ");
               Gotoxy(*bx, *by + 1);
               printf("��");
               Gotoxy((*bx) + 1, *by - 1);
               printf(" ");
               Gotoxy(*bx, *by - 1);
               printf("��");
            }
            *bx = *bx - 1;
         }

         else if (bullet_direct == B_RIGHT || bullet_direct == 2) {
            Gotoxy((*bx) - 1, *by);
            printf(" ");
            Gotoxy(*bx, *by);
            printf("��");
            if (item_re == enemy_check) {
               Gotoxy((*bx) - 1, *by + 1);
               printf(" ");
               Gotoxy(*bx, *by + 1);
               printf("��");
               Gotoxy((*bx) - 1, *by - 1);
               printf(" ");
               Gotoxy(*bx, *by - 1);
               printf("��");
            }
            *bx = *bx + 1;
         }
         else if (bullet_direct == B_DOWN || bullet_direct == 3) {
            Gotoxy(*bx, (*by) - 1);
            printf(" ");
            Gotoxy(*bx, *by);
            printf("��");
            if (item_re == enemy_check) {
               Gotoxy(*bx + 2, (*by) - 1);
               printf(" ");
               Gotoxy(*bx + 2, *by);
               printf("��");
               Gotoxy(*bx - 2, (*by) - 1);
               printf(" ");
               Gotoxy(*bx - 2, *by);
               printf("��");
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
   SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14); //������-�����
   for (i = 0; i < Width / 2; i++) {
      printf("��");
   }
   for (i = 0; i < Height; i++) {
      Gotoxy(0, i); printf("��");
   }
   for (i = 0; i < Height; i++) {
      Gotoxy(Width, i);
      printf("��");
   }
   Gotoxy(0, Height - 1);
   for (i = 0; i < Width / 2; i++) {
      printf("��");
   }

}

//�Ѿ� �¾Ҵ��� Ȯ��
int CheckBullet(int e_bd, int bx, int by, int* e_bullet_exist, int ex, int ey, int* enemy) {

   if (*e_bullet_exist == 1) {//������ �Ѿ��� ���� �Ҷ�

      if (item_re == 2) {//������ �Ծ�����

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
               printf("���ӳ�");

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
               printf("���ӳ�");

               return 1;
            }
         }//if end

         return 0;

      }
      else {//������ �ȸԾ�����

         if ((bx == ex && by == ey)) {
            Gotoxy(bx, by);
            printf("  ");
            e_bd = 0;
            *e_bullet_exist = 0;
            *enemy = 0;
            Gotoxy(115, 20);
            printf("���ӳ�");

            return 1;
         }//if end
         return 0;
      }

      return 0;
   }
}

void game_intro() {
   Gotoxy(123, 3);
   printf("���۹�");
   Gotoxy(112, 6);
   printf("��");
   Gotoxy(112, 7);
   printf("��");
   Gotoxy(106, 8);
   printf("���ʡ��������     Space : �Ѿ˹߻�");
   Gotoxy(111, 9);
   printf("�Ʒ�");
   Gotoxy(104, 11);
   printf("===========================================");
   Gotoxy(123, 13);
   printf("���Ӽ���");
   Gotoxy(106, 15);
   printf("�� ��ũ : �á�             �� ��ũ : �ݡ�");
   Gotoxy(106, 18);
   printf("1. �� ��ũ�� �� �Ѿ˿� �¾��� �� �й�");
   Gotoxy(106, 20);
   printf("2. �Ѿ��� �ѹ߸� �߻� ����");
   Gotoxy(106, 22);
   printf("3. ���� ȹ���� �� �Ѿ��� 3�߾� �߻��");
}

//������ ȹ���ߴ���
int CheckItem(int ix, int iy, int x, int y, int check) {

   if (ix == x && iy == y && check == 0) {
      Gotoxy(x, y);
      printf(" ");
      Gotoxy(118, 25);
      printf("�ھ����� ȹ���");
      item = 'Y';
      check = 1;
      return 1;
   }
   else {
      Gotoxy(ix, iy);
      printf("��");
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
   printf("��");
   Gotoxy(x, y - 1);
   printf("��");
}

void E_LEFT(int x, int y) {

   Gotoxy(x + 1, y);
   printf("   ");
   Gotoxy(x + 2, y - 1);
   printf("  ");
   Gotoxy(x + 2, y + 1);
   printf("  ");

   Gotoxy(x, y);
   printf("��");
   Gotoxy(x - 2, y);
   printf("��");
}

void E_RIGHT(int x, int y) {
   Gotoxy(x - 3, y);
   printf("   ");
   Gotoxy(x - 1, y - 1);
   printf("  ");
   Gotoxy(x - 1, y + 1);
   printf("  ");

   Gotoxy(x, y);
   printf("��");
   Gotoxy(x + 2, y);
   printf("��");
}

void E_DOWN(int x, int y) {
   Gotoxy(x, y - 2);
   printf(" ");
   Gotoxy(x, y - 1);
   printf(" ");
   Gotoxy(x - 2, y - 1);
   printf("      ");

   Gotoxy(x, y);
   printf("��");
   Gotoxy(x, y + 1);
   printf("��");
}

void game_defeat() {

   system("cls");
   draw_outline();
   Gotoxy(46, 13);
   printf("�����й�");
   Gotoxy(36, 15);
   printf("����Ű�� �����ø� ����˴ϴ�.");
   while (getch() != 13);
}

void game_win() {

   system("cls");
   draw_outline();
   Gotoxy(46, 13);
   printf("���ӽ¸�");
   Gotoxy(36, 15);
   printf("����Ű�� �����ø� ����˴ϴ�.");
   while (getch() != 13);

}