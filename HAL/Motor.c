#include "Motor.h"

#define MAX_SPEED           60000
#define STOP                0
#define SPEED_20_PERCENT    (MAX_SPEED * 0.2)
#define SPEED_40_PERCENT    (MAX_SPEED * 0.4)
#define SPEED_60_PERCENT    (MAX_SPEED * 0.6)
#define SPEED_80_PERCENT    (MAX_SPEED * 0.8)


u8 Motor_u8MoveForward(){
    MTIM_voidPWMUPCh1(SPEED_80_PERCENT);
    MTIM_voidPWMUPCh2(STOP);
    MTIM_voidPWMUPCh3(SPEED_80_PERCENT);
    MTIM_voidPWMUPCh4(STOP);
}

u8 Motor_u8MoveBackward(){
    MTIM_voidPWMUPCh1(STOP);
    MTIM_voidPWMUPCh2(SPEED_80_PERCENT);
    MTIM_voidPWMUPCh3(STOP);
    MTIM_voidPWMUPCh4(SPEED_80_PERCENT);
}

u8 Motor_u8TurnRight(){
    MTIM_voidPWMUPCh1(SPEED_60_PERCENT);
    MTIM_voidPWMUPCh2(STOP);
    MTIM_voidPWMUPCh3(SPEED_80_PERCENT);
    MTIM_voidPWMUPCh4(STOP);
}

u8 Motor_u8TurnLeft(){
    MTIM_voidPWMUPCh1(SPEED_80_PERCENT);
    MTIM_voidPWMUPCh2(STOP);
    MTIM_voidPWMUPCh3(SPEED_60_PERCENT);
    MTIM_voidPWMUPCh4(STOP);
}

u8 Motor_u8Stop(){
    MTIM_voidPWMUPCh1(STOP);
    MTIM_voidPWMUPCh2(STOP);
    MTIM_voidPWMUPCh3(STOP);
    MTIM_voidPWMUPCh4(STOP);
}
