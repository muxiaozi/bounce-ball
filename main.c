#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

#define LEFT 1   
#define RIGHT 30
#define TOP 1
#define BUTTON 22 

//=============================================================================
// 自定义结构体
typedef struct POINT	//点
{
	int x;
	int y;
}Point;

struct BALL		//球
{
	int x;
	int y;
	int x_Speed;
	int y_Speed;
	int mode;	//0->普通；1->实心；2->冰霜；
}Ball[3];

struct BORD		//板
{
	int x;
	int y;
	int length;	//长度
}Bord;

struct TREASURE	//宝石
{
	int x;
	int y;
	int type;
}Treasure;

enum color{ green,blue,red };
enum mode{ run_Game,creat_Map };
	
//=============================================================================
// 自定义函数的声明
void gotoxy(int x, int y);

int mainGame();		//主函数
void initGame(int mode);	//初始化游戏

void runGame();		//运行游戏
void creatMap();	//制作地图
void helpGame();	//游戏帮助
void selectMap();	//选择地图
void playGame();	//游戏完成时
int checkComplete();	//检测游戏是否完成

void PrintBord();	//绘制木板和小球
void PrintLev();	//初始化相关关卡
void MoveBord(int orientation);	//移动木板
void MoveBall(int index);	//移动小球

void removeDiamond(int index,int direction);	//移除方块
void removeColor(enum color,Point diamond);		//解决颜色
void showTreasure(int intRand,Point point_diamond);		//宝物
void move_Treasure();	//移动宝石
void get_Treasure();	//获得宝石
void time_Treasure();	//宝石失效

//==============================================================================
//自定义全局变量、全局常量
HANDLE g_hConsoleOutput;  // 获取控制台输出句柄

short potVal[40][30];	//0—空白，1—1级方块，2—2级方块，3—3级方块，4—围墙

int live=1;				//生命
int stop=0;		//0->不暂停，1->暂停
int speed=1000;	//速度
int intGrade=0;	//分数
int intTreasure=0;	//宝石数
int getTreasure=0;	//获得宝石
int level_Now=0;	//当前关卡
int level_Max=0;	//关卡数量

char fileName[100][20];	//目前设置最高地图容量为100个
char game_Map[20];	//游戏地图

clock_t treaTime_Last,treaTime_Now;	//宝物持续时间

static const char* BALL_SHAPE[]={"⊙","◎","●"};	
// =============================================================================
// 主菜单

int main()
{
    g_hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);  // 获取控制台输出句柄
	SetConsoleTitleA("弹弹乐 - by 慕宵子");

// 	char chVolume[256];
// 	long lVolume;
// 	mciSendString("status movie volume",chVolume,255,0);
// 	lVolume=strtol(chVolume,NULL,10);
	
	mciSendString("play music\\bg_music.mp3 repeat ",NULL, 0,NULL);	//背景音乐
	mciSendString ("setaudio music\\bg_music.mp3 volume to 100",NULL,0,NULL);	//设置音量
	
	do
	{
		CONSOLE_CURSOR_INFO cursorInfo = { 1, FALSE };  // 光标信息
		SetConsoleCursorInfo(g_hConsoleOutput, &cursorInfo);  // 设置光标隐藏

		switch(mainGame())	//游戏选项
		{
		case 0:
			playGame();
			break;
		case 1:
			creatMap();
			break;
		case 2:
			helpGame();
			break;
		case 3:
			selectMap();
			playGame();
			break;
		}
	}while(1);
	
	system("pause>nul");
	return 0;
}

// =============================================================================
// 主菜单
int mainGame()
{
	FILE *fMap;

	int ch,index=0;
	static const char *modeItem[]={"☆开始游戏","☆制作关卡","☆游戏帮助","☆选择关卡"};
	
	system("cls");
	
	SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_INTENSITY);
    gotoxy(15, 5);
    printf("┏━━━━━━━━━━━━━━┓");
    gotoxy(15, 6);
    printf("┃%2s%s%2s┃", "", "★弹弹乐★", "");
    gotoxy(15, 7);
    printf("┗━━━━━━━━━━━━━━┛");
	
	SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);
	gotoxy(16, 14);
	printf("%2s%s%2s", "", modeItem[0], "");
	SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
	gotoxy(16, 16);
	printf("%2s%s%2s", "", modeItem[1], "");
	SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
	gotoxy(16, 18);
	printf("%2s%s%2s", "", modeItem[2], "");
	SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
	gotoxy(16, 20);
	printf("%2s%s%2s", "", modeItem[3], "");

	if(fopen_s(&fMap, "maps\\MapInfo.dat", "r"))
	{
		gotoxy(LEFT+4,TOP+2);
		printf("打开地图信息文件发生错误!");
		gotoxy(LEFT+4,TOP+4);
		printf("请查看MapInfo.dat文件是否存在!!!");
		exit(0);
	}
	
	while(fgets(fileName[level_Max],20,fMap)!=NULL)
	{
		if(fileName[level_Max][strlen(fileName[level_Max])-1]=='\n')
		{
			fileName[level_Max][strlen(fileName[level_Max])-1]=0;
		}
		level_Max++;
	}
	
	fclose(fMap);
	
	level_Now=0;	//默认第一个地图
	
	do
	{
        ch = _getch();
        switch (ch)
		{
        case 's': case 'S': case '2': case 80:  // 下
			if(index==0)
			{
                index = 1;
                SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                gotoxy(16, 14);
                printf("%2s%s%2s", "", modeItem[0], "");
                SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);
                gotoxy(16, 16);
                printf("%2s%s%2s", "", modeItem[1], "");
            }
            else if(index == 1)
			{
                index = 2;
                SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                gotoxy(16, 16);
                printf("%2s%s%2s", "", modeItem[1], "");
                SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);
                gotoxy(16, 18);
                printf("%2s%s%2s", "", modeItem[2], "");
            }
			else if(index==2)
			{
				index = 3;
				SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);
				gotoxy(16, 20);
				printf("%2s%s%2s", "", modeItem[3], "");
				SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
				gotoxy(16, 18);
				printf("%2s%s%2s", "", modeItem[2], "");
			}
			else if(index==3)
			{
				index = 0;
				SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);
				gotoxy(16, 14);
				printf("%2s%s%2s", "", modeItem[0], "");
				SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
				gotoxy(16, 20);
				printf("%2s%s%2s", "", modeItem[3], "");
			}
			break;

		case 'w': case 'W': case '8': case 72:  // 上
			if(index==0)
			{
                index = 3;
                SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                gotoxy(16, 14);
                printf("%2s%s%2s", "", modeItem[0], "");
                SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);
                gotoxy(16, 20);
                printf("%2s%s%2s", "", modeItem[3], "");
            }
            else if(index == 1)
			{
                index = 0;
                SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);
                gotoxy(16, 14);
                printf("%2s%s%2s", "", modeItem[0], "");
                SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
                gotoxy(16, 16);
                printf("%2s%s%2s", "", modeItem[1], "");
            }
			else if(index==2)
			{
				index = 1;
				SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);
				gotoxy(16, 16);
				printf("%2s%s%2s", "", modeItem[1], "");
				SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
				gotoxy(16, 18);
				printf("%2s%s%2s", "", modeItem[2], "");
			}
			else if(index==3)
			{
				index = 2;
				SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);
				gotoxy(16, 18);
				printf("%2s%s%2s", "", modeItem[2], "");
				SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
				gotoxy(16, 20);
				printf("%2s%s%2s", "", modeItem[3], "");
			}
            break;

        case ' ': case 13:
            return index;
			break;
        }

    }while(1);
	
}
//=============================================================================
// 以全角定位到某点
void gotoxy(int x, int y)
{
    static COORD cd;
	
    cd.X = (int)(x << 1);
    cd.Y = y;
    SetConsoleCursorPosition(g_hConsoleOutput, cd);
}

//=============================================================================
// 运行主程序
void playGame()	//游戏完成时
{
	char ch;

	do 
	{
		runGame(fileName[level_Now]);

		if(level_Now<level_Max && checkComplete())
		{
			level_Now++;
			gotoxy(LEFT+2,BUTTON-3);
			printf("恭喜你已经成功过关！！！");
			gotoxy(LEFT+2,BUTTON-2);
			printf("是否继续游戏? < 按ENTER键继续，按任意键返回主菜单>");
		}
		else
		{
			break;
		}
	} while((ch=_getch())==13);
	

}
//=============================================================================
// 开始游戏
void runGame()
{
	int ch;
	clock_t clockLast, clockNow;	//游戏主进程
	clock_t treasureLast,treasureNow;	//宝物下落过程
	
    clockLast = treasureLast =clock();  // 计时
	
	initGame(run_Game);

	PrintLev(); // 绘制游戏方块
	
	while(live)
	{
		while(!stop && live)
		{
			while (_kbhit())  // 有键按下
			{
				ch = _getch();
				switch(ch)
				{
				case 27: 
					return;
					break;
				case 'a':case 'A':case '4': case 75:
					MoveBord(1);
					break;
				case 'd': case 'D': case '6': case 77:
					MoveBord(2);
					break;
				case 32:
					stop=1;
				}
			}
			
			clockNow = clock();  // 计时
			// 两次记时的间隔超过0.45秒
			if (clockNow - clockLast > 0.2F *speed)
			{
				clockLast = clockNow;
				MoveBall(0);
			}

			if(intTreasure==1)
			{
				treasureNow=clock();
				if (treasureNow - treasureLast > 0.5F * CLOCKS_PER_SEC)
				{
					treasureLast = treasureNow;
					move_Treasure();
				}
			}
			
			if(getTreasure==1)
			{
				treaTime_Now=clock();
				if(treaTime_Now-treaTime_Last > 1.0F * CLOCKS_PER_SEC)
				{
					if(treaTime_Now-treaTime_Last < 1.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+27,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 2.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+26,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 3.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+25,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 4.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+24,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 5.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+23,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 6.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+22,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 7.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+21,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 8.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+20,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 9.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+19,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 10.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+18,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 11.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+17,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 12.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+16,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 13.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+15,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 14.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+14,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 15.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+13,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 16.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+12,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 17.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+11,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 18.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+10,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 19.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+9,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 20.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+8,BUTTON-2);
						printf("%2s","");
					}
					else if(treaTime_Now-treaTime_Last < 21.1F * CLOCKS_PER_SEC)
					{
						SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
						gotoxy(LEFT+2,BUTTON-2);
						printf("%50s","");
						time_Treasure();
						getTreasure=0;
					}
				}
			}
			if(checkComplete()) 
			{
				return;
			}
		}

		if(live)
		{
			if ((ch=_getch())==32) stop=0;
		}
	}
	
}

//=============================================================================
// 显示帮助界面
void helpGame()
{
	SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
	system("cls");
	
	SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
	gotoxy(LEFT,2);
	printf("====================物品详解=====================");
	
	SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_INTENSITY);
	gotoxy(LEFT,4);
	printf("■:一级砖块：可以很轻易的把它砸碎");
	
	SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_BLUE|FOREGROUND_INTENSITY);
	gotoxy(LEFT,6);
	printf("■:二级砖块：可能需要一点力气来搞定它");
	
	SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_RED|FOREGROUND_INTENSITY);
	gotoxy(LEFT,8);
	printf("■:三级砖块：这家伙很硬的，你得小心应对");
	
	SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
	gotoxy(LEFT,10);
	printf("★:紫金宝石：它可以使小球充满力量，横冲直撞!");
	
	SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
	gotoxy(LEFT,12);
	printf("▓:冰霜宝石：小球被冰霜笼罩，速度也会随之变慢~");
	
	SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
	gotoxy(LEFT,14);
	printf("◆:贪婪宝石：你的木板将会吸收天地气息来扩张它的领地！");
	
	SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_RED|FOREGROUND_INTENSITY);
	gotoxy(LEFT,16);
	printf("◆:恶魔宝石：你的木板会被恶魔吃掉一部分，后果你懂得~");
	
	SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
	gotoxy(LEFT,18);
	printf("▼:风之宝石：风之力将赋予小球闪电般的移动速度~");
	
	SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_INTENSITY);
	gotoxy(LEFT,20);
	printf("★:疯狂宝石：接下来就是观看表演的时间啦!");

	system("pause>nul");
}

//=================================================================================
//选择关卡
void selectMap()
{
	FILE *fMap;
	char ch;

	level_Max=0;

	SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
	system("cls");

	if(fopen_s(&fMap, "maps\\MapInfo.dat", "r"))
	{
		gotoxy(LEFT+4,TOP+4);
		printf("打开地图信息文件发生错误!");
		gotoxy(LEFT+4,TOP+6);
		printf("请查看MapInfo.dat文件是否存在!!!");
		exit(0);
	}

	while(fgets(fileName[level_Max],20,fMap)!=NULL)
	{
		if(fileName[level_Max][strlen(fileName[level_Max])-1]=='\n')
		{
			fileName[level_Max][strlen(fileName[level_Max])-1]=0;
		}
		level_Max++;
	}

	fclose(fMap);

	SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
	for (level_Now=0;level_Now<level_Max;level_Now++)
	{
		gotoxy(LEFT+8,TOP+2*level_Now);
		printf("%s",fileName[level_Now]);
	}

	level_Now=0;	//默认第一个地图
	SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_INTENSITY);
	gotoxy(LEFT+18,TOP-1);
	printf("地图列表");

	SetConsoleTextAttribute(g_hConsoleOutput,0xF0);
	gotoxy(LEFT+8,TOP);
	printf("%s",fileName[level_Now]);

	do 
	{
		ch=_getch();

		switch(ch)
		{
			case 'w': case 'W': case '8': case 72:  // 上
				if(level_Now>0)
				{
					SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
					gotoxy(LEFT+8,TOP+2*level_Now);
					printf("%s",fileName[level_Now]);
					
					SetConsoleTextAttribute(g_hConsoleOutput,0xF0);
					gotoxy(LEFT+8,TOP+2*(level_Now-1));
					printf("%s",fileName[level_Now-1]);

					level_Now--;

				}

				break;

			case 's': case 'S': case '2': case 80:  // 下
				if(level_Now<level_Max-1)
				{
					SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
					gotoxy(LEFT+8,TOP+2*level_Now);
					printf("%s",fileName[level_Now]);
					
					SetConsoleTextAttribute(g_hConsoleOutput,0xF0);
					gotoxy(LEFT+8,TOP+2*(level_Now+1));
					printf("%s",fileName[level_Now+1]);

					level_Now++;
					
				}

				break;

			case ' ':case 13:
				return;
				break;
			case 27:
				level_Now=0;
				return;
				break;	

		}

	} while(1);

}
//=================================================================================
//制作关卡
void creatMap()
{
	int i,j;
	char ch;
	Point pot_map;
	char map_Name[20]="maps//";
	char temp_Name[20];
	
	FILE *fp,*fMap;

	CONSOLE_CURSOR_INFO cursorInfo = { 1, TRUE };
	SetConsoleCursorInfo(g_hConsoleOutput, &cursorInfo);

	initGame(creat_Map);

	pot_map.x=LEFT+1;
	pot_map.y=TOP+1;
	gotoxy(pot_map.x,pot_map.y);
	
	do{
		switch(ch=_getch())
		{
		case 'w':case 'W':case 72:	//上
			if (pot_map.y>TOP+1)
			{
				pot_map.y-=1;
				gotoxy(pot_map.x,pot_map.y);
			}
			break;

		case 's':case 'S':case 80:	//下
			if(pot_map.y<BUTTON-10)
			{
				pot_map.y+=1;
				gotoxy(pot_map.x,pot_map.y);
			}
			break;

		case 'a':case 'A':case 75:	//左
			if(pot_map.x>LEFT+1)
			{
				pot_map.x-=1;
				gotoxy(pot_map.x,pot_map.y);
			}
			break;

		case 'd':case 'D':case 77:	//右
			if(pot_map.x<RIGHT-1)
			{
				pot_map.x+=1;
				gotoxy(pot_map.x,pot_map.y);
			}
			break;

		case '0':
			SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
			gotoxy(pot_map.x,pot_map.y);
			printf("%2s","");
			potVal[pot_map.x][pot_map.y]=0;
			break;

		case '1':
			SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
			gotoxy(pot_map.x,pot_map.y);
			printf("■");
			potVal[pot_map.x][pot_map.y]=1;
			break;

		case '2':
			SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE|FOREGROUND_INTENSITY);
			gotoxy(pot_map.x,pot_map.y);
			printf("■");
			potVal[pot_map.x][pot_map.y]=2;
			break;

		case '3':
			SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED|FOREGROUND_INTENSITY);
			gotoxy(pot_map.x,pot_map.y);
			printf("■");
			potVal[pot_map.x][pot_map.y]=3;
			break;

		case 32:	//空格
			if (potVal[pot_map.x][pot_map.y]==0)
			{
				SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
				gotoxy(pot_map.x,pot_map.y);
				printf("■");
				potVal[pot_map.x][pot_map.y]=1;
			}
			else if(potVal[pot_map.x][pot_map.y]==1)
			{
				SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE|FOREGROUND_INTENSITY);
				gotoxy(pot_map.x,pot_map.y);
				printf("■");
				potVal[pot_map.x][pot_map.y]=2;
			}
			else if(potVal[pot_map.x][pot_map.y]==2)
			{
				SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED|FOREGROUND_INTENSITY);
				gotoxy(pot_map.x,pot_map.y);
				printf("■");
				potVal[pot_map.x][pot_map.y]=3;
			}
			else if(potVal[pot_map.x][pot_map.y]==3)
			{
				SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
				gotoxy(pot_map.x,pot_map.y);
				printf("%2s","");
				potVal[pot_map.x][pot_map.y]=0;
			}
			break;

		case 27:
			return;
			break;
		}

		if(ch==13)
		{
			gotoxy(LEFT+2,BUTTON-5);
			printf("你确定保存好了吗？< 按ENTER键确认，按任意键继续编辑>");
			_getch();
			ch=_getch();
			if(ch==13){
				gotoxy(LEFT+5,BUTTON-3);
				printf("给你的地图取一个牛掰的名字吧！-->  ");
				scanf_s("%s", temp_Name, sizeof(temp_Name));
				gotoxy(LEFT+5,BUTTON-2);
				printf("正在保存...");
				Sleep(100);
				break;
			}
			else
			{
				gotoxy(LEFT+2,BUTTON-5);
				printf("%53s","");
			}
		}

		gotoxy(pot_map.x,pot_map.y);

	}while(1);


	strcat_s(map_Name, sizeof(map_Name), temp_Name);
	strcat_s(map_Name, sizeof(map_Name), ".map");

	if(fopen_s(&fp, map_Name, "wb") || fopen_s(&fMap, "maps\\MapInfo.dat","a"))
	{
		gotoxy(LEFT+12,BUTTON-3);
		printf("保存失败!");
		Sleep(1000);
		exit(0);
	}
	for(i=TOP+1;i<BUTTON-9;i++)
	{
		for(j=LEFT+1;j<RIGHT;j++)
		{
			fwrite(&potVal[j][i],sizeof(short),1,fp);
		}
	}

	strcat_s(temp_Name, sizeof(temp_Name), "\n");
	fputs(temp_Name,fMap);

	fclose(fMap);
	fclose(fp);

	gotoxy(LEFT+12,BUTTON-2);
	printf("保存成功!");
	Sleep(1000);

}

//============================================================================
// 显示关卡界面
void PrintLev()
{
	int i,j;

	FILE *fp;	//定义读关卡文件的指针
	short file_temp;	//临时读出的关卡文件
	char url_Map[30]="maps//";	//地图路径

	strcat_s(url_Map, sizeof(url_Map), fileName[level_Now]);
	strcat_s(url_Map, sizeof(url_Map), ".map");
	if(fopen_s(&fp, url_Map, "rb"))
	{
		printf("关卡文件不存在或不能打开关卡文件!");
		exit(0);
	}

	gotoxy(LEFT+2,TOP+2);

	for(i=TOP+1;i<BUTTON-9;i++)
	{
		for(j=LEFT+1;j<RIGHT;j++)
		{
			gotoxy(j,i);

			fread(&file_temp,sizeof(short),1,fp);

			switch(file_temp)
			{
			case 1:
				SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
				potVal[j][i]=1;
				printf("■");
				break;
			case 2:
				SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE|FOREGROUND_INTENSITY);
				potVal[j][i]=2;
				printf("■");
				break;
			case 3:
				SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_RED|FOREGROUND_INTENSITY);
				potVal[j][i]=3;
				printf("■");
				break;
			default:
				potVal[j][i]=0;
				break;
			}

			Sleep(10);
		}
	}

	fclose(fp);	//擦屁股

	PrintBord(); //绘制木板

}

//=================================================================================
//绘制木板和小球
void PrintBord()
{
	//绘制小球
	SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
	Ball[0].x=19;
	Ball[0].y=18;
	gotoxy(Ball[0].x,Ball[0].y);
	printf("%s",BALL_SHAPE[0]);
	
	//绘制木板
	SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
	Bord.x=18;
	Bord.y=19;
	gotoxy(Bord.x,Bord.y);
	potVal[Bord.x][Bord.y]=potVal[Bord.x+1][Bord.y]=potVal[Bord.x+2][Bord.y]=4;
	printf("▔▔▔");
	
}

//=================================================================================
//移动木板
void MoveBord(int orientation)
{
	if(orientation==1)
	{
		if(Bord.length==1)
		{
			if(Bord.x>LEFT+1)
			{
				Bord.x=Bord.x-1;
				SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
				gotoxy(Bord.x,Bord.y);
				printf("▔  ");
				potVal[Bord.x][Bord.y]=4;
				potVal[Bord.x+1][Bord.y]=0;
			}
		}
		else if(Bord.length==3)
		{
			if(Bord.x>LEFT+1)
			{
				Bord.x=Bord.x-1;
				SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
				gotoxy(Bord.x,Bord.y);
				printf("▔▔▔  ");
				potVal[Bord.x][Bord.y]=4;
				potVal[Bord.x+3][Bord.y]=0;
			}
		}
		else if(Bord.length==5)
		{
			if(Bord.x>LEFT+1)
			{
				Bord.x=Bord.x-1;
				SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
				gotoxy(Bord.x,Bord.y);
				printf("▔▔▔▔▔  ");
				potVal[Bord.x][Bord.y]=4;
				potVal[Bord.x+5][Bord.y]=0;
			}
		}
	}
	else
	{
		if(Bord.length==1)
		{
			if((Bord.x+1)<RIGHT)
			{
				SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
				gotoxy(Bord.x,Bord.y);
				printf("  ▔");
				Bord.x=Bord.x+1;
				potVal[Bord.x][Bord.y]=4;
				potVal[Bord.x-1][Bord.y]=0;
			}
		}
		else if(Bord.length==3)
		{
			if((Bord.x+3)<RIGHT)
			{
				SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
				gotoxy(Bord.x,Bord.y);
				printf("  ▔▔▔");
				Bord.x=Bord.x+1;
				potVal[Bord.x+2][Bord.y]=4;
				potVal[Bord.x-1][Bord.y]=0;
			}
		}
		else if(Bord.length==5)
		{
			if((Bord.x+5)<RIGHT)
			{
				SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
				gotoxy(Bord.x,Bord.y);
				printf("  ▔▔▔▔▔");
				Bord.x=Bord.x+1;
				potVal[Bord.x+4][Bord.y]=4;
				potVal[Bord.x-1][Bord.y]=0;
			}
		}
	}
}

//=================================================================================
//移动小球
void MoveBall(int index)
{
//右上
	if(Ball[index].x_Speed==1&&Ball[index].y_Speed==1)
	{
		if(potVal[Ball[index].x][Ball[index].y-1]==4)
		{
			Ball[index].y_Speed=-1;
			PlaySound("music\\knock_1.wav",NULL,SND_FILENAME|SND_ASYNC);
		}
		else if(potVal[Ball[index].x+1][Ball[index].y]==4)
		{
			Ball[index].x_Speed=-1;
			PlaySound("music\\knock_1.wav",NULL,SND_FILENAME|SND_ASYNC);
		}
		else if(potVal[Ball[index].x][Ball[index].y-1]!=0)
		{
			removeDiamond(index,1);
		}
		else if(potVal[Ball[index].x+1][Ball[index].y]!=0)
		{
			removeDiamond(index,3);
		}
		else if(potVal[Ball[index].x+1][Ball[index].y-1]!=0)
		{
			removeDiamond(index,2);
		}
		else
		{
			if(Ball[index].mode==0)
			{
				SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%2s","");
				Ball[index].x=Ball[index].x+1;
				Ball[index].y=Ball[index].y-1;
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%s",BALL_SHAPE[0]);
			}
			else if(Ball[index].mode==1)
			{
				
				gotoxy(Ball[index].x,Ball[index].y);
				if(potVal[Ball[index].x][Ball[index].y]==1)
				{
					SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_BLUE|FOREGROUND_INTENSITY);
					printf("■");
				}
				else if(potVal[Ball[index].x][Ball[index].y]==0)
				{
					SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_BLUE|FOREGROUND_INTENSITY);
					printf("%2s","");
				}
				
				Ball[index].x=Ball[index].x+1;
				Ball[index].y=Ball[index].y-1;
				
				SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_BLUE|FOREGROUND_RED|FOREGROUND_INTENSITY);
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%s",BALL_SHAPE[1]);
			}
			else
			{
				SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%2s","");
				Ball[index].x=Ball[index].x+1;
				Ball[index].y=Ball[index].y-1;
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%s",BALL_SHAPE[2]);
			}
		}

		
	}

//右下
	if(Ball[index].x_Speed==1&&Ball[index].y_Speed==-1)
	{
		
		if(potVal[Ball[index].x][Ball[index].y+1]==4)
		{
			Ball[index].y_Speed=1;
			PlaySound("music\\knock_1.wav",NULL,SND_FILENAME|SND_ASYNC);
		}
		else if(potVal[Ball[index].x+1][Ball[index].y]==4)
		{
			Ball[index].x_Speed=-1;
			PlaySound("music\\knock_1.wav",NULL,SND_FILENAME|SND_ASYNC);
		}
		else if(potVal[Ball[index].x+1][Ball[index].y+1]==4)
		{
			Ball[index].x_Speed=-1;
			Ball[index].y_Speed=1;
			PlaySound("music\\knock_1.wav",NULL,SND_FILENAME|SND_ASYNC);
		}
		else if(potVal[Ball[index].x+1][Ball[index].y]!=0)
		{
			removeDiamond(index,3);
		}
		else if(potVal[Ball[index].x][Ball[index].y+1]!=0)
		{
			removeDiamond(index,5);
		}
		else if(potVal[Ball[index].x+1][Ball[index].y+1]!=0)
		{
			removeDiamond(index,4);
		}
		else
		{
			if(Ball[index].mode==0)
			{
				SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%2s","");
				Ball[index].x=Ball[index].x+1;
				Ball[index].y=Ball[index].y+1;
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%s",BALL_SHAPE[0]);
			}
			else if(Ball[index].mode==1)
			{
				
				gotoxy(Ball[index].x,Ball[index].y);
				if(potVal[Ball[index].x][Ball[index].y]==1)
				{
					SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_BLUE|FOREGROUND_INTENSITY);
					printf("■");
				}
				else if(potVal[Ball[index].x][Ball[index].y]==0)
				{
					SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_BLUE|FOREGROUND_INTENSITY);
					printf("%2s","");
				}
				
				Ball[index].x=Ball[index].x+1;
				Ball[index].y=Ball[index].y+1;
				
				SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_BLUE|FOREGROUND_RED|FOREGROUND_INTENSITY);
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%s",BALL_SHAPE[1]);
			}
			else
			{
				SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%2s","");
				Ball[index].x=Ball[index].x+1;
				Ball[index].y=Ball[index].y+1;
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%s",BALL_SHAPE[2]);
			}
		}
	}

//左下
	if(Ball[index].x_Speed==-1&&Ball[index].y_Speed==-1)
	{
		if(potVal[Ball[index].x][Ball[index].y+1]==4)
		{
			Ball[index].y_Speed=1;
			PlaySound("music\\knock_1.wav",NULL,SND_FILENAME|SND_ASYNC);
		}
		else if(potVal[Ball[index].x-1][Ball[index].y]==4)
		{
			Ball[index].x_Speed=1;
			PlaySound("music\\knock_1.wav",NULL,SND_FILENAME|SND_ASYNC);
		}
		else if(potVal[Ball[index].x-1][Ball[index].y+1]==4)
		{
			Ball[index].x_Speed=1;
			Ball[index].y_Speed=1;
			PlaySound("music\\knock_1.wav",NULL,SND_FILENAME|SND_ASYNC);
		}
		else if(potVal[Ball[index].x][Ball[index].y+1]!=0)
		{
			removeDiamond(index,5);
		}
		else if(potVal[Ball[index].x-1][Ball[index].y]!=0)
		{
			removeDiamond(index,7);
		}
		else if(potVal[Ball[index].x-1][Ball[index].y+1]!=0)
		{
			removeDiamond(index,6);
		}
		else
		{
			if(Ball[index].mode==0)
			{
				SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%2s","");
				Ball[index].x=Ball[index].x-1;
				Ball[index].y=Ball[index].y+1;
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%s",BALL_SHAPE[0]);
			}
			else if(Ball[index].mode==1)
			{
				
				gotoxy(Ball[index].x,Ball[index].y);
				if(potVal[Ball[index].x][Ball[index].y]==1)
				{
					SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_BLUE|FOREGROUND_INTENSITY);
					printf("■");
				}
				else if(potVal[Ball[index].x][Ball[index].y]==0)
				{
					SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_BLUE|FOREGROUND_INTENSITY);
					printf("%2s","");
				}
				
				Ball[index].x=Ball[index].x-1;
				Ball[index].y=Ball[index].y+1;
				
				SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_BLUE|FOREGROUND_RED|FOREGROUND_INTENSITY);
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%s",BALL_SHAPE[1]);
			}
			else
			{
				SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%2s","");
				Ball[index].x=Ball[index].x-1;
				Ball[index].y=Ball[index].y+1;
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%s",BALL_SHAPE[2]);
			}
		} 
	}

//左上
	if(Ball[index].x_Speed==-1&&Ball[index].y_Speed==1)
	{
		if(potVal[Ball[index].x][Ball[index].y-1]==4)
		{
			Ball[index].y_Speed=-1;
			PlaySound("music\\knock_1.wav",NULL,SND_FILENAME|SND_ASYNC);
		}
		else if(potVal[Ball[index].x-1][Ball[index].y]==4)
		{
			Ball[index].x_Speed=1;
			PlaySound("music\\knock_1.wav",NULL,SND_FILENAME|SND_ASYNC);
		}
		else if(potVal[Ball[index].x][Ball[index].y-1]!=0)
		{
			removeDiamond(index,1);
		}
		else if(potVal[Ball[index].x-1][Ball[index].y]!=0)
		{
			removeDiamond(index,7);
		}
		else if(potVal[Ball[index].x-1][Ball[index].y-1]!=0)
		{
			removeDiamond(index,8);
		}
		else 
		{
			if(Ball[index].mode==0)
			{
				SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%2s","");
				Ball[index].x=Ball[index].x-1;
				Ball[index].y=Ball[index].y-1;
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%s",BALL_SHAPE[0]);
			}
			else if(Ball[index].mode==1)
			{
				
				gotoxy(Ball[index].x,Ball[index].y);
				if(potVal[Ball[index].x][Ball[index].y]==1)
				{
					SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_BLUE|FOREGROUND_INTENSITY);
					printf("■");
				}
				else if(potVal[Ball[index].x][Ball[index].y]==0)
				{
					SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_BLUE|FOREGROUND_INTENSITY);
					printf("%2s","");
				}
				
				Ball[index].x=Ball[index].x-1;
				Ball[index].y=Ball[index].y-1;
				
				SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_BLUE|FOREGROUND_RED|FOREGROUND_INTENSITY);
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%s",BALL_SHAPE[1]);
			}
			else
			{
				SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%2s","");
				Ball[index].x=Ball[index].x-1;
				Ball[index].y=Ball[index].y-1;
				gotoxy(Ball[index].x,Ball[index].y);
				printf("%s",BALL_SHAPE[2]);
			}
		}
	}
	
	if(Ball[index].y>BUTTON) live=0;
}

//======================方块处理部分===================================
//消除方块
void removeDiamond(int index,int direction)
{
	Point point_diamond;

	switch(direction)
	{
	case 1:	//上

		if(Ball[index].mode!=1)
		{
			Ball[index].y_Speed=-1;
		}

		point_diamond.x=Ball[index].x;
		point_diamond.y=Ball[index].y-1;

		break;
		
	case 2:	//右上
		if(Ball[index].mode!=1)
		{
			Ball[index].x_Speed=-1;
			Ball[index].y_Speed=-1;
		}

		point_diamond.x=Ball[index].x+1;
		point_diamond.y=Ball[index].y-1;

		break;
		
	case 3:	//右
		if(Ball[index].mode!=1)
		{
			Ball[index].x_Speed=-1;
		}

		point_diamond.x=Ball[index].x+1;
		point_diamond.y=Ball[index].y;

		break;
		
	case 4:	//右下
		if(Ball[index].mode!=1)
		{
			Ball[index].x_Speed=-1;
			Ball[index].y_Speed=1;
		}

		point_diamond.x=Ball[index].x+1;
		point_diamond.y=Ball[index].y+1;

		break;

	case 5:	//下
		if(Ball[index].mode!=1)
		{
			Ball[index].y_Speed=1;
		}

		point_diamond.x=Ball[index].x;
		point_diamond.y=Ball[index].y+1;

		break;

	case 6:	//左下
		if(Ball[index].mode!=1)
		{
			Ball[index].x_Speed=1;
			Ball[index].y_Speed=1;
		}

		point_diamond.x=Ball[index].x-1;
		point_diamond.y=Ball[index].y+1;

		break;
		
	case 7:	//左
		if(Ball[index].mode!=1)
		{
			Ball[index].x_Speed=1;
		}

		point_diamond.x=Ball[index].x-1;
		point_diamond.y=Ball[index].y;

		break;
		
	case 8:	//左上
		if(Ball[index].mode!=1)
		{
			Ball[index].x_Speed=1;
			Ball[index].y_Speed=-1;
		}

		point_diamond.x=Ball[index].x-1;
		point_diamond.y=Ball[index].y-1;

		break;

	}

	if(potVal[point_diamond.x][point_diamond.y]==3)
	{
		removeColor(red,point_diamond);
	}
	else if(potVal[point_diamond.x][point_diamond.y]==2)
	{
		removeColor(blue,point_diamond);
	}
	else if(potVal[point_diamond.x][point_diamond.y]==1)
	{
		removeColor(green,point_diamond);
	}

}

//解决各种颜色的方块
void removeColor(enum color color_diamond,Point point_diamond)
{
	PlaySound("music\\knock_2.wav",NULL,SND_FILENAME|SND_ASYNC);

	switch(color_diamond)
	{
	case green:
		SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
		gotoxy(point_diamond.x,point_diamond.y);
		potVal[point_diamond.x][point_diamond.y]=0;
		printf("%2s","");
		break;

	case blue:
		SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
		gotoxy(point_diamond.x,point_diamond.y);
		potVal[point_diamond.x][point_diamond.y]=1;
		printf("■");
		break;

	case red:
		SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_BLUE|FOREGROUND_INTENSITY);
		gotoxy(point_diamond.x,point_diamond.y);
		potVal[point_diamond.x][point_diamond.y]=2;
		printf("■");
		break;
	}

	//计分
	intGrade+=10;
	SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
	gotoxy(RIGHT+5,2);
	printf("%d",intGrade);

	srand((int)time(NULL));
	if(rand()%100<60 && intTreasure==0 && getTreasure==0)	//30%的概率出现宝物
	{
		showTreasure(rand()%100,point_diamond);
	}
}

//=======================宝石处理部分===================================
//随机宝物出现
void showTreasure(int intRand,Point point_diamond)
{
	int y=Bord.y;
	intTreasure=1;

	if (intRand<15)	//实心球
	{
		Treasure.type=1;
		Treasure.x=point_diamond.x;
	}
	else if(intRand<25)	//冰球（速度变慢）
	{
		Treasure.type=2;
		Treasure.x=point_diamond.x;
		
	}
	else if(intRand<40)	//板加长
	{
		Treasure.type=3;
		Treasure.x=point_diamond.x;
		
	}
	else if(intRand<65)	//板缩短
	{
		Treasure.type=4;
		Treasure.x=point_diamond.x;
		
	}
	else if(intRand<75)	//速度变快
	{
		Treasure.type=5;
		Treasure.x=point_diamond.x;
		
	}
	else	//疯狂时间
	{
		Treasure.type=6;
		Treasure.x=point_diamond.x;
		
	}

	while(potVal[point_diamond.x][y-1]==0 && y>point_diamond.y) y--;
	Treasure.y=y;

}

//随机宝物出现
void move_Treasure()	
{
	gotoxy(Treasure.x,Treasure.y);
	printf("%2s","");

	if(Treasure.y+1==Bord.y && potVal[Treasure.x][Treasure.y+1]==4)
	{
		get_Treasure();
		intTreasure=0;
		return;
	}
	else if(Treasure.y+1<BUTTON)
	{
		Treasure.y++;
	}
	else
	{
		intTreasure=0;
		return;
	}

	gotoxy(Treasure.x,Treasure.y);

	switch(Treasure.type)
	{
	case 1:	//穿透
		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
		printf("★");
		break;

	case 2:	//减速
		SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
		printf("▓");
		break;

	case 3:	//伸长
		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
		printf("◆");
		break;

	case 4:	//缩短
		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_RED|FOREGROUND_INTENSITY);
		printf("◆");
		break;

	case 5:	//加速
		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
		printf("▼");
		break;

	case 6:	//疯狂
		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_INTENSITY);
		printf("★");
		break;

	}

}

//获得宝石
void get_Treasure()	
{
	int i;

	switch(Treasure.type)
	{
	case 1:	//穿透

		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
		gotoxy(LEFT+2,BUTTON-2);
		printf("★");
		SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
		gotoxy(LEFT+4,BUTTON-2);
		printf("Time: ■■■■■■■■■■■■■■■■■■■■");

		speed=1000;
		Ball[0].mode=1;

		break;
		
	case 2:	//减速
		
		SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
		gotoxy(LEFT+2,BUTTON-2);
		printf("▓");
		SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
		gotoxy(LEFT+4,BUTTON-2);
		printf("Time: ■■■■■■■■■■■■■■■■■■■■");

		speed=1600;
		Ball[0].mode=2;

		break;
		
	case 3:	//伸长

		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
		gotoxy(LEFT+2,BUTTON-2);
		printf("◆");
		SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
		gotoxy(LEFT+4,BUTTON-2);
		printf("Time: ■■■■■■■■■■■■■■■■■■■■");

		Bord.length=5;
		potVal[Bord.x][Bord.y]=potVal[Bord.x+1][Bord.y]=potVal[Bord.x+2][Bord.y]=0;

		if(Bord.x+5>=RIGHT)	//防止木板超过右边界
		{
			Bord.x=RIGHT-5;
		}
		SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
		gotoxy(Bord.x,Bord.y);
		printf("▔▔▔▔▔");
		potVal[Bord.x][Bord.y]=potVal[Bord.x+1][Bord.y]=potVal[Bord.x+2][Bord.y]=potVal[Bord.x+3][Bord.y]=potVal[Bord.x+4][Bord.y]=4;

		break;
		
	case 4:	//缩短
		
		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_RED|FOREGROUND_INTENSITY);
		gotoxy(LEFT+2,BUTTON-2);
		printf("◆");
		SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
		gotoxy(LEFT+4,BUTTON-2);
		printf("Time: ■■■■■■■■■■■■■■■■■■■■");

		Bord.length=1;
		potVal[Bord.x+1][Bord.y]=potVal[Bord.x+2][Bord.y]=0;
		
		SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
		gotoxy(Bord.x,Bord.y);
		printf("▔%4s","");

		break;
		
	case 5:	//加速
		
		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
		gotoxy(LEFT+2,BUTTON-2);
		printf("▼");
		SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
		gotoxy(LEFT+4,BUTTON-2);
		printf("Time: ■■■■■■■■■■■■■■■■■■■■");
		
		speed=500;
		break;
		
	case 6:	//疯狂

		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_INTENSITY);
		gotoxy(LEFT+2,BUTTON-2);
		printf("★");
		SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
		gotoxy(LEFT+4,BUTTON-2);
		printf("Time: ■■■■■■■■■■■■■■■■■■■■");

		speed=200;
		for(i=LEFT+1;i<=RIGHT-1;i++)
		{
			SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
			gotoxy(i,BUTTON-4);
			printf("■");
			potVal[i][BUTTON-4]=4;
			Sleep(10);
		}
		
		break;
	}

	getTreasure=1;	//获得宝石
	treaTime_Last =clock();
}

//宝石有用时间
void time_Treasure()
{
	int i;

	switch(Treasure.type)
	{
	case 1:	//穿透
		Ball[0].mode=0;
		break;
		
	case 2:	//减速
		speed=1000;
		Ball[0].mode=0;
		break;
		
	case 3:	//伸长

		Bord.length=3;
		potVal[Bord.x+3][Bord.y]=potVal[Bord.x+4][Bord.y]=0;
		
		SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
		gotoxy(Bord.x,Bord.y);
		printf("▔▔▔%4s","");
		
		break;
		
	case 4:	//缩短

		Bord.length=3;
		if(Bord.x+3>=RIGHT)	//防止木板超过右边界
		{
			Bord.x=RIGHT-3;
		}
		potVal[Bord.x][Bord.y]=potVal[Bord.x+1][Bord.y]=potVal[Bord.x+2][Bord.y]=4;
		
		SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
		gotoxy(Bord.x,Bord.y);
		printf("▔▔▔");
		
		break;
		
	case 5:	//加速
		speed=1000;
		break;
		
	case 6:	//疯狂

		speed=1000;
		for(i=LEFT+1;i<=RIGHT-1;i++)
		{
			SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
			gotoxy(i,BUTTON-4);
			printf("%2s","");
			potVal[i][BUTTON-4]=0;
			Sleep(10);
		}
		
		break;
		
	}

}
//==================================================================
//初始化游戏中的各种值
void initGame(enum mode game_Mode)
{
	int i,j;

	for(i=LEFT+1;i<RIGHT;i++)
	{
		for (j=TOP+1;j<BUTTON;j++)
		{
			potVal[i][j]=0;
		}
	}

	SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
	system("cls");
	SetConsoleTextAttribute(g_hConsoleOutput, 0xF0);
	
	for(i=LEFT;i<=RIGHT;i++)
	{
		gotoxy(i,TOP);
		printf("%2s","");
		potVal[i][TOP]=4;
	}
	
	for(i=TOP;i<BUTTON+3;i++)
	{
		gotoxy(LEFT,i);
		printf("%2s","");
		potVal[LEFT][i]=4;
		
		gotoxy(RIGHT,i);
		printf("%2s","");
		potVal[RIGHT][i]=4;
	}
	
	SetConsoleTextAttribute(g_hConsoleOutput, 0x0F);
	
	//完善边界
	for(i=TOP+1;i<BUTTON+3;i++)
	{
		gotoxy(LEFT+1,i);
		printf("%2s","");

		gotoxy(RIGHT-1,i);
		printf("%2s","");
	}

	switch(game_Mode)
	{
	case run_Game:
		live=1;		//有生命
		stop=0;		//0->不暂停，1->暂停
		intGrade=0;
		speed=1000;
		intTreasure=0;	//宝石数
		getTreasure=0;	//获得宝石

		Ball[0].mode=0;
		Ball[0].x_Speed=1;
		Ball[0].y_Speed=1;
		Bord.length=3;

		SetConsoleTextAttribute(g_hConsoleOutput, FOREGROUND_GREEN|FOREGROUND_INTENSITY);
		gotoxy(RIGHT+2,2);
		printf("成绩: 0");

		SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
		gotoxy(RIGHT+2,4);
		printf("===物品简介===");

		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_INTENSITY);
		gotoxy(RIGHT+2,6);
		printf("■:一级砖块");

		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_BLUE|FOREGROUND_INTENSITY);
		gotoxy(RIGHT+2,8);
		printf("■:二级砖块");

		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_RED|FOREGROUND_INTENSITY);
		gotoxy(RIGHT+2,10);
		printf("■:三级砖块");

		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
		gotoxy(RIGHT+2,12);
		printf("★:紫金[穿透]");

		SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
		gotoxy(RIGHT+2,14);
		printf("▓:冰霜[减速]");

		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
		gotoxy(RIGHT+2,16);
		printf("◆:贪婪[伸长]");

		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_RED|FOREGROUND_INTENSITY);
		gotoxy(RIGHT+2,18);
		printf("◆:恶魔[缩短]");

		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
		gotoxy(RIGHT+2,20);
		printf("▼:风之[加速]");

		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_INTENSITY);
		gotoxy(RIGHT+2,22);
		printf("★:疯狂[Crazy]");

		break;

	case creat_Map:

		SetConsoleTextAttribute(g_hConsoleOutput,0x0F);
		gotoxy(RIGHT+2,2);
		printf("操作提示：");

		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
		gotoxy(RIGHT+2,4);
		printf("<0>-->取消当前");

		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_GREEN|FOREGROUND_INTENSITY);
		gotoxy(RIGHT+2,6);
		printf("<1>-->■");
		
		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_BLUE|FOREGROUND_INTENSITY);
		gotoxy(RIGHT+2,8);
		printf("<2>-->■");
		
		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_RED|FOREGROUND_INTENSITY);
		gotoxy(RIGHT+2,10);
		printf("<3>-->■");

		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
		gotoxy(RIGHT+2,15);
		printf("<ENTER>完成构建");

		SetConsoleTextAttribute(g_hConsoleOutput,FOREGROUND_RED|FOREGROUND_INTENSITY);
		gotoxy(RIGHT+2,17);
		printf("<ESC>  取消构建");

		break;
	}
	
}

int checkComplete()	//检测游戏是否完成
{
	int i,j;
	int intSurplus=0;	//剩余


	for(i=LEFT+1;i<RIGHT;i++)
	{
		for (j=TOP+1;j<Bord.y;j++)
		{
			if(potVal[i][j]!=0)
			{
				intSurplus++;
			}
			if(intSurplus>3)
			{
				return 0;
			}
		}
	}

	return 1;
}