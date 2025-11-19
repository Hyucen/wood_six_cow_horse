//传感器状态定义
//无偏移情况
#define STATE_00100 0b00100 //理论不存在
#define STATE_01110 0b01110 //常规
//偏移情况
#define STATE_01100 0b01100 //右偏 向左调平
#define STATE_11000 0b11000 //左偏 向右调平

#define STATE_00110 0b00110 //左偏 向右调平
#define STATE_00011 0b00011 //左偏 向右调平
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
bool Stateing = false;

int debugcount = 0;
//任务流程 2 8 5 6 7 3 1 10 9 4
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
/* 除了长距离任务 其余所有的任务中间一定是至多间隔一个十字路口 
执行完十字路口函数之后开始任务前 十字路口设为禁止 任务结束后设为允许
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
  lineMove();
  /*switch(StateCount){
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
      setMove(80,80);
      delay(3000);
      stop();
      break;
    }
    default: {
      stop();
    }
  }*/
}

//任务函数
/*TODO 所有任务前应当加入十字路口判定 所有任务后应当加入状态机更改判定*/



void notFastBlock(){//减速带 0
}
void fuWaFuWaGrass(){//草地 1 全黑地形 需要自行设定偏移
  StateCount++; // 任务完成计数 1-2
}
void squareTunnel(){//方形隧道 2 
  StateCount++; // 任务完成计数 2-3
}
void uTunnel(){//U形隧道 3
  StateCount++; //任务完成计数 3-4
}
void woodenTriangle(){//防滑带 4 疑似全白地形
  StateCount++; // 任务完成计数4-5
}
void smallStair(){//小型阶梯 5 全黑色 地形 需加速到165 上坡完成后减速到100
  StateCount++; // 任务完成计数 5 - 6
}
void longLine(){//用于小型阶梯到栅格的长距离移动 6 速度理应加快
  StateCount++; // 任务完成计数 6 -7
}
void grid(){//栅格 疑似全白地形 7
  StateCount++; // 任务完成计数 7 - 8
}
void narrowBridge() { // 窄桥任务 8 全黑色 地形 需加速到165 上坡完成后减速到100
  StateCount++; // 任务完成计数 8 - 9
}
void bigStair(){//大型阶梯 9 全黑色地形 需加速到165 上坡完成后减速到100
  StateCount++; // 任务完成计数 9 - 10
}
void shitSpeed(){//石块 10 难走
  StateCount++; // 任务完成计数 10 - 11
}





//巡线模式
void lineMove(){//基础巡线模式
  //正常移动
  if (readSensorState() == STATE_01110 || readSensorState() == STATE_00100) {
    move();
  }else 

  //左偏移修正
  if(readSensorState() == STATE_01100 || readSensorState() == STATE_11000){
    turnLeftS();
  }else 

  //右偏移修正
  if(readSensorState() == STATE_00110 || readSensorState() == STATE_00011){
    turnRightS();
  }else 

  //带有路口判定的制动左转弯
  if(readSensorState() == STATE_11100 || readSensorState() == STATE_11110 && Stateing == false){

    //L T + 型路口判定
    //制动流程
    brake();
    delay(200); 

    //制动后判定是否是T +路口
    if(readSensorState() == STATE_11111){
      crossMove();
    }else 
    //非十字路口就左转
    {
      turnLeftL();
    }
  }else

  //带有路口判定的制动右转弯
  if(readSensorState() == STATE_00111 || readSensorState() == STATE_01111 && Stateing == false){

    //L T + 型路口判定
    //制动流程
    brake();
    delay(200);

    //制动后判定是否是T +路口
    if(readSensorState() == STATE_11111){
      crossMove();
    }else
    //非十字路口就右转
    {
      turnRightL();
    }
  }else if(readSensorState() == STATE_00000){
    //TODO 丢失线路判定
  }else {
    move();
  }
}


//移动和转向模式
void crossMove(){
  
}
void move(){//基础移动
  Serial.println("Move");
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);
  analogWrite(ENA, 140);
  analogWrite(ENB, 145);
}
void fixMove(){//无巡线模式下的修正尽力均衡移动
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);
  analogWrite(ENA, 140);
  analogWrite(ENB, 140);
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

void brake(){
    //制动
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 1);
    digitalWrite(IN4, 0);
    analogWrite(ENA, 200);
    analogWrite(ENB, 200);
    delay(5);
    stop();
}
void turnLeftL(){//左转
  //转弯流程
  unsigned long startTime= millis();
  const unsigned long timeout = 1000;
  while(readSensorState() != STATE_01100 && readSensorState() != STATE_01110 && readSensorState() != STATE_00110){
    digitalWrite(IN1, 0);
    digitalWrite(IN2, 1);
    digitalWrite(IN3, 1);
    digitalWrite(IN4, 0);
    analogWrite(ENA, 140);
    analogWrite(ENB, 140);
    if(millis() - startTime > timeout) return;
  }
}
void turnRightL(){//右转
  //转弯流程
  unsigned long startTime = millis();
  const unsigned long timeout = 1000;
  while(readSensorState() != STATE_01100 && readSensorState() != STATE_01110 && readSensorState() != STATE_00110){
    digitalWrite(IN1, 1);
    digitalWrite(IN2, 0);
    digitalWrite(IN3, 0);
    digitalWrite(IN4, 1);
    analogWrite(ENA, 140);
    analogWrite(ENB, 140);
    if(millis() - startTime > timeout) return;
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
