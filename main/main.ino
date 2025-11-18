//传感器状态定义
//无偏移情况
#define STATE_00100 0b00100 //理论不存在
#define STATE_01110 0b01110 //常规
//偏移情况
#define STATE_01100 0b01100 //右偏 向左调平
#define STATE_00110 0b00110 //左偏 向右调平
//转弯情况
#define STATE_00111 0b00111 //向右转
#define STATE_01111 0b01111 //向右转
#define STATE_11110 0b11110 //向左转
#define STATE_11100 0b11100 //向左转

#define STATE_00000 0b00000
#define STATE_11111 0b11111



//七路灰度传感器作为伪五路传感器使用 无视2 6路
const int L1 = A1; 
const int L2 = A2;
const int MID = A3;
const int R2 = A4;
const int R1 = A5;

//方向控制及直流电机控制引脚
const int IN1 = 4;
const int IN2 = 5;
const int ENA = 9;//右排轮
const int ENB = 10;//左排轮
const int IN3 = 6;
const int IN4 = 7;

int StateCount = 0;
//任务状态字 2 8 5 6 7 3 1 10 9 4
// 栅格1
// 减速带2
// 小型阶梯3
// 石块4
// 方形隧道5
// U型隧道6
// 防滑带7
// 柔软草地8
// 大型阶梯9
// 窄桥10
  /*1减速带本体带全黑
  全黑后++ 入线一段距离十字路口 直走到草坪
  草坪理应全白，
  全白后++ 入线一段距离十字路口， 直走到方形隧道
  方形隧道 两个左转后++ 右转再左转（能否直接巡线？)
  到十字路口后右转
  再左转再右转到U型隧道
  U型隧道出口后右转++
  后右转到十字路口后直走到防滑带
  防滑带全白
  全白后 到十字路口后直行再++
  小型阶梯全黑
  全黑后++ 入线一段距离到十字路口右转
  长距离计算一个十字一个T形增加速度（多写一个函数）
  T形后++到栅格
  栅格出来后遇到十字路口直走后++
  到窄桥 窄桥理应全黑
  全黑后 入线++
  遇到十字路口后右转
  右转后进全黑大型阶梯
  出全黑进线++
  下个十字路口直行
  触发直行后切换计时定速移动
  */


void setup() {
  //传感器初始化

  pinMode(L1, INPUT); 
  pinMode(L2, INPUT);
  pinMode(MID, INPUT);
  pinMode(R2, INPUT);
  pinMode(R1, INPUT);


  //电机初始化
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  Serial.begin(9600);
  Serial.println("系统启动");

}

void loop() {
  switch(StateCount){
    case 0: {
      notFastBlock();
      break;
    }
    case 1: {
      fuWaFuWaGrass();
      break;
    }
    case 2: {
      squareTunnel();
      break;
    }
    case 3: {
      uTunnel();
      break;
    }
    case 4: {
      woodenTriangle();
      break;
    }
    case 5: {
      smallStair();
      break;
    }
    case 6: {
      longLine();
      break;
    }
    case 7: {
      grid();
      break;
    }
    case 8: {
      narrowBridge();
      break;
    }
    case 9: {
      bigStair();
      break;
    }
    case 10: {
      shitSpeed();
      break;
    }
    case 11: {
      moveWithSpeed(70,40);
      delay(3000);
      stop();
      break;
    }
    default: {
      stop();
    }
  }
}

//任务函数
/*TODO 所有任务前应当加入十字路口判定 所有任务后应当加入状态机更改判定*/



void notFastBlock(){//减速带 0

  while (readSensorState() != STATE_11111) {
    fixMove();
  }
  set
  StateCount++; // 任务完成计数 0-1

}
void fuWaFuWaGrass(){//草地 1
  //十字路口监测
  while(readSensorState() != STATE_11111) {
    lineMove();
  }
  fixMove();
  delay(1000);
  // 上草检测：巡线直到传感器全白（草地入口）
  while(readSensorState() != STATE_00000) {
    lineMove();
  }
  // 下草检测：直行直到传感器非全白（草地出口）
  while (readSensorState() == STATE_00000) {
    fixMove();
  }
  StateCount++; // 任务完成计数 1-2
}
void squareTunnel(){//方形隧道 2 定制巡线逻辑
  int TurnLeftCount = 0;
  while(readSensorState() != STATE_11111) {
    lineMove();
  }
  fixMove();
  delay(1000);
  //左右左左右左
  while(1){//定制巡线逻辑
    if (readSensorState() == STATE_00100 && TurnLeftCount<4) {
      move();
    }else if(readSensorState() == STATE_01100  && TurnLeftCount < 4){
      turnLeftS();
    }else if(readSensorState() == STATE_00110  && TurnLeftCount < 4){
      turnRightS();
    }else if(readSensorState() == STATE_11100 && TurnLeftCount < 4){
      turnLeftL();
      TurnLeftCount++;
    }else if(readSensorState() == STATE_00111 && TurnLeftCount < 4){
      turnRightL();
    }else if(TurnLeftCount == 4){
      break;
    }
  }
  StateCount++; // 任务完成计数 2-3

}
void uTunnel(){//U形隧道 3
  //十字路口监测
  while(readSensorState() != STATE_11111) {
    turnRightL();
  }
  fixMove();
  delay(1000);
  while(readSensorState() != STATE_11111) {
    lineMove();
  }
  fixMove();
  delay(1000);
  StateCount++; //任务完成计数 3-4
}
void woodenTriangle(){//防滑带 4
  // 上格检测：巡线直到传感器全白（防滑带入口）
  while(readSensorState() != STATE_00000) {
    lineMove();
  }
  // 下格检测：直行直到传感器非全白（防滑带出口）
  while (readSensorState() == STATE_00000) {
    fixMove();
  }
  StateCount++; // 任务完成计数4-5
}
void smallStair(){//小型阶梯 5 全黑色 
  // 上梯检测：巡线直到传感器全黑（入口）
  while (readSensorState() != STATE_11111) {
    lineMove();
  }

  // 下梯检测：直行直到传感器非全黑（出口）
  while (readSensorState() == STATE_11111) {
    fixMove();
  }

  StateCount++; // 任务完成计数 5 - 6
}
void longLine(){//用于小型阶梯到栅格的长距离移动 6 速度理应加快
  while (readSensorState() != STATE_11111) {
    lineMove();
  }
  turnRightL();
  while (readSensorState() != STATE_11111) {
    lineMove();
  }
  fixMove();
  delay(1000);
  while (readSensorState() != STATE_11111) {
    lineMove();
  }
  turnRightL();
  StateCount++; // 任务完成计数 6 -7
}
void grid(){//栅格 疑似全白地形 7
  // 上格检测：巡线直到传感器全白
  while(readSensorState() != STATE_00000) {
    lineMove();
  }
  // 格子检测：直行直到传感器非白（草地出口）
  while (readSensorState() == STATE_00000) {
    fixMove();
  }
  StateCount++; // 任务完成计数 7 - 8
}
void narrowBridge() { // 窄桥任务（全黑色） 8
  while (readSensorState() != STATE_11111) {
    lineMove();
  }
  fixMove();
  delay(1000);
  // 上桥检测：巡线直到传感器全黑（窄桥入口）
  while (readSensorState() != STATE_11111) {
    lineMove();
  }

  // 下桥检测：直行直到传感器非全黑（窄桥出口）
  while (readSensorState() == STATE_11111) {
    fixMove();
  }
  StateCount++; // 任务完成计数 8 - 9
}
void bigStair(){//大型阶梯 全黑色 9
  // 十字路口监测
  while (readSensorState() != STATE_11111) {
    lineMove();
  }
  turnRightL();
  // 上梯检测：巡线直到传感器全黑（入口）
  while (readSensorState() != STATE_11111) {
    lineMove();
  }

  // 下梯检测：直行直到传感器非全黑（出口）
  while (readSensorState() == STATE_11111) {
    fixMove();
  }

  StateCount++; // 任务完成计数 9 - 10
}
void shitSpeed(){//石块（屎块地形） 带黑色 一半
  // 十字路口监测
  while (readSensorState() != STATE_11111) {
    lineMove();
  }
  turnRightL();
  fixMove();
  delay(2000);
  lineMove();
  delay(5000);
  fixMove();
  delay(7000);
  StateCount++; // 任务完成计数 10 - 11
}

//巡线模式
void lineMove(){//基础巡线模式
  if (readSensorState() == STATE_01110) {
    move();
  }else if(readSensorState() == STATE_01100){
    turnLeftS();
  }else if(readSensorState() == STATE_00110){
    turnRightS();
  }else if(readSensorState() == STATE_11100 || readSensorState() == STATE_11110){
    delay(500);
    if(readSensorState() != STATE_11111){
    turnLeftL();
    } else {
      move();
    }
  }else if(readSensorState() == STATE_00111 || readSensorState() == STATE_01111){
    delay(500);
    if(readSensorState() != STATE_11111){
    turnRightL();
    } else {
      move();
    }
  }else if(readSensorState() == STATE_00000){
    handleLostLine();
  }else {
    move();
  }
}


//移动和转向模式
void move(){//基础移动
  Serial.println("Move");
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);
  analogWrite(ENA, 160);
  analogWrite(ENB, 160);
}
void fixMove(){//无巡线模式下的修正尽力均衡移动
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);
  analogWrite(ENA, 160);
  analogWrite(ENB, 160);
}
void setMove(int enbspeed,int enaspeed) {//定速移动 参数 左 右
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);
  analogWrite(ENA, enaspeed);
  analogWrite(ENB, enbspeed);
}
void stop(){//停止
  Serial.println("STOP");
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 0);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 0);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
void turnRightS(){//向右侧原地修正
  Serial.println("TRs");
  digitalWrite(IN1, 1);
  digitalWrite(IN2, 0);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);
  analogWrite(ENA, 140);
  analogWrite(ENB, 140);
}

void turnLeftS(){//向左侧原地修正
  Serial.println("TLs");
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 1);
  digitalWrite(IN4, 0);
  analogWrite(ENA, 140);
  analogWrite(ENB, 140);
}
void turnLeftL(){//左转
  Serial.println("TRR");
  while(readSensorState() != STATE_01100 && readSensorState() != STATE_01110 && readSensorState() != STATE_00110){
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 1);
  digitalWrite(IN4, 0);
  analogWrite(ENA, 140);
  analogWrite(ENB, 140);
  }
}
void turnRightL(){//右转
  Serial.println("TRL");
  while(readSensorState() != STATE_01100 && readSensorState() != STATE_01110 && readSensorState() != STATE_00110){
  digitalWrite(IN1, 1);
  digitalWrite(IN2, 0);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);
  analogWrite(ENA, 140);
  analogWrite(ENB, 140);
  }
}

int readSensorState() {//传感器状态
  int g1 = digitalRead(L1);
  int g2 = digitalRead(L2);
  int g3 = digitalRead(MID);
  int g4 = digitalRead(R2);
  int g5 = digitalRead(R1);
  // 将7个传感器状态组合成一个7位二进制数（范围0-127）
  return (g1 << 4) | (g2 << 3) | (g3 << 2) | (g4 << 1) | g5;
}
