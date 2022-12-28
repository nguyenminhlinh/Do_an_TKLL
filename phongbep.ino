#include <Servo.h>

#define den_bep 14
#define quat_bep 13
#define buzz 12
#define day_phoi 2

#define cbmua 0
#define cbgas 4
#define cbnd A0


# define N0_OF_BUTTONS 2
// we define two buffers for debouncing
boolean state_drive[3] = {0};
//state_drive[0] la trang thai den dong/mo
//state_drive[1] la trang thai quat dong/mo
//state_drive[2] la la che do cua day phoi auto/thuong
int button[N0_OF_BUTTONS]={16,5};
int debounceButtonBuffer[N0_OF_BUTTONS]={1};


int nhiet_do = 0, gas=0, i=0;
int pos = 179, data_send = 0, data_receive = 0;
Servo day;

void setup() {
  
  pinMode(den_bep, OUTPUT);
  pinMode(quat_bep, OUTPUT);
  pinMode(buzz, OUTPUT);
  pinMode(button[0], INPUT_PULLUP);
  pinMode(button[1], INPUT_PULLUP);
  pinMode(cbgas, INPUT);
  pinMode(cbnd, INPUT);
  pinMode(cbmua, INPUT);
  digitalWrite(den_bep, LOW);
  digitalWrite(quat_bep, LOW);
  digitalWrite(buzz, HIGH);
  Serial.begin(115200);
  day.attach(day_phoi,500,2400);
  for (pos = 100; pos < 179; ) 
    {
      pos++;    
      day.write(pos);
      delay(20);
    }
    delay(500);
  
}
void loop() {
  data_send = 0;
  ONLINE();
  REDING_BUTTON();
  DAY_PHOI();
  DEN_VA_QUAT();
  ALARM();
  Serial.write(255);
  Serial.write(data_send);
  Serial.write(nhiet_do);
}

void REDING_BUTTON()
{
  for ( int i = 0; i < N0_OF_BUTTONS ; i ++)
  {
    debounceButtonBuffer[i] = digitalRead(button[i]);
    //Serial.println(digitalRead(button[1]));
    if(debounceButtonBuffer[i]==0){state_drive[i]=!state_drive[i];}      
   // Serial.println(state_drive[1]);
  }
  delay(200);
}
void ONLINE() {
    if (Serial.available()) {
     while (Serial.available() )
    {
      data_receive=Serial.read()-48;
    }

    switch (data_receive) {
      case 0: // tat den
        digitalWrite(den_bep, 0);
        state_drive[0] = 0;
        break;
      case 1: // bat den
        digitalWrite(den_bep, 1);
        state_drive[0] = 1;
        break;
      case 2: // tat quat
        digitalWrite(quat_bep, 1); 
        state_drive[1] = 0;
        break;
      case 3: // bat quat
        digitalWrite(quat_bep, 0); 
        state_drive[1] = 1;
        break;
      case 4: // bat auto rem phoi
        digitalWrite(day_phoi, 1); 
        state_drive[2] = 1;
        break;
      case 5: // tat auto rem phoi
        digitalWrite(day_phoi, 0); 
        state_drive[2] = 0;
        break;
        
    }
  }
}
void DEN_VA_QUAT() {
  // BAT/TAT den
  if ( state_drive[0]==1 ) {    
      digitalWrite(den_bep, 1); // bat len
      //Serial.write('1');
      data_send += 1;
    }
    else {
      digitalWrite(den_bep, 0); // tat di
      data_send = 0;
      //Serial.write('0');
      data_send += 0;
    }
  // BAT/TAT quat
    if (state_drive[1]==1) {
      digitalWrite(quat_bep, 1); // Bat len
      //Serial.write('1');
      data_send = data_send * 2 + 1;
    }
    else {
      digitalWrite(quat_bep, 0); // tat di
      //Serial.write('0');
      data_send = data_send * 2 + 0;
    }
}
void ALARM() {
  //canh bao khi co khi gas
  nhiet_do = (int)(3.3 * 100.0 * analogRead(cbnd) / 1024.0+30);
  //gas =  3.3 * 100.0 * analogRead(cbnd) / 1024.0;
  if(digitalRead(cbgas))
  {
    gas = 1;
  }
  else
  {
    gas = 0;
  }
  Serial.println(digitalRead(cbgas));
  if (gas == 0 || nhiet_do > 60)
  {
    digitalWrite(buzz, HIGH);
    //state_drive[1]=1;
  }
  else
  {
    digitalWrite(buzz, LOW);
  }

  if (gas == 0 ) {
    //Serial.write('1');
    data_send = data_send * 2 + 1;
  }
  else {
    //Serial.write('0');
    data_send = data_send * 2 + 0;
  }
}
void DAY_PHOI() {
  if ((digitalRead(cbmua) == HIGH) && (pos == 179)) // ko co mua
    {
      while (pos > 0) 
      { 
        pos--;
        day.write(pos);              // xuất tọa độ ra cho servo
        delay(15);                       // đợi 15 ms cho servo quay đến góc đó rồi tới bước tiếp theo
      }
    }
    if ((digitalRead(cbmua) == LOW) && (pos == 0 )) // co mua
    {
      while (pos < 179) 
      {
        pos++;
        day.write(pos);              // xuất tọa độ ra cho servo
        delay(15);                       // đợi 15 ms cho servo quay đến góc đó rồi tới bước tiếp theo
      }
    }
}
