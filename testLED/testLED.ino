/*
 * LED测试程序 - Arduino Nano 33 BLE
 * 用于测试四个LED灯的连接是否正常，并设置为最大亮度
 * 
 * 硬件连接:
 * - 绿灯（正常）: D2
 * - 红灯（烟雾报警器）: D3
 * - 黄灯（敲门）: D4
 * - 蓝灯（门铃）: D5
 */

// 定义LED引脚
#define GREEN_LED 2   // 绿灯（正常状态）
#define RED_LED 3     // 红灯（烟雾报警器）
#define YELLOW_LED 4  // 黄灯（敲门）
#define BLUE_LED 5    // 蓝灯（门铃）

// 亮度设置（0-255，255为最亮）
#define MAX_BRIGHTNESS 255

void setup() {
  // 初始化串口通信
  Serial.begin(9600);
  Serial.println("LED测试程序启动 - 最大亮度模式");
  
  // 设置LED引脚为输出模式
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  
  // 初始状态：所有LED熄灭
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  
  Serial.println("所有LED初始化完成");
}

void loop() {
  // 测试模式1：所有LED同时点亮（最大亮度）
  Serial.println("测试模式1：所有LED同时点亮（最大亮度）");
  
  // 使用analogWrite可以控制亮度，但Arduino Nano 33 BLE的部分引脚支持PWM
  // 如果引脚支持PWM，使用analogWrite设置最大亮度
  // 否则使用digitalWrite设置为HIGH
  
  // 由于我们要最大亮度，直接使用digitalWrite更简单
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, HIGH);
  digitalWrite(YELLOW_LED, HIGH);
  digitalWrite(BLUE_LED, HIGH);
  
  delay(2000);  // 等待2秒
  
  // 测试模式2：所有LED同时熄灭
  Serial.println("测试模式2：所有LED同时熄灭");
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  delay(1000);  // 等待1秒
  
  // 测试模式3：逐个LED点亮（最大亮度）
  Serial.println("测试模式3：逐个LED点亮（最大亮度）");
  
  // 绿灯
  Serial.println("绿灯点亮");
  digitalWrite(GREEN_LED, HIGH);
  delay(1000);
  
  // 红灯
  Serial.println("红灯点亮");
  digitalWrite(RED_LED, HIGH);
  delay(1000);
  
  // 黄灯
  Serial.println("黄灯点亮");
  digitalWrite(YELLOW_LED, HIGH);
  delay(1000);
  
  // 蓝灯
  Serial.println("蓝灯点亮");
  digitalWrite(BLUE_LED, HIGH);
  delay(1000);
  
  // 测试模式4：逐个LED熄灭
  Serial.println("测试模式4：逐个LED熄灭");
  
  // 绿灯
  Serial.println("绿灯熄灭");
  digitalWrite(GREEN_LED, LOW);
  delay(1000);
  
  // 红灯
  Serial.println("红灯熄灭");
  digitalWrite(RED_LED, LOW);
  delay(1000);
  
  // 黄灯
  Serial.println("黄灯熄灭");
  digitalWrite(YELLOW_LED, LOW);
  delay(1000);
  
  // 蓝灯
  Serial.println("蓝灯熄灭");
  digitalWrite(BLUE_LED, LOW);
  delay(1000);
}