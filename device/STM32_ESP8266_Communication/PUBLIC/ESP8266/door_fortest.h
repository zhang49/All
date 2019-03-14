#include "sys.h"
#include "cJSON.h"

#ifndef _DOOR_FORTEST_H
#define _DOOR_FORTEST_H


//和负载相关的配置
struct speed_def {
	float open_speed;                 //开门速度，单位mm/s，取值范围：100-1000
	float close_speed;                //关门速度，单位mm/s，取值范围：10-1000    
	float study_speed;                //学习速度，单位mm/s，取值范围：10-1000
	float max_speed;                  //最大速度，最大电流时的速度，单位mm/s，取值范围：100-1000
	float min_speed;                  //最低速度，单位mm/s，取值范围：100-1000    
	float acc_speed;                  //加速度
	float dec_speed;                  //减速度
	float demand_pre_dec_speed_ratio; //规划预减速比率
	float execute_pre_dec_speed_ratio;//执行预减速比率

	float start_current;              //启动电流
	float brake_start_current;        //刹车启动电流
	float brake_speed_ratio;          //启用刹车的速度差比率        
	float change_current_1ms;         //1ms电流变化值
	float brake_change_current_1ms;   //1ms刹车电流变化值
};

//配置内容定义
struct config_def {
	uint16_t magic_flag;        //读取配置时，是否有效的标记，有这个标记，代表是曾经保存的数据
								//工程参数
	int hall_distance;          //两组hall之间的距离，用来计算滑动轴承的长度，单位mm，取值范围：0-10000
	int motor_len;              //电机模组长度，单位mm，取值范围：100-1000    
	int max_current;            //最大电流，单位毫安（占空比满），取值范围：500-10000
	int limit_current;          //限制电流，单位毫安（运行电流）取值范围：500-10000

								//用户参数
	int motion_range;           //导轨的活动范围，取值范围：100-10000
	u8 is_lock_enabled;       //是否启用锁
	float lock_delay_time;      //上锁前等待时间，单位s，取值范围：0-100

	u8 is_auto_close;         //是否自动关门
	float open_stay_time;       //开门停留时间，单位s，取值范围：0-100

	u8 is_detect_ir;          //是否启用感应开门
	u8 is_double_group;       //是否双门联动

	u8 is_detect_move;        //是否检测位移开门
	int move_distance;          //位移的距离，单位mm，取值范围：0-100
	float move_speed;           //位移的速度，单位mm/s，取值范围：10-1000

	u8 is_detect_resist;      //检测遇阻
	float resist_time;          //遇阻时间，单位s，取值范围：0.01-1

	u8 is_external_control_enabled;//支持外部接口

	struct speed_def speed;            //速度相关定义

								//新版本不需要的
	float speed_ratio;          //速度比率，开关门的速度比率
};

void Door_Init();
double get_json_value(const cJSON *json_obj, const char *name);
u8 from_json(cJSON *root);
void set_json_value(cJSON *json_obj, const char *name, double v);
cJSON *to_json();

#endif