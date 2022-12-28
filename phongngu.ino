#include <Servo.h>
//#include <ArduinoJson.h>
//#include <Wire.h> 
//#include <LiquidCrystal_I2C.h>
#define cbas 14
#define cbnd A0
#define rem_ngu 12
#define den_hien 2
#define den_ngu 15
#define quat_ngu 13

# define N0_OF_BUTTONS 4
// we define two buffers for debouncing
boolean state_drive[N0_OF_BUTTONS] = {0};
//state_drive[0] la che do cua rem auto/thuong
//state_drive[1] la trang thai rem dong/mo
//state_drive[2] la trang thai den dong/mo
//state_drive[3] la trang thai quat dong/mo
int button[N0_OF_BUTTONS]={16,5,4,0};
int debounceButtonBuffer[N0_OF_BUTTONS]={1};
int pos=179;
int nhiet_do;
int data_send = 0, data_receive = 0;
Servo rem ;
//LiquidCrystal_I2C lcd(0x27,16,2);
void setup() 
{
  pinMode(den_hien, OUTPUT);
  pinMode(den_ngu, OUTPUT);
  pinMode(quat_ngu, OUTPUT);
  pinMode(button[0], INPUT);
  pinMode(button[1], INPUT);
  pinMode(button[2], INPUT);
  pinMode(button[3], INPUT);
  pinMode(cbas, INPUT);
  Serial.begin(115200); 
  //lcd.init();
  //lcd.backlight();
  rem.attach(rem_ngu,500,2400);
  for (pos = 100; pos < 179; pos++) 
  {
    rem.write(pos);
    delay(20);
  }
  delay(500);
}


void loop() 
{
  data_send = 0;
  ONLINE();
  REDING_BUTTON();
  DEN_HIEN();
  DEN_VA_QUAT();
  REM_NGU();
  //LCD_PHONG_NGU();
  nhiet_do = (int)((3.3 * analogRead(cbnd) * 100.0 / 1024.0) - 2.0);
  Serial.println(nhiet_do);
  Serial.write(255);
  Serial.write(data_send);
  Serial.write(nhiet_do);
}
void DEN_HIEN()
{
  if (digitalRead(cbas) == 0) {digitalWrite(den_hien, 0); }
  else{digitalWrite(den_hien, 1);}
}
void REM_NGU()
{
  if(state_drive[0]==1)//1 la che do auto cua rem
  {
    //
    if (digitalRead(cbas) == 0) { // 0 la troi sang
      while (pos < 179) {
        pos++;
        rem.write(pos);
        delay(20); 
      }
      data_send = data_send * 2 + 0;
      //digitalWrite(rem_ngu, 1);
    }
    if (digitalRead(cbas) == 1) { // 1 la troi toi
      while (pos > 0) {
        pos--;
        rem.write(pos);
        delay(20);
      }
      data_send = data_send * 2 + 1;
      //digitalWrite(rem_ngu, 0);
    }
    data_send = data_send * 2 + 1;
  }else if(state_drive[0]==0) //0 la che do thuong cua rem
  {
    if (state_drive[1] == 0) { // 0 la rem dong 
      while (pos < 179) {
        pos++;
        rem.write(pos);
        delay(20); 
      }
      data_send = data_send * 2 + 0;
      //digitalWrite(rem_ngu, 1);
    }
    if (state_drive[1] == 1) { // 1 la rem mo
      while (pos > 0) {
        pos--;
        rem.write(pos);
        delay(20);
      }
      data_send = data_send * 2 + 1;
    }
    data_send = data_send * 2 + 0;
  }
}

void DEN_VA_QUAT() {
  // BAT/TAT den
  if ( state_drive[2]==1 ) {    
      digitalWrite(den_ngu, 1); // bat len
      data_send = 1;
    }
    else {
      digitalWrite(den_ngu, 0); // tat di
      data_send = 0;
    }
  // BAT/TAT quat
    if (state_drive[3]==1) {
      digitalWrite(quat_ngu, 1); // Bat len
      data_send = data_send * 2 + 1;
    }
    else {
      digitalWrite(quat_ngu, 0); // tat di
      data_send = data_send * 2 + 0;
    }
}
void ONLINE()
{
  if (Serial.available()) {
      data_receive = Serial.read() - 48;
    //Serial1.println(data_receive);
    //int b1 = Serial1.read();
    //int b2 = Serial1.read();
    //data_receive = b1 * 256 + b2;

    switch (data_receive) {
      case 0: // Tat den
        digitalWrite(den_ngu, 0);
        state_drive[2] = 0;
        break;
      case 1:// Bat den
        digitalWrite(den_ngu, 1);
        state_drive[2] = 1;
        break;
      case 2: // Tat quat
        digitalWrite(quat_ngu, 0);
        state_drive[3] = 0;
        break;
      case 3: //Bat quat
        digitalWrite(quat_ngu, 1);
        state_drive[3] = 1;
        break;
      case 4: // Kéo rèm và tắt chế độ tự động// 0 la rèm đóng,180 là rèm mở
        while (pos >1) {
          pos--;
          rem.write(pos);
          delay(20);
        }
        state_drive[1]=0;
        state_drive[0] = 0; //rèm đóng và tắt chế độ tự động
        break;
        case 5: // mo rèm và tắt chế độ tự động// 0 la rèm đóng,180 là rèm mở
         while (pos < 179) {
          pos++;
          rem.write(pos);
          delay(20);
        }
        state_drive[1]=1;
        state_drive[0] = 0; //rèm đóng và tắt chế độ tự động
        break;
      case 6: //Bật chế độ tự động điều chỉnh của rèm
        state_drive[0] = 1;
    }
  }
}
/*void LCD_PHONG_NGU() {
  nhiet_do = (3.3 * analogRead(cbnd) * 100.0 / 1024.0) - 2.0;
  //Serial1.println(nhiet_do);              //Xuất trạng thái 
  //delay(200);
  //lcd.clear();
  //lcd.print("Nhiet do: ");
 // lcd.print(nhiet_do);
  //lcd.print("*C");
}*/
void REDING_BUTTON()
{
  for ( int i = 0; i < N0_OF_BUTTONS ; i ++)
	{
		debounceButtonBuffer[i] = digitalRead(button[i]);
    //Serial1.println(digitalRead(button[2]));
    if(debounceButtonBuffer[i]==0){state_drive[i]=!state_drive[i];}         	
  }
  delay(200);
}