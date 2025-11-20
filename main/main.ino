//main 仓库
//传感器状态定义
//无偏移情况
#define STATE_00100 0b00100 //理论不存在
#define STATE_01110 0b01110 //常规
//偏移情况
#define STATE_01100 0b01100 //右偏 向左调平
#define STATE_11000 0b11000 //右偏 向左调平
#define STATE_01000 0b01000 //右偏 向左调平
#define STATE_10000 0b10000 //右偏 向左调平



#define STATE_00110 0b00110 //左偏 向右调平
#define STATE_00011 0b00011 //左偏 向右调平
#define STATE_00010 0b00010 //左偏 向右调平
#define STATE_00001 0b00001 //左偏 向右调平


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

int substate;
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
  if (Stateing) {
    switch (StateCount) {
      case 0: { // 主任务0 准备进入减速带并识别十字路口
        while (StateCount != 1) {
          lineMove();
        }
        break;
      }
      
      case 1: { // 主任务1 减速带
        substate = 0; // 子状态0 巡线并等待第一个减速带,1 第一个减速带，2 空线, 3 第二个减速带
        
        while (substate <= 3) {
          switch (substate) {
            case 0: { // 巡线并等待第一个减速带
              Serial.println("巡线并等待第一个减速带");
              while (readSensorState() != STATE_11111) {
                lineMove();
              }
              substate++;
              break;
            }
            
            case 1: { // 进入第一个减速带 等待无减速带区
              Serial.println("进入第一个减速带 等待无减速带区");
              while (readSensorState() == STATE_11111) {
                setMove(140, 140);
              }
              substate++;
              break;
            }
            
            case 2: { // 进入空线 等待减速带
              Serial.println("进入空线 等待减速带");
              while (readSensorState() != STATE_11111) {
                lineMove();
              }
              substate++;
              break;
            }
            
            case 3: { // 进入减速带 等待空线
              Serial.println("进入减速带 等待空线");
              while (readSensorState() == STATE_11111) {
                setMove(140, 140);
              }
              substate++;
              break;
            }
          }
        }
        
        Stateing = false;
        StateCount++;
        break;
      }
      
      case 2: { // 主任务2 草地
        substate = 0;   // 子状态0 巡线并等待草地 1 经过草地并等待空线
        while (substate <= 1) {
          switch (substate) { 
            case 0: {//巡线并等待草地
              Serial.println("巡线并等待草地");
              while (readSensorState() != STATE_11111) {
                lineMove();
              }
              substate++;
              break;
            }
            case 1: {//经过草地 等待空线
              Serial.println("经过草地并等待空线");
              while (readSensorState() != STATE_01110 || readSensorState() != STATE_00110 || readSensorState() != STATE_01100){
                setMove(160,120);
              }
              substate++;
              break;
            }
          }
        }
        Stateing = false;
        StateCount++;
        break;
      }

      case 3: { // 主任务3 方形隧道
        substate = 0; // 执行自定义巡线逻辑 左转四次后方形隧道任务结束 【左转完成后才能更改状态】
        while(substate < 4){

        }
        Stateing = false;
        StateCount++;
        break;
      }

      case 4: { // 主任务4 U形隧道
        substate = 0; // 执行计时器 计时器结束后任务结束
        

        Stateing = false;
        StateCount++;
      }

      case 5: { // 主任务5 防滑带
        substate = 0; // 子状态0 巡线并等待防滑带, 子状态1 进入防滑带并等待空线（或使用delay控制经过时间）
        while (substate <=1 ) {
          switch (substate) {
            case 0: {//巡线并等待防滑带
              Serial.println("巡线并等待防滑带");
              while (readSensorState() != STATE_00000) {
                lineMove();
              }
              substate++;
              break;
            }
            case 1: {//经过防滑带
              Serial.println("经过防滑带");
              while (readSensorState() != STATE_01110 && readSensorState() != STATE_01100 && readSensorState() != STATE_00110){
                setMove(140,140);
              }
              substate++;
              break;
            }
          }
        }
        Stateing = false;
        StateCount++;
      }

      case 6: { // 主任务6X 长距离巡线处理
        substate = 0; // 执行自定义巡线逻辑 第一次 00111右拐 第二次00111直行 第三次00111右拐 结束
        // TODO 还没写
        Stateing = false;
        StateCount++;
      }

      case 7: { // 主任务7 栅格 
        substate = 0; // 子状态0 巡线并等待栅格， 子状态1 进入栅格并等待
        while(substate <= 1){
          switch(substate){
            case 0: { // 巡线并等待栅格
              Serial.println("巡线并等待栅格");
              while(readSensorState() != 11111){
                lineMove();
              }
              substate++;
              break;
            }
            case 1: { // 经过栅格并等待空线
              Serial.println("经过栅格并等待空线");
              while(readSensorState() != STATE_01110 && readSensorState() != STATE_01100 && readSensorState() != STATE_00110){
                setMove(120,120);
              }
              substate++;
              break;
            }
          }
        }
      }
      
      case 8: { // 主任务8 窄桥
        substate = 0; // 子状态0  巡线并等待窄桥, 子状态1 进入窄桥并等待
        Stateing = false;
        while(substate <= 1){
          switch(substate){
            case 0: { // 巡线并等待窄桥
              Serial.println("巡线并等待窄桥");
              while(readSensorState() != 11111){
                lineMove();
              }

            }
          }
        }

      }

    }// 这是 switch 的结尾

  }// 这是 if 的结尾
}// 这是 loop() 函数的结尾

//任务函数
/*TODO 所有任务前应当加入十字路口判定 所有任务后应当加入状态机更改判定*/


void notFastBlock() {

}
void fuWaFuWaGrass(){//草地 1 全黑地形 需要自行设定偏移
  static int substate = 0; // 子状态：0=草地 1=下一个十字路口
  Stateing = false;
  StateCount++; // 任务完成计数 1-2
}
void squareTunnel(){//方形隧道 2 
  Stateing = false;
  StateCount++; // 任务完成计数 2-3
}
void uTunnel(){//U形隧道 3
  Stateing = false;
  StateCount++; //任务完成计数 3-4
}
void woodenTriangle(){//防滑带 4 疑似全白地形
  Stateing = false;
  StateCount++; // 任务完成计数4-5
}
void smallStair(){//小型阶梯 5 全黑色 地形 需加速到165 上坡完成后减速到100
  Stateing = false;
  StateCount++; // 任务完成计数 5 - 6
}
void longLine(){//用于小型阶梯到栅格的长距离移动 6 速度理应加快
  StateCount++; // 任务完成计数 6 -7
}
void grid(){//栅格 疑似全白地形 7
  Stateing = false;
  StateCount++; // 任务完成计数 7 - 8
}
void narrowBridge() { // 窄桥任务 8 全黑色 地形 需加速到165 上坡完成后减速到100
  Stateing = false;
  StateCount++; // 任务完成计数 8 - 9
}
void bigStair(){//大型阶梯 9 全黑色地形 需加速到165 上坡完成后减速到100
  Stateing = false;
  StateCount++; // 任务完成计数 9 - 10
}
void shitSpeed(){//石块 10 难走
  Stateing = false;
  StateCount++; // 任务完成计数 10 - 11
}





//巡线模式
void lineMove(){//基础巡线模式
  //正常移动
  if (readSensorState() == STATE_01110 || readSensorState() == STATE_00100) {
    move();
  }else 

  //左偏移修正
  if(readSensorState() == STATE_01100 || readSensorState() == STATE_11000 || readSensorState() == STATE_01000 || readSensorState() == STATE_10000){
    turnLeftS();
  }else 

  //右偏移修正
  if(readSensorState() == STATE_00110 || readSensorState() == STATE_00011 || readSensorState() == STATE_00010 || readSensorState() == STATE_00001){
    turnRightS();
  }else 

  //带有路口判定的制动左转弯
  if(readSensorState() == STATE_11100 || readSensorState() == STATE_11110){

    //L T + 型路口判定
    //制动流程
    brake();
    delay(200); 

    //制动后判定是否是T +路口
    if(readSensorState() == STATE_11111 || readSensorState() == STATE_01110 && Stateing == false){
      crossMove();
    }else 
    //非十字路口就左转
    {
      turnLeftL();
    }
  }else

  //带有路口判定的制动右转弯
  if(readSensorState() == STATE_00111 || readSensorState() == STATE_01111 ){

    //L T + 型路口判定
    //制动流程
    brake();
    delay(200);

    //制动后判定是否是T +路口
    if(readSensorState() == STATE_11111 || readSensorState() == STATE_01110 && Stateing == false){
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

//十字路口判定
void crossMove() {
  if (Stateing == true) return; // 任务中不处理十字路口
  
  // 根据当前 StateCount 处理十字路口
  switch (StateCount) {
    case 0: case 1: case 2: case 5: case 7: case 9: 
      move();
      delay(500);
      break;
    case 3: case 6: case 8: 
      turnRightL();
      break;
  }
  
  // 处理完十字路口后，增加 StateCount 并设置 Stateing = true
  StateCount++;
  
  // 确保 StateCount 不越界（根据任务数量）
  if (StateCount > 11) {
    StateCount = 12; // 最终状态
  }
}
void move(){//基础移动
  Serial.println("Move");
  digitalWrite(IN1, 0);
  digitalWrite(IN2, 1);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);
  analogWrite(ENA, 110);
  analogWrite(ENB, 120);
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
    analogWrite(ENA, 250);
    analogWrite(ENB, 250);
    delay(10);
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
