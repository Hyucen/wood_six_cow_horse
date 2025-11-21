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
  Serial.println(StateCount);
  if (Stateing) {
    switch (StateCount) {
      case 0: { // 主任务0 准备进入减速带并识别十字路口
        while (StateCount != 1) {
          lineMove();
        }
        break;
      }
      
      case 1: { // 主任务1 减速带
        static int substate = 0; // 子状态0 巡线并等待第一个减速带,1 第一个减速带，2 空线, 3 第二个减速带  
        
        while (substate <= 3) {
          switch (substate) {
            case 0: { // 巡线并等待第一个减速带
              Serial.println("巡线并等待第一个减速带");
              delay(100);
              while (readSensorState() != STATE_11111 ) {
                lineMove();
              }
              substate++;
              break;
            }
            
            case 1: { // 进入第一个减速带 等待无减速带区
              Serial.println("进入第一个减速带 等待无减速带区");
              delay(100);
              while (readSensorState() == STATE_11111) {
                setMove(140, 140);
              }
              substate++;
              break;
            }
            
            case 2: { // 进入空线 等待减速带
              Serial.println("进入空线 等待减速带");
              delay(100);
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
        break;
      }
      
      case 2: { // 主任务2 草地
        static int substate = 0;   // 子状态0 巡线并等待草地 1 经过草地并等待空线
        while (substate <= 1) {
          switch (substate) { 
            case 0: { // 巡线并等待草地
              Serial.println("巡线并等待草地");
              while (readSensorState() != STATE_11111) {
                lineMove();
              }
              substate++;
              break;
            }
            case 1: { // 经过草地 等待空线
              Serial.println("经过草地并等待空线");
              while (readSensorState() != STATE_01110 && readSensorState() != STATE_00110 && readSensorState() != STATE_01100) {
                setMove(170, 170);
              }
              substate++;
              break;
            }
          }
        }
        Stateing = false;
        break;
      }

      case 3: { // 主任务3 方形隧道
        static int turnCount = 0; // 记录已经完成的左转弯次数
        static bool waitingForTurn = true; // 是否正在等待转弯条件

        if (turnCount < 4) {
          if (waitingForTurn) {
            // 巡线直到检测到左转弯条件
            lineMove();
            // 检测左转弯条件（根据您的传感器状态定义）
            if (readSensorState() == STATE_11100 || readSensorState() == STATE_11110) {
              waitingForTurn = false;
              Serial.println("检测到左转弯条件，开始左转");
            }
         } else {
           // 执行左转弯（调用您的阻塞函数 turnLeftL）
           turnLeftL();
            turnCount++;
            Serial.print("完成左转弯次数: ");
            Serial.println(turnCount);
            waitingForTurn = true; // 重置状态，等待下一次转弯条件
          }
       } else {
         // 完成四次转弯，任务结束
          Serial.println("方形隧道任务完成");
         Stateing = false;
         turnCount = 0; // 重置计数
         waitingForTurn = true; // 重置状态
        }
       break;
           }

      case 4: { // 主任务4 U形隧道
        static unsigned long tunnelStartTime = 0;
        
        if (tunnelStartTime == 0) {
          tunnelStartTime = millis();
          Serial.println("U形隧道任务开始");
        }
        
        // 在隧道内持续巡线
        lineMove();
        
        // 检查是否达到10秒
        if (millis() - tunnelStartTime >= 10000) {
          Serial.println("U形隧道任务完成");
          Stateing = false;
          tunnelStartTime = 0; // 重置计时器
        }
        break;
      }

      case 5: { // 主任务5 防滑带
        static int substate = 0; // 子状态0 巡线并等待防滑带, 子状态1 进入防滑带并等待空线
        while (substate <= 1) {
          switch (substate) {
            case 0: { // 巡线并等待防滑带
              Serial.println("巡线并等待防滑带");
              while (readSensorState() != STATE_00000) {
                lineMove();
              }
              substate++;
              break;
            }
            case 1: { // 经过防滑带
              Serial.println("经过防滑带");
              while (readSensorState() != STATE_01110 && readSensorState() != STATE_01100 && readSensorState() != STATE_00110) {
                setMove(140, 140);
              }
              substate++;
              break;
            }
          }
        }
        Stateing = false;
        break;
      }
      
      case 6: { // 主任务6 小型阶梯
        static int substate = 0; // 子状态0 巡线并等待阶梯, 子状态1 进入阶梯并等待
        while (substate <= 1) {
          switch (substate) {
            case 0: { // 巡线并等待阶梯
              Serial.println("巡线并等待阶梯");
              while (readSensorState() != STATE_11111) {
                lineMove();
              }
              substate++;
              break;
            }
            case 1: { // 进入阶梯等待空线
              Serial.println("进入阶梯");
              setMove(170, 170);
              delay(2000);
              while (readSensorState() != STATE_01110 && readSensorState() != STATE_01100 && readSensorState() != STATE_00110) {
                setMove(70, 70);
              }
              substate++;
              break;
            }
          }
        }
        Stateing = false;
        break;
      }
      
      case 7: { // 主任务7 长距离巡线处理
        static int crossCount = 0; // 十字路口计数
        static bool inAction = false; // 是否正在执行动作
        static unsigned long actionStartTime = 0;
        
        if (!inAction) {
          // 正常巡线
          lineMove();
          
          // 检测十字路口
          if (readSensorState() == STATE_00111 || readSensorState() == STATE_01111) {
            inAction = true;
            actionStartTime = millis();
            
            switch (crossCount) {
              case 0: // 第一次右拐
                Serial.println("检测到第一个十字路口，右转");
                turnRightL();
                crossCount++;
                break;
              case 1: // 第二次直行
                Serial.println("检测到第二个十字路口，直行");
                move();
                crossCount++;
                break;
              case 2: // 第三次右拐
                Serial.println("检测到第三个十字路口，右转");
                turnRightL();
                crossCount++;
                break;
            }
          }
        } else {
          // 检查动作是否完成
          if (millis() - actionStartTime > 1000) { // 假设动作完成需要1秒
            inAction = false;
            
            // 检查是否完成所有十字路口
            if (crossCount >= 3) {
              Serial.println("长距离巡线任务完成");
              Stateing = false;
              crossCount = 0; // 重置计数
            }
          }
        }
        break;
      }
      
      case 8: { // 主任务8 栅格
        static int substate = 0; // 子状态0 巡线并等待栅格，子状态1 进入栅格并等待
        while (substate <= 1) {
          switch (substate) {
            case 0: { // 巡线并等待栅格
              Serial.println("巡线并等待栅格");
              while (readSensorState() != STATE_11111) {
                lineMove();
              }
              substate++;
              break;
            }
            case 1: { // 经过栅格并等待空线
              Serial.println("经过栅格并等待空线");
              while (readSensorState() != STATE_01110 && readSensorState() != STATE_01100 && readSensorState() != STATE_00110) {
                setMove(120, 120);
              }
              substate++;
              break;
            }
          }
        }
        Stateing = false;
        break;
      }
      
      case 9: { // 主任务9 窄桥
        static int substate = 0; // 子状态0 巡线并等待窄桥, 子状态1 进入窄桥并等待
        while (substate <= 1) {
          switch (substate) {
            case 0: { // 巡线并等待窄桥
              Serial.println("巡线并等待窄桥");
              while (readSensorState() != STATE_11111) {
                lineMove();
              }
              substate++;
              break;
            }
            case 1: { // 进入窄桥等待空线
              Serial.println("进入窄桥");
              setMove(170, 170);
              delay(2000);
              while (readSensorState() != STATE_01110 && readSensorState() != STATE_01100 && readSensorState() != STATE_00110) {
                setMove(70, 70);
              }
              substate++;
              break;
            }
          }
        }
        Stateing = false;
        break;
      }
      
      case 10: { // 主任务10 大型阶梯
        static int substate = 0; // 子状态0 巡线并等待阶梯, 子状态1 进入阶梯并等待
        while (substate <= 1) {
          switch (substate) {
            case 0: { // 巡线并等待阶梯
              Serial.println("巡线并等待阶梯");
              while (readSensorState() != STATE_11111) {
                lineMove();
              }
              substate++;
              break;
            }
            case 1: { // 进入阶梯等待空线
              Serial.println("进入阶梯");
              setMove(170, 170);
              delay(3500);
              while (readSensorState() != STATE_01110 && readSensorState() != STATE_01100 && readSensorState() != STATE_00110) {
                setMove(70, 70);
              }
              substate++;
              break;
            }
          }
        }
        Stateing = false;
        break;
      }
      
      case 11: { // 主任务11 巡线加速冲刺
        Stateing = false;
        break;
      }
      
      default: {
        stop();
        break;
      }
    } // 结束 switch
  } // 结束 if (Stateing)
} // 结束 loop()



//巡线模式
void lineMove() {
  // 正常移动和偏移修正（始终执行）
  if (readSensorState() == STATE_01110 || readSensorState() == STATE_00100) {
    move();
  } else if (readSensorState() == STATE_01100 || readSensorState() == STATE_11000 || 
             readSensorState() == STATE_01000 || readSensorState() == STATE_10000) {
    turnLeftS();
  } else if (readSensorState() == STATE_00110 || readSensorState() == STATE_00011 || 
             readSensorState() == STATE_00010 || readSensorState() == STATE_00001) {
    turnRightS();
  } else if (readSensorState() == STATE_00000) {
  } else {
    move();
  }

  // 只有在 Stateing 为 false 时才执行路口判定和转弯
  if (!Stateing) {
    // 带有路口判定的制动左转弯
    if (readSensorState() == STATE_11100 || readSensorState() == STATE_11110) {
      brake();
      delay(200);
      if (readSensorState() == STATE_11111 || readSensorState() == STATE_01110) {
        crossMove();
      } else {
        turnLeftL();
      }
    }
    // 带有路口判定的制动右转弯
    else if (readSensorState() == STATE_00111 || readSensorState() == STATE_01111) {
      brake();
      delay(200);
      if (readSensorState() == STATE_11111 || readSensorState() == STATE_01110) {
        crossMove();
      } else {
        turnRightL();
      }
    }
  }
}


//移动和转向模式

//十字路口判定
void crossMove() {
  if (Stateing == true) return; // 任务中不处理十字路口
  
  // 根据当前 StateCount 处理十字路口
  switch (StateCount) {
    case 0: case 1: case 2: case 4: case 5: case 7: case 9: case 10: 
      // 这些任务后的十字路口直行
      move();
      delay(500);
      break;
    case 3: case 6: case 8: 
      // 这些任务后的十字路口右转
      turnRightL();
      break;
  }
  
  // 处理完十字路口后，增加 StateCount 并设置 Stateing = true
  StateCount++;
  Stateing = true;
  // 确保 StateCount 不越界（根据您的任务数量）
  if (StateCount > 11) {
    StateCount = 12; // 最终状态
  }
}
void move(){//基础移动
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
  digitalWrite(IN1, 1);
  digitalWrite(IN2, 0);
  digitalWrite(IN3, 0);
  digitalWrite(IN4, 1);
  analogWrite(ENA, 140);
  analogWrite(ENB, 140);
}

void turnLeftS(){//向左侧原地修正
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
