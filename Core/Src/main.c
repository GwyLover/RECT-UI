/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "rtc.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "u8g2.h"												//导入u8g2库
#include "key.h"												//自定义按键
#include "string.h"
#include <math.h>
#include "encoder.h"										//自定义旋转编码器
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
u8g2_t u8g2;														//u8g2句柄
#define OLED_I2C_ADDR   (0x3C<<1)				//OLED地址
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
																				//硬件II2
uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)	
{
    static uint8_t  buffer[128];
    static uint8_t  buf_idx;

    switch(msg)
    {
    case U8X8_MSG_BYTE_INIT:
        break;                         
    case U8X8_MSG_BYTE_START_TRANSFER:
        buf_idx = 0;
        break;
    case U8X8_MSG_BYTE_SEND:
        {
            uint8_t *data = (uint8_t*)arg_ptr;
            while(arg_int--) buffer[buf_idx++] = *data++;
        }
        break;
    case U8X8_MSG_BYTE_END_TRANSFER:
        HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDR,
                                buffer, buf_idx, 200);
        break;
    default:
        return 0;
    }
    return 1;
}

																				//u8g2延时函数
uint8_t u8x8_gpio_and_delay_stm32f1(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch(msg)
    {
    case U8X8_MSG_DELAY_MILLI:    
        HAL_Delay(arg_int);
        break;
    case U8X8_MSG_DELAY_I2C:       
        for(uint16_t i=0;i<5*arg_int;i++) __NOP();
        break;
    default:
        return 0;
    }
    return 1;
}

void u8g2_init(void){										//u8g2初始化
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2,U8G2_R0,u8x8_byte_hw_i2c,u8x8_gpio_and_delay_stm32f1);		//硬件IIC
	u8g2_InitDisplay(&u8g2);
  u8g2_SetPowerSave(&u8g2, 0); 
}

																				//动画函数
void animation(float *a, float *a_trg,int16_t n)	
{
  if (*a != *a_trg)
  {
    if (fabs(*a - *a_trg) < 0.05f) *a = *a_trg;
    else *a += (*a_trg - *a) / (n / 10.0f);
  }
}

#define img_width 30										//菜单图标宽度
#define img_height 30										//菜单图标高度

																				//菜单图标
static uint8_t menu_img1[] = {					
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0xff, 0x3f, 0x00, 0xc0, 0xff, 0xff, 0x00, 
  0xc0, 0xff, 0xff, 0x00, 0xe0, 0xff, 0xff, 0x01, 
  0xe0, 0xff, 0xff, 0x01, 0xf0, 0xff, 0xff, 0x03, 
  0xf0, 0xff, 0xff, 0x03, 0xf8, 0xff, 0xff, 0x07, 
  0xf8, 0xff, 0xff, 0x07, 0xfc, 0xff, 0xff, 0x0f, 
  0xfc, 0x1f, 0xfe, 0x0f, 0xfe, 0x0f, 0xfc, 0x1f, 
  0xfe, 0x0f, 0xfc, 0x1f, 0xfe, 0x0f, 0xfc, 0x1f, 
  0xfe, 0x0f, 0xfc, 0x1f, 0xfc, 0x1f, 0xfe, 0x0f, 
  0xfc, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x07, 
  0xf8, 0xff, 0xff, 0x07, 0xf0, 0xff, 0xff, 0x03, 
  0xf0, 0xff, 0xff, 0x03, 0xe0, 0xff, 0xff, 0x01, 
  0xe0, 0xff, 0xff, 0x01, 0xc0, 0xff, 0xff, 0x00, 
  0xc0, 0xff, 0xff, 0x00, 0x00, 0xff, 0x3f, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t menu_img2[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x07, 0x00, 
  0xff, 0xff, 0x0f, 0x00, 0xff, 0xff, 0xff, 0x1f, 
  0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f, 
  0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f, 
  0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f, 
  0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f, 
  0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f, 
  0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f, 
  0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f, 
  0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f, 
  0xfe, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t menu_img3[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 
  0x00, 0xa0, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 
  0x00, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 
  0x00, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 
  0x00, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 
  0x00, 0x40, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 
  0x00, 0xe0, 0x00, 0x00, 0x00, 0xf0, 0x01, 0x00, 
  0xfe, 0xff, 0xff, 0x1f, 0xff, 0xff, 0xff, 0x3f, 
  0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f, 
  0x3f, 0xfc, 0x9f, 0x3f, 0x9f, 0xf9, 0x9f, 0x3f, 
  0xcf, 0xf3, 0x9f, 0x3f, 0xef, 0xf7, 0x03, 0x3c, 
  0xef, 0xf7, 0x03, 0x3c, 0xcf, 0xf3, 0x9f, 0x3f, 
  0x9f, 0xf9, 0x9f, 0x3f, 0x3f, 0xfc, 0x9f, 0x3f, 
  0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f, 
  0xff, 0xff, 0xff, 0x3f, 0xfe, 0xff, 0xff, 0x1f
};

static uint8_t menu_img4[] = {
  0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x80, 0x07, 
  0x7c, 0x00, 0x80, 0x0f, 0x0e, 0x00, 0x00, 0x1c, 
  0x06, 0x00, 0x00, 0x18, 0x06, 0x00, 0x00, 0x18, 
  0x06, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x18, 
  0x06, 0x00, 0x00, 0x18, 0x06, 0x00, 0x00, 0x18, 
  0x0e, 0x00, 0x00, 0x1c, 0xfc, 0x00, 0x80, 0x0f, 
  0xf8, 0x00, 0x80, 0x07, 0x00, 0x00, 0x00, 0x00
};

static uint8_t menu_img5[] = {
  0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x80, 0x07, 
  0x7c, 0x00, 0x80, 0x0f, 0x0e, 0x00, 0x00, 0x1c, 
  0x06, 0x00, 0x00, 0x18, 0x06, 0x00, 0x00, 0x18, 
  0x06, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x18, 
  0x06, 0x00, 0x00, 0x18, 0x06, 0x00, 0x00, 0x18, 
  0x0e, 0x00, 0x00, 0x1c, 0xfc, 0x00, 0x80, 0x0f, 
  0xf8, 0x00, 0x80, 0x07, 0x00, 0x00, 0x00, 0x00
};

static uint8_t menu_img6[] = {
  0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x80, 0x07, 
  0x7c, 0x00, 0x80, 0x0f, 0x0e, 0x00, 0x00, 0x1c, 
  0x06, 0x00, 0x00, 0x18, 0x06, 0x00, 0x00, 0x18, 
  0x06, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x18, 
  0x06, 0x00, 0x00, 0x18, 0x06, 0x00, 0x00, 0x18, 
  0x0e, 0x00, 0x00, 0x1c, 0xfc, 0x00, 0x80, 0x0f, 
  0xf8, 0x00, 0x80, 0x07, 0x00, 0x00, 0x00, 0x00
};

																	
static uint8_t *menu_imgs[] = {
	menu_img1,														//设置图标
	menu_img2,														//文件夹图标
	menu_img3,														//游戏图标
	menu_img4,														//空白图标
	menu_img5,														//空白图标
	menu_img6,														//空白图标
};				
				
#define MENU_NUM 6											//菜单元素数目
#define SETTING_NUM 2										//设置元素数目
#define FOLDER_NUM 9										//文件夹元素数目
#define GAME_NUM 3											//游戏元素数目
				
int16_t keynum = 0;											//按键值(有前进后退两个按键)
int16_t encodernum;											//旋钮值
int16_t state = 0;											//当前页面(对应enum菜单层级的内容)
int16_t previous_state = 0;							//前一个页面(对应enum菜单层级的内容)
struct tm timedate;											//时间

enum 																		//菜单层级
{
  M_OPEN,																//开机动画
		M_MENU,															//菜单
			M_SETTING,												//设置
			M_FOLDER,													//文件夹
			M_GAME,														//游戏
				M_WINDOW,												//弹窗
};

typedef struct fps{											//帧率结构体(由于设置的定时时间是0.2s，所有需要b+5次a读取一次)
	uint16_t a;
	uint16_t b;
}FPS;

typedef	struct frame{										//矩形结构体(所有显示内容都是通过这个结构体实现的)
	float frame_x;												//目标x
	float frame_y;												//目标y
	float frame_w;												//目标w
	float frame_h;												//目标h
	float frame_old_x;										//目标x
	float frame_old_y;										//当前y
	float frame_old_w;										//当前w
	float frame_old_h;										//当前h
}FRAME;

typedef	struct text{										//文本结构体(用于和矩形结构体结合显示文字内容)
	char *str;														//文字
	int select;														//点击确定之后会进入的页面(对应enum菜单层级的内容)
	int value;														//显示该文本对应的数值，值为0-100，填-1表示该文本不显示数值
}TEXT;

typedef	struct m_state{									//页面状态结构体
	int select;														//该页面点击确定之后会进入的页面(对应enum菜单层级的内容)
	int state;														//当前页面状态，0是不在运行，1是在运行，同一时间只有一个1
	int previous;													//上一个页面对应的enum值
	int number;														//如果该页面有多个元素，记录该页面选中的是哪一个元素
	int list_num;													//该页面有几个元素(主要是为了将相似的SETTING,FOLDER,GAME页面功能合并在一起，只需要传入该结构体就可以区分这三个页面)
	TEXT *list_text;											//文本结构体
	FRAME *m_lists;												//矩形结构体
	void (*m_init)(FRAME *m_lists,TEXT *list_text,int LIST_NUM);   //用于SETTING,FOLDER,GAME页面，记录初始化函数
}M_STATE;

FPS fps;																//fps结构体实例化

TEXT menu_text[] = {										//菜单文本
	{"Setting",2,-1},
	{"Folder",3,-1},
	{"Game",4,-1},
	{"None",1,-1},
	{"None",1,-1},
	{"None",1,-1},
};

TEXT setting_text[] = {									//设置文本
	{"reset",0,-1},
	{"light",5,100},
};

TEXT folder_text[] = {									//文件夹文本
	{"Text1",3,-1},
	{"Tggt2",3,-1},
	{"Text3",3,-1},
	{"Welcome",3,-1},
	{"Gnosis",3,-1},
	{"1",3,-1},
	{"0-000",3,-1},
	{"zzz",3,-1},
	{"????",3,-1},
};

TEXT game_text[] = {										//游戏文本
	{"Snake",4,-1},
	{"Game of Life",4,-1},
	{"Dinosaur Running",4,-1},
};

FRAME frame_list[4];										//开机动画
				
FRAME menu_name;												//菜单名
FRAME m_menus[MENU_NUM];								//菜单图标
FRAME frame;														//选中光标
FRAME slider;														//右侧进度条
				
FRAME m_folders[FOLDER_NUM];						//文件夹列表
FRAME m_settings[SETTING_NUM];					//设置列表
FRAME m_games[GAME_NUM];								//游戏列表
				
FRAME m_window;													//弹窗

M_STATE m_states[6];										//页面状态

void open_init(void){										//开机动画初始化
	for(int16_t i =0;i<4;i++){
		frame_list[i].frame_x = -2;
		frame_list[i].frame_y = i*16;
		frame_list[i].frame_w = 0;
		frame_list[i].frame_h = 16;
		frame_list[i].frame_old_x = -2;
		frame_list[i].frame_old_y = i*16;
		frame_list[i].frame_old_w = 0;
		frame_list[i].frame_old_h = 16;
	}
}

void menu_init(void){										//菜单初始化
	for(int16_t i =0;i<MENU_NUM;i++){
		m_menus[i].frame_x = 0;
		m_menus[i].frame_y = 22;
		m_menus[i].frame_old_x = 128 + i*40;
		m_menus[i].frame_old_y = 22;
	}
	
	menu_name.frame_x = 49;
	menu_name.frame_y = 58;
	menu_name.frame_old_x = 49;
	menu_name.frame_old_y = 75;
}

																				//列表初始化(将SETTING,FOLDER,GAME统一为列表)
void list_init(FRAME *m_lists,TEXT *list_text,int LIST_NUM){				
	for(int16_t i =0;i<LIST_NUM;i++){
		m_lists[i].frame_x = -6*(int)strlen(list_text[i].str);
		m_lists[i].frame_y = 11 + 16*i;
		m_lists[i].frame_old_x = -6*(int)strlen(list_text[i].str);
		m_lists[i].frame_old_y = 11 + 16*i;
	}
	
	frame.frame_x = 2;
	frame.frame_y = 0;
	frame.frame_w = 0;
	frame.frame_h = 16;
	frame.frame_old_x = 2;
	frame.frame_old_y = 0;
	frame.frame_old_w = 0;
	frame.frame_old_h = 16;
	
	slider.frame_x = 124;
	slider.frame_y = 0;
	slider.frame_w = 4;
	slider.frame_h = 0;
	slider.frame_old_x = 124;
	slider.frame_old_y = 0;
	slider.frame_old_w = 4;
	slider.frame_old_h = 0;
}

void window_init(void){									//弹窗初始化
	m_window.frame_x = 24;
	m_window.frame_y = -34;
	m_window.frame_w = 80;
	m_window.frame_h = 32;
	m_window.frame_old_x = 24;
	m_window.frame_old_y = -34;
	m_window.frame_old_w = 80;
	m_window.frame_old_h = 32;
}

void state_init(void){									//页面状态初始化
	for(int i = 0;i<6;i++){
		m_states[i].state = 0; 
		m_states[i].number = 0; 
	}
	
																				//对SETTING,FOLDER,GAME特殊处理
	m_states[M_SETTING].list_num = SETTING_NUM;
	m_states[M_SETTING].m_lists = m_settings;
	m_states[M_SETTING].list_text = setting_text;
	m_states[M_SETTING].m_init = list_init;
	
	m_states[M_FOLDER].list_num = FOLDER_NUM;
	m_states[M_FOLDER].m_lists = m_folders;
	m_states[M_FOLDER].list_text = folder_text;
	m_states[M_FOLDER].m_init = list_init;
	
	m_states[M_GAME].list_num = GAME_NUM;
	m_states[M_GAME].m_lists = m_games;
	m_states[M_GAME].list_text = game_text;
	m_states[M_GAME].m_init = list_init;
}

void show_fps(void){										//获取帧率
	static char buf[32];
	if(fps.a > 5){
		sprintf(buf, "fps:%d", fps.b);
		fps.a = 0;
		fps.b = 0;
	}
	fps.b++;
	u8g2_DrawStr(&u8g2,91,10,buf);
}

void show_time(void){										//获取时间，时间计算的函数位于rtc.c最末尾
	static char buf[32];
	My_Rtime(&timedate);
	sprintf(buf, "%02d:%02d", timedate.tm_hour - 8,timedate.tm_min);
	u8g2_DrawStr(&u8g2,0,10,buf);
}

void open_animation(void){							//开机动画
	static int16_t over = 1;							//开机动画有两部分，判断前一部分是否结束
	
	if(!m_states[M_OPEN].state){					//判断该页面是否是初次进入，是则初始化
		m_states[M_OPEN].state = 1;
		m_states[M_OPEN].select = M_OPEN;
		m_states[M_OPEN].previous = M_OPEN;
		open_init();
	}
	
	if(over){															//将动画分为两部分
		for(int16_t i =0;i<4;i++){
			if(i == 0 || frame_list[i-1].frame_old_w > (frame_list[i-1].frame_w/2))frame_list[i].frame_w = 132;
			animation(&frame_list[i].frame_old_w,&frame_list[i].frame_w,70);
		}
	}else{		
		for(int16_t i =0;i<4;i++){
			if(i == 0 || frame_list[i-1].frame_old_w < 66)frame_list[i].frame_w = 0;
			animation(&frame_list[i].frame_old_w,&frame_list[i].frame_w,70);
		}
	}
	
																				//这里判断动画是否结束，进入下一个页面
	if(frame_list[3].frame_old_w == 132)over = 0;
	if(over == 0 && frame_list[3].frame_old_w == 0){state = M_MENU;over = 1;m_states[M_OPEN].state = 0;}
	
																				//设置字体类型
	u8g2_SetFont(&u8g2, u8g2_font_9x18B_mf); 	
	u8g2_DrawStr(&u8g2, 33, 38, "RECT UI");
	
	u8g2_SetDrawColor(&u8g2, 2);   				// 参数0画黑色，参数1画白色，参数2反色，实现重叠部分字体反色
	if(over){
		for(uint16_t i=0;i<4;i++){
			u8g2_DrawBox(&u8g2, (int)frame_list[i].frame_old_x, (int)frame_list[i].frame_old_y, (int)frame_list[i].frame_old_w, (int)frame_list[i].frame_old_h); 
		}
	}else{
		for(uint16_t i=0;i<4;i++){
			u8g2_DrawBox(&u8g2, (int)(132 - frame_list[i].frame_old_w), (int)frame_list[i].frame_old_y, (int)frame_list[i].frame_old_w, (int)frame_list[i].frame_old_h); 
		}		
	}
}

void menu_animation(void){							//菜单显示
	static int16_t menunum = 0;						//记录选中项
	
																				//是否是第一次进入，执行初始化，并且判断前一个状态和当前是否相同，避免在当前页面点击无效按钮的时候触发初始化
	if( previous_state != state && !m_states[M_MENU].state){
		m_states[M_MENU].state = 1;
		m_states[M_MENU].previous = M_MENU;
		menu_init();
	}
	
	if(encodernum){												//读取编码器值
		if(encodernum < 0 && menunum > 0)menunum --;
		if(encodernum > 0 && menunum < MENU_NUM-1)menunum ++;
		encodernum = 0;
	}
	
	for(int16_t i =0;i<MENU_NUM;i++){			//配置动画
		m_menus[i].frame_x = 49+40*(i-menunum);
		if(i != menunum)m_menus[i].frame_y = 22;
		else m_menus[menunum].frame_y = 12;
		animation(&m_menus[i].frame_old_x,&m_menus[i].frame_x,20);
		animation(&m_menus[i].frame_old_y,&m_menus[i].frame_y,20);
	}
	if(fabs(m_menus[menunum].frame_old_x - m_menus[menunum].frame_x) > 8){
		menu_name.frame_y = 64;
	}else{
		menu_name.frame_old_y = menu_name.frame_y = 58;
	}
	menu_name.frame_old_x = menu_name.frame_x = (128 - 6*strlen(menu_text[menunum].str))/2; 
	animation(&menu_name.frame_old_y,&menu_name.frame_y,20);

	u8g2_SetFont(&u8g2, u8g2_font_6x10_mf); 
	show_fps();														//显示帧率
	show_time();													//显示时间
	for(int i=0;i<MENU_NUM;i++){					//绘制内容
		u8g2_DrawXBM(&u8g2, (int)m_menus[i].frame_old_x, (int)m_menus[i].frame_old_y, img_width, img_height, menu_imgs[i]);
	}
	u8g2_DrawStr(&u8g2, (int)menu_name.frame_old_x, (int)menu_name.frame_old_y, menu_text[menunum].str);
	
																				//记录当前选中的元素所对应的下一个页面的enum值
	m_states[M_MENU].select = menu_text[menunum].select;
}

																				//列表动画
void list_animation(int list_state, TEXT *list_text, FRAME *m_lists){
	static int16_t listnum = 0;						//当前选中项
	static int16_t framenum = 0;					//选中光标的位置

																				//是否是第一次进入，执行初始化，并且判断前一个状态和当前是否相同，
																				//避免在当前页面点击无效按钮的时候触发初始化，对弹窗页面特殊判断，因为弹窗页面也会使用该函数
	if( previous_state != state && state != M_WINDOW && !m_states[list_state].state){
		listnum = 0;
		framenum = 0;
		m_states[list_state].state = 1;
		m_states[list_state].select = list_text[listnum].select;
		m_states[list_state].previous = M_MENU;
		m_states[list_state].m_init(m_lists,list_text,m_states[list_state].list_num);
	}
	
	if(state != M_WINDOW && encodernum){	//当通过弹窗页面进入的时候，旋转编码器控制的是弹窗函数里面的数值，所以禁用这里的
		if(encodernum < 0 && listnum > 0){
			listnum --;
			if(framenum > 0)framenum --;
		}
		if(encodernum > 0 && listnum < m_states[list_state].list_num -1){
			listnum ++;
			if(framenum < 3)framenum ++;
		}
		encodernum = 0;
	}
	
																				//配置动画
	for(int16_t i =0;i<m_states[list_state].list_num;i++){
		m_lists[i].frame_y = 11+16*(i-(listnum-framenum));
		m_lists[i].frame_x = 7;
		animation(&m_lists[i].frame_old_y,&m_lists[i].frame_y,30);
		animation(&m_lists[i].frame_old_x,&m_lists[i].frame_x,30);
	}
	frame.frame_y = 16*framenum;
	frame.frame_w = 10+6*strlen(list_text[listnum].str); 
	animation(&frame.frame_old_y,&frame.frame_y,30);
	animation(&frame.frame_old_w,&frame.frame_w,30);
	slider.frame_h = (11 - m_lists[0].frame_y)/(16*(m_states[list_state].list_num - 4))*64;
	animation(&slider.frame_old_h,&slider.frame_h,30);
	
	u8g2_SetFont(&u8g2, u8g2_font_6x10_mf);
																				//绘制动画
	for(uint16_t i=0;i<m_states[list_state].list_num;i++){
		char buf[12];
		sprintf(buf,"%d",list_text[i].value);
		u8g2_DrawStr(&u8g2, (int)m_lists[i].frame_old_x, (int)m_lists[i].frame_old_y, list_text[i].str);
																				//特殊判断，只有文本参数值大于0才显示
		if(list_text[i].value >= 0)u8g2_DrawStr(&u8g2, (int)(110 - m_lists[i].frame_old_x), (int)m_lists[i].frame_old_y, buf);
	}
	u8g2_DrawRBox(&u8g2, (int)slider.frame_old_x, (int)slider.frame_old_y, (int)slider.frame_old_w, (int)slider.frame_old_h,0); 
	u8g2_SetDrawColor(&u8g2, 2);
	u8g2_DrawRBox(&u8g2, (int)frame.frame_old_x, (int)frame.frame_old_y, (int)frame.frame_old_w, (int)frame.frame_old_h,0); 
	
																				//记录当前选中的元素所对应的下一个页面的enum值
	m_states[list_state].select = list_text[listnum].select;
	m_states[list_state].number = listnum;//记录当前选中元素的列表序号
}
																				//弹窗显示动画
void window_animation(int previous_state, TEXT *list_text, FRAME *m_lists, int list_num){
	if(!m_states[state].state){
		m_states[state].state = 1;
		m_states[state].select = state;
		m_states[state].previous = previous_state;
		window_init();
	}
	
	if(encodernum){
		if(encodernum < 0 && list_text[list_num].value > 0){
			list_text[list_num].value --;
		}
		if(encodernum > 0 && list_text[list_num].value < 100){
			list_text[list_num].value ++;
		}
	}
		
	m_window.frame_y = 16;
	animation(&m_window.frame_old_y,&m_window.frame_y,30);
	
	u8g2_SetFont(&u8g2, u8g2_font_6x10_mf);
	u8g2_SetDrawColor(&u8g2, 0);
	u8g2_DrawBox(&u8g2, (int)m_window.frame_old_x, (int)m_window.frame_old_y, (int)m_window.frame_old_w, (int)m_window.frame_old_h);
	u8g2_SetDrawColor(&u8g2, 1);
	u8g2_DrawFrame(&u8g2, (int)m_window.frame_old_x, (int)m_window.frame_old_y, (int)m_window.frame_old_w, (int)m_window.frame_old_h);
	u8g2_DrawBox(&u8g2, (int)m_window.frame_old_x + 10, (int)m_window.frame_old_y + 22, (int)6*list_text[list_num].value/10, 3); 
	u8g2_DrawFrame(&u8g2, (int)m_window.frame_old_x + 9, (int)m_window.frame_old_y + 21, 62, 5);
	u8g2_DrawStr(&u8g2, (int)m_window.frame_old_x + 5, (int)m_window.frame_old_y + 13, "value:");
	char buf[12];
	sprintf(buf,"%d",list_text[list_num].value);
	u8g2_DrawStr(&u8g2, (int)m_window.frame_old_x + 57, (int)m_window.frame_old_y + 13, buf);
	
	u8g2_SetContrast(&u8g2,51*list_text[list_num].value/20);
}

																				//每0.2s中断一次，记录fps和读取旋转编码器
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim == &htim2){
		fps.a++;
		encodernum = get_cnt();
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();											
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
	u8g2_init();													//u8g2初始化
	encoder_init();												//旋转编码器初始化
	state_init();													//页面状态初始化
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	HAL_TIM_Base_Start_IT(&htim2);				//开启TIM中断
  while (1)
  {
		keynum = key_scanf();								//读取按键
		if(keynum){
			m_states[state].state = 0;				//将所有页面状态清0，让对应页面自己置1
																				//记录上一个页面，排除弹窗页面，因为previous_state用于传递给列表动画让其判断弹窗是从SETTINF,FOLDER,GAME哪一个页面进入的，
																				//如果再在弹窗页面点一次按钮previous_state就变成了M_WINDOW，list_animation就不知道是哪个页面了
			if(state != M_WINDOW)previous_state = state;
																				//前进
			if(keynum == 1)state = m_states[state].select;
																				//后退
			if(keynum == 2)state = m_states[state].previous;
		}

		u8g2_ClearBuffer(&u8g2);						//清空页面
		
		switch(state){
			case M_OPEN: open_animation();           	break;
      case M_MENU: 	 menu_animation();					break;
			case M_SETTING:
			case M_FOLDER:
			case M_GAME:
			case M_WINDOW:				
				if(state == M_WINDOW){					//因为弹窗和列表是同时显示的，所以特殊判断
					list_animation(previous_state, m_states[previous_state].list_text,m_states[previous_state].m_lists);
					window_animation(previous_state,m_states[previous_state].list_text,m_states[previous_state].m_lists,m_states[previous_state].number);
				}else{
					list_animation(state, m_states[state].list_text,m_states[state].m_lists);
				}
																								break;
		}
		
		u8g2_SendBuffer(&u8g2); 						//绘制页面
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
