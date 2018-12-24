#ifndef MESSAGEDEF_H
#define MESSAGEDEF_H

//当前状态
  enum RunState{
    uint8_t RS_READY = 0;            //上电就绪状态
    uint8_t RS_STUDY_STEP_1 = 1;     //学习第一步（开门）
    uint8_t RS_STUDY_STEP_2 = 2;     //学习第二步（关门）
    uint8_t RS_STUDY_STEP_3 = 3;     //学习第三步（开门）
    uint8_t RS_CLOSED = 4;           //关门，没落锁
    uint8_t RS_LOCKED = 5;           //上锁
    uint8_t RS_UNLOCKING = 6;        //解锁中（解锁后会自动进入开门流程）
    uint8_t RS_OPENING = 7;          //开门中
    uint8_t RS_OPENED = 8;           //已开门
    uint8_t RS_CLOSING = 9;          //关门中
    uint8_t RS_ALWAYS_OPENED = 10;   //常开
    uint8_t RS_FREEZED = 11;         //冻结（按遥控的锁定进入冻结状态，和关门和锁定状态可以进入冻结状态）
    uint8_t RS_ERROR = 12;            //错误(比如学习失败)
    };

    //运行模式
    enum RunMode{
     uint8_t RM_SINGLE = 0;     //单门
     uint8_t RM_DOUBLE = 1;     //双门
    };

    //一般操作类型
    enum CommandType {
      uint8_t CT_OPEN = 0;      //开门
      uint8_t CT_CLOSE = 1;     //关门
      uint8_t CT_FREEZE = 2;    //冻结
      uint8_t CT_UNFREEZE = 3;  //解冻
    };
#endif // MESSAGEDEF_H
