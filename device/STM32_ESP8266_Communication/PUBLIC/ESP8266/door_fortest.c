#include "door_fortest.h"
#include "cJSON.h"

struct config_def m_config;


void Door_Init(){
	m_config.hall_distance = 160;
	m_config.motor_len = 18 * 21;
	m_config.max_current = 4000;
	m_config.limit_current = 3000; 
	m_config.motion_range = 560;
	m_config.is_double_group = 1;
	m_config.is_lock_enabled = 0; 
	m_config.speed_ratio = 0.5f; 
	m_config.lock_delay_time = 1.0f;
	m_config.is_auto_close = 1;
	m_config.open_stay_time = 5.0f;
	m_config.is_detect_ir = 1;
	m_config.is_external_control_enabled = 1;
	m_config.is_detect_move = 1;
	m_config.move_distance = 40;
	m_config.move_speed = 50.0f;
	m_config.is_detect_resist = 1;
	m_config.resist_time = 0.03f;
	m_config.speed.open_speed = 300.0f;
	m_config.speed.close_speed = 300.0f;
	m_config.speed.study_speed = 75.0f;
	m_config.speed.max_speed = 500.0f;
	m_config.speed.min_speed = 50.0f;
	m_config.speed.acc_speed = 300.0f / 2.0f;
	m_config.speed.dec_speed = 300.0f / 4.0f;
	m_config.speed.demand_pre_dec_speed_ratio = 1.0f;
	m_config.speed.execute_pre_dec_speed_ratio = 1.0f;
	m_config.speed.start_current = 100.0f;
	m_config.speed.brake_start_current = 100.0f;
	m_config.speed.brake_speed_ratio = 2.0f;
	m_config.speed.change_current_1ms = 1.5f;
	m_config.speed.brake_change_current_1ms = 1.5f;
};

double get_json_value(const cJSON *json_obj, const char *name)
{
	double ret = 0.0f;
	const cJSON *value = cJSON_GetObjectItemCaseSensitive(json_obj, name);
	if (value && cJSON_IsNumber(value)) {
		ret = value->valuedouble;
	}

	return ret;
}

//从json应用配置
u8 from_json(cJSON *root)
{
	u8 ret = 1;
	do {
		cJSON *speed;
		if (root == NULL) {
			const char *error_ptr = cJSON_GetErrorPtr();
			if (error_ptr != NULL) {
				//ErrorMsg("from json error, %s", error_ptr);
			}
			break;
		}
		m_config.hall_distance = get_json_value(root, "hall_distance");
		m_config.motor_len = get_json_value(root, "motor_len");
		m_config.max_current = get_json_value(root, "max_current");
		m_config.limit_current = get_json_value(root, "limit_current");
		m_config.motion_range = get_json_value(root, "motion_range");
		m_config.is_lock_enabled = get_json_value(root, "is_lock_enabled");
		m_config.lock_delay_time = get_json_value(root, "lock_delay_time");
		m_config.is_auto_close = get_json_value(root, "is_auto_close");
		m_config.open_stay_time = get_json_value(root, "open_stay_time");
		m_config.is_detect_ir = get_json_value(root, "is_detect_ir");
		m_config.is_double_group = get_json_value(root, "is_double_group");
		m_config.is_detect_move = get_json_value(root, "is_detect_move");
		m_config.move_distance = get_json_value(root, "move_distance");
		m_config.move_speed = get_json_value(root, "move_speed");
		m_config.is_detect_resist = get_json_value(root, "is_detect_resist");
		m_config.resist_time = get_json_value(root, "resist_time");
		m_config.is_external_control_enabled = get_json_value(root, "is_external_control_enabled");

		speed = cJSON_GetObjectItemCaseSensitive(root, "speed");

		m_config.speed.open_speed = get_json_value(speed, "open_speed");
		m_config.speed.close_speed = get_json_value(speed, "close_speed");
		m_config.speed.study_speed = get_json_value(speed, "study_speed");
		m_config.speed.max_speed = get_json_value(speed, "max_speed");
		m_config.speed.min_speed = get_json_value(speed, "min_speed");
		m_config.speed.acc_speed = get_json_value(speed, "acc_speed");
		m_config.speed.dec_speed = get_json_value(speed, "dec_speed");
		m_config.speed.demand_pre_dec_speed_ratio = get_json_value(speed, "demand_pre_dec_speed_ratio");
		m_config.speed.execute_pre_dec_speed_ratio = get_json_value(speed, "execute_pre_dec_speed_ratio");
		m_config.speed.start_current = get_json_value(speed, "start_current");
		m_config.speed.brake_start_current = get_json_value(speed, "brake_start_current");
		m_config.speed.brake_speed_ratio = get_json_value(speed, "brake_speed_ratio");
		m_config.speed.change_current_1ms = get_json_value(speed, "change_current_1ms");
		m_config.speed.brake_change_current_1ms = get_json_value(speed, "brake_change_current_1ms");

		ret = 1;
	} while (0);
	return ret;
}

void set_json_value(cJSON *json_obj, const char *name, double v)
{
	cJSON *obj_v = cJSON_CreateNumber(v);

	if (obj_v) {
		cJSON_AddItemToObject(json_obj, name, obj_v);
	}
}

//把配置转json
cJSON *to_json()
{
	u8 ret = 1;
	cJSON *speed;
	cJSON *root = cJSON_CreateObject();
	if (root == NULL) {
		return NULL;
	}
	set_json_value(root, "hall_distance", m_config.hall_distance);
	set_json_value(root, "motor_len", m_config.motor_len);
	set_json_value(root, "max_current", m_config.max_current);
	set_json_value(root, "limit_current", m_config.limit_current);
	set_json_value(root, "motion_range", m_config.motion_range);
	set_json_value(root, "is_lock_enabled", m_config.is_lock_enabled);
	set_json_value(root, "lock_delay_time", m_config.lock_delay_time);
	set_json_value(root, "is_auto_close", m_config.is_auto_close);
	set_json_value(root, "open_stay_time", m_config.open_stay_time);
	set_json_value(root, "is_detect_ir", m_config.is_detect_ir);
	set_json_value(root, "is_double_group", m_config.is_double_group);
	set_json_value(root, "is_detect_move", m_config.is_detect_move);
	set_json_value(root, "move_distance", m_config.move_distance);
	set_json_value(root, "move_speed", m_config.move_speed);
	set_json_value(root, "is_detect_resist", m_config.is_detect_resist);
	set_json_value(root, "resist_time", m_config.resist_time);
	set_json_value(root, "is_external_control_enabled", m_config.is_external_control_enabled);

	speed = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "speed", speed);

	set_json_value(speed, "open_speed", m_config.speed.open_speed);
	set_json_value(speed, "close_speed", m_config.speed.close_speed);
	set_json_value(speed, "study_speed", m_config.speed.study_speed);
	set_json_value(speed, "max_speed", m_config.speed.max_speed);
	set_json_value(speed, "min_speed", m_config.speed.min_speed);
	set_json_value(speed, "acc_speed", m_config.speed.acc_speed);
	set_json_value(speed, "dec_speed", m_config.speed.dec_speed);
	set_json_value(speed, "demand_pre_dec_speed_ratio", m_config.speed.demand_pre_dec_speed_ratio);
	set_json_value(speed, "execute_pre_dec_speed_ratio", m_config.speed.execute_pre_dec_speed_ratio);
	set_json_value(speed, "start_current", m_config.speed.start_current);
	set_json_value(speed, "brake_start_current", m_config.speed.brake_start_current);
	set_json_value(speed, "brake_speed_ratio", m_config.speed.brake_speed_ratio);
	set_json_value(speed, "change_current_1ms", m_config.speed.change_current_1ms);
	set_json_value(speed, "brake_change_current_1ms", m_config.speed.brake_change_current_1ms);
	return root;
}