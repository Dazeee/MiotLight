#define BLINKER_WIFI
#define BLINKER_MIOT_LIGHT  // 注册为灯
#include <Servo.h>
#include <Blinker.h>

// #define DEBUG_ENABLE  // 启用Debug，支持print数据到串口监视器
#ifdef DEBUG_ENABLE
    #define BLINKER_PRINT Serial
#endif

char auth[] = "";   // 点灯密匙key
char ssid[] = "";          //wifi名称
char pswd[] = "";      //wifi密码

// 创建开关
BlinkerButton Button1("btn_k");    //舵机的关
BlinkerButton Button2("btn_g");    //舵机的开

//创建舵机
// 舵机1的数据线连接IO0，舵机2的数据线连接IO2，注意要共地，即单片机的GND要连接舵机1、舵机2的负极
Servo myservo, myservo2;//定义（创建）一个舵机名称为myservo

//创建3个滑块
BlinkerSlider Slider1("ran_max1");   //滑块1 控制第一个舵机最大角度
BlinkerSlider Slider2("ran_min1");   //滑块2 控制第一个舵机最小角度
BlinkerSlider Slider3("ran_mid1");   //滑块3 控制第一个舵机中间角度

//定义3个变量
int ran_max1 = 10, ran_min1 = 180, ran_mid1 = 86; // 舵机1的角度
int ran_max2 = 180, ran_min2 = 1, ran_mid2 = 86; // 舵机2的角度
// 舵机角度，建议基于三个滑块方法 slider1_callback、slider2_callback、slider3_callback 调试好角度后，写死上面的两个角度

// 开关状态，但因为开关灯是无状态的（无须关注之前灯的状态），可以说无须该变量了
bool status1 = false, status2 = false;

// app 端开灯按键 回调函数 
void button1_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);//读取按键状态

  myservo.write(ran_max1);  //写入滑块1 的角度   这个角度大小可以通过滑块1 来设置
  myservo2.write(ran_max2);
  Blinker.vibrate();       //使手机震动
  Blinker.delay(1000);    //这个delay 一定要有 不然舵机转不过来，1000毫秒延迟
  myservo.write(ran_mid1);  //1000毫秒=1s后恢复到滑块3设置的角度，
  myservo2.write(ran_mid2);
  Blinker.vibrate();  //使手机震动

}
// app 端关灯按键 回调函数 
void button2_callback(const String & state) {
  BLINKER_LOG("get button state: ", state);
  myservo.write(ran_min1);
  myservo2.write(ran_min2);
  Blinker.vibrate();
  Blinker.delay(1000);
  myservo.write(ran_mid1);
  myservo2.write(ran_mid2);
  Blinker.vibrate();
}


//小爱电源回调函数
//里面同样设置舵机的动作
void miotPowerState(const String & state)
{
  BLINKER_LOG("need set power state: ", state);
  if (state == BLINKER_CMD_ON) {  //上面条件不满足就是我在点灯设置了角度，重新判断是否为开，并按照点灯的数据来设置
    // digitalWrite(LED_BUILTIN, HIGH);
    status1 = true;
    BlinkerMIOT.powerState("on");
    BlinkerMIOT.print();
    myservo.write(ran_max1);  //写入滑块1 的角度   这个角度大小可以通过滑块1 来设置
    myservo2.write(ran_max2);
    Blinker.delay(1000);    //这个delay 一定要有 不然舵机转不过来
    myservo.write(ran_mid1);  //1s后恢复到滑块3设置的角度，
    myservo2.write(ran_mid2);
  }
  else if (state == BLINKER_CMD_OFF) {
    //  digitalWrite(LED_BUILTIN, LOW);
    status1 = false;
    BlinkerMIOT.powerState("off");
    BlinkerMIOT.print();
    myservo.write(ran_min1);
    myservo2.write(ran_min2);
    Blinker.delay(1000);
    myservo.write(ran_mid1);
    myservo2.write(ran_mid2);
  }
}



void miotQuery(int32_t queryCode)
{
  BLINKER_LOG("MIOT Query codes: ", queryCode, ",  status1: ", status1, ", status2: ", status2);

  switch (queryCode)
  {
    case BLINKER_CMD_QUERY_ALL_NUMBER :
      BLINKER_LOG("MIOT Query All");
      BlinkerMIOT.powerState(status1 ? "on" : "off");
      BlinkerMIOT.print();
      break;
    case BLINKER_CMD_QUERY_POWERSTATE_NUMBER :
      BLINKER_LOG("MIOT Query Power State");
      BlinkerMIOT.powerState(status1 ? "on" : "off");
      BlinkerMIOT.print();
      break;
    default :
      BlinkerMIOT.powerState(status1 ? "on" : "off");
      BlinkerMIOT.print();
      break;
  }
}

void miotMode(uint8_t mode)
{
    BLINKER_LOG("need set mode: ", mode);
    BlinkerMIOT.mode(mode);
    BlinkerMIOT.print();
    if (mode == BLINKER_CMD_MIOT_READING) {
        // 阅读模式：开灯1 关灯2
        myservo.write(ran_max1); 
        myservo2.write(ran_min2); 
        Blinker.delay(1000);
        myservo.write(ran_mid1);
        myservo2.write(ran_mid2);
    }
    else if (mode == BLINKER_CMD_MIOT_COMPUTER) {
        // 游戏模式：关灯1 开灯2
        myservo.write(ran_min1);
        myservo2.write(ran_max2);
        Blinker.delay(1000);
        myservo.write(ran_mid1);
        myservo2.write(ran_mid2);
    }
}

void dataRead(const String & data)
{
  BLINKER_LOG("Blinker readString: ", data);

  Blinker.vibrate();

  uint32_t BlinkerTime = millis();
  Blinker.print(BlinkerTime);
  Blinker.print("millis", BlinkerTime);
}



//滑块设置
void slider1_callback(int32_t value)   //最大角度滑动条
{
//   ran_max1 = value;
  ran_max2 = value;
//   myservo.write(ran_max1);
  myservo2.write(ran_max2);
  Blinker.vibrate();  //使手机震动
  Blinker.delay(1000);
  BLINKER_LOG("get slider value: ", value);
}


void slider2_callback(int32_t value)  //最小角度滑动条
{
//   ran_min1 = value;
  ran_min2 = value;
//   myservo.write(ran_min1);
  myservo2.write(ran_min2);
  Blinker.vibrate();  //使手机震动
  Blinker.delay(1000);
  BLINKER_LOG("get slider value: ", value);
}


void slider3_callback(int32_t value)   //中间角度滑动条
{
//   ran_mid1 = value;
  ran_mid2 = value;
//   myservo.write(ran_mid1);//打印输出滑条角度设置数据
  myservo2.write(ran_mid2);
  Blinker.vibrate();  //使手机震动
  Blinker.delay(1000);
  BLINKER_LOG("get slider value: ", value);
}



// 初始化串口，并开启调试信息
void setup() {


  Serial.begin(115200);
#ifdef DEBUG_ENABLE
  BLINKER_DEBUG.stream(BLINKER_PRINT); 
#endif

  Blinker.begin(auth, ssid, pswd);

  Button1.attach(button1_callback); //绑定按键执行回调函数
  Button2.attach(button2_callback); //绑定按键执行回调函数


  Slider1.attach(slider1_callback);
  Slider2.attach(slider2_callback);
  Slider3.attach(slider3_callback);

  myservo.attach(0); //定义舵机连接的引脚
  myservo2.attach(2);
  pinMode(LED_BUILTIN, OUTPUT);//定义灯引脚


  BlinkerMIOT.attachQuery(miotQuery);   //小爱同学回调函数
  Blinker.attachData(dataRead);
  BlinkerMIOT.attachPowerState(miotPowerState);
  BlinkerMIOT.attachMode(miotMode);
}





void loop() {

  Blinker.run();  //*每次运行都会将设备收到的数据进行一次解析。
//  digitalWrite(LED_BUILTIN, HIGH);//不要亮灯把这个删掉
//  digitalWrite(LED_BUILTIN, LOW); //不要亮灯把这个删掉
}



//blink app 界面配置代码复制到app里面即可
//{¨version¨¨2.0.0¨¨config¨{¨headerColor¨¨transparent¨¨headerStyle¨¨dark¨¨background¨{¨img¨¨assets/img/headerbg.jpg¨¨isFull¨«}}¨dashboard¨|{¨type¨¨tex¨¨t0¨¨blinker入门示例¨¨t1¨¨文本2¨¨bg¨Ë¨ico¨´´¨cols¨Í¨rows¨Ê¨key¨¨tex-272¨´x´É´y´É¨speech¨|÷¨lstyle¨Ê¨clr¨¨#FFF¨}{ßC¨btn¨ßJ¨fal fa-power-off¨¨mode¨ÉßE¨关灯¨ßGßHßIÉßKËßLËßM¨btn_g¨´x´Î´y´ÏßPÉ}{ßCßSßJßTßUÉßE¨开灯¨ßGßHßIÉßKËßLËßM¨btn_k¨´x´Ê´y´ÏßPÉ}{ßC¨ran¨ßE¨最大角度¨ßQ¨#389BEE¨¨max¨¢2u¨min¨ÉßIÉßKÑßLËßM¨ran_max1¨´x´É´y´¤DßPÉ}{ßCßZßE¨最小角度¨ßQßbßcº0ßdÉßIÉßKÑßLËßM¨ran_min1¨´x´É´y´ÒßPÉ}{ßCßZßE¨中间角度¨ßQßbßcº0ßdÉßIÉßKÑßLËßM¨ran_mid1¨´x´É´y´¤BßPÉ}{ßC¨deb¨ßUÉßIÉßKÑßLÌßM¨debug¨´x´É´y´Ì}÷¨actions¨|¦¨cmd¨¦¨switch¨‡¨text¨‡¨on¨¨打开?name¨¨off¨¨关闭?name¨—÷¨triggers¨|{¨source¨ßn¨source_zh¨¨开关状态¨¨state¨|ßpßr÷¨state_zh¨|¨打开¨´关闭´÷}÷´rt´|÷}
