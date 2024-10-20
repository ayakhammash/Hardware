#include <SPI.h>
#include <RFID.h>
#include <Servo.h>
#include <Keypad.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

#define START 0
#define QUANTITY 1
#define PAYMENT 2
#define DESPENSE 3

#define H_MOTOR_DIR 2
#define H_MOTOR_STEP 3

#define V_MOTOR_DIR 4
#define V_MOTOR_STEP 5

#define V_SWITCH 22
#define H_SWITCH 23

#define ROW_NUM 4
#define COLUMN_NUM 4

#define RFID_PIN_SDA 53
#define RFID_PIN_RST 49

#define BUZZER 13

#define TRIG 34
#define ECHO 36

#define LOCK 24
#define LIGHT 26

#define IR 6

#define SERVO1 11
#define SERVO2 10
#define SERVO3 9
#define SERVO4 8

char keys[ROW_NUM][COLUMN_NUM] =
{
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

byte pin_rows[ROW_NUM] = {37, 39, 41, 43};
byte pin_column[COLUMN_NUM] = {29, 31, 33, 35};

LiquidCrystal_I2C lcd(0x27, 20, 4);
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);
RFID rfid(RFID_PIN_SDA, RFID_PIN_RST);
Servo fisrt, second, third, fourth;

Servo servos[4];

struct Product 
{
  int slot = 0;
  int amount = 0;
  int price = 0;
  String name = "";
};

struct Products 
{
  Product products[4];
} currentProducts;

int state = START;

int index = 0;
bool selected[4] = {false, false, false, false};
int quantity[4] = {1, 1, 1, 1};
int totalPrice = 0;

int x[4] = {20, 20, 500, 500};
int y[4] = {700, 1200, 1200, 700};

bool lockState = LOW;

void setup() 
{
  Serial.begin(115200);
  SPI.begin();

  rfid.init();

  lcd.begin();
  lcd.backlight();
  lcd.clear();

  pinMode(H_MOTOR_DIR,OUTPUT);
  pinMode(H_MOTOR_STEP,OUTPUT);

  pinMode(V_MOTOR_DIR,OUTPUT);
  pinMode(V_MOTOR_STEP,OUTPUT);

  digitalWrite(H_MOTOR_DIR,LOW);
  digitalWrite(H_MOTOR_STEP,LOW);
  digitalWrite(V_MOTOR_DIR,LOW);
  digitalWrite(V_MOTOR_STEP,LOW);
  
  pinMode(H_SWITCH,INPUT_PULLUP);
  pinMode(V_SWITCH,INPUT_PULLUP);

  pinMode(BUZZER, OUTPUT);
  pinMode(LOCK, OUTPUT);
  pinMode(LIGHT, OUTPUT);

  digitalWrite(BUZZER,LOW);
  digitalWrite(LOCK,lockState);
  digitalWrite(LIGHT,HIGH);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  
  pinMode(IR, INPUT);

  fisrt.attach(SERVO1);
  second.attach(SERVO2); 
  third.attach(SERVO3); 
  fourth.attach(SERVO4);

  servos[0] = fisrt;
  servos[1] = second;
  servos[2] = third;
  servos[3] = fourth;

  initialPosition();
}

void loop() 
{
  digitalWrite(LIGHT, (msrDistance() < 40));

  if((state != PAYMENT) && scanCard() == "96939427")
  {
    lockState = !lockState;
    digitalWrite(LOCK, HIGH);
    delay(2000);
    digitalWrite(LOCK, LOW);
  }

  if (Serial.available() > 0) 
  {
    String fromesp = Serial.readStringUntil('\n');

    if(state == START && (fromesp.startsWith("first") || fromesp.startsWith("second") || fromesp.startsWith("third") || fromesp.startsWith("fourth")))
    {
      if(fromesp.startsWith("first"))
      {
        selected[0] = true;
      }
      else if(fromesp.startsWith("second"))
      {
        selected[1] = true;
      }
      else if(fromesp.startsWith("third"))
      {
        selected[2] = true;
      }
      else if(fromesp.startsWith("fourth"))
      {
        selected[3] = true;
      }

      lcd.clear();
      state = PAYMENT;
    }
    else
    {
      char separator = ',';
      const int maxItems = 4;
      String resultArray[4];
      splitString(fromesp, separator, resultArray, maxItems);

      int slot = resultArray[0].toInt();
      int amount = resultArray[1].toInt();
      int price = resultArray[2].toInt();
      String name = resultArray[3];

      currentProducts.products[slot - 1].slot = slot;
      currentProducts.products[slot - 1].amount = amount;
      currentProducts.products[slot - 1].price = price;
      currentProducts.products[slot - 1].name = name;

      currentProducts.products[slot - 1].name.trim();

      if(state == START)
      {
        lcd.clear();
      }
    }
  }

  switch(state)
  {
    case(START):
    {
      lcd.setCursor(0,0);
      lcd.print(String((selected[0] ? "*" : "")) + "A: " + currentProducts.products[0].name + " " + String(currentProducts.products[0].price) + "$");

      lcd.setCursor(0,1);
      lcd.print(String((selected[1] ? "*" : "")) + "B: " + currentProducts.products[1].name + " " + String(currentProducts.products[1].price) + "$");

      lcd.setCursor(0,2);
      lcd.print(String((selected[2] ? "*" : "")) + "C: " + currentProducts.products[2].name + " " + String(currentProducts.products[2].price) + "$");

      lcd.setCursor(0,3);
      lcd.print(String((selected[3] ? "*" : "")) + "D: " + currentProducts.products[3].name + " " + String(currentProducts.products[3].price) + "$");

      char key = keypad.getKey();

      if(key != NO_KEY)
      {
        if(key == 'A')
        {
          if(currentProducts.products[0].amount == 0) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("OUT OF STOCK");
            delay(2000);
            lcd.clear();
          } else {
            selected[0] = !selected[0];
          }
        }
        else if(key == 'B')
        {
          if(currentProducts.products[1].amount == 0) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("OUT OF STOCK");
            delay(2000);
            lcd.clear();
          } else {
            selected[1] = !selected[1];
          }
        }
        else if(key == 'C')
        {
          if(currentProducts.products[2].amount == 0) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("OUT OF STOCK");
            delay(2000);
            lcd.clear();
          } else {
            selected[2] = !selected[2];
          }
        }
        else if(key == 'D')
        {
          if(currentProducts.products[3].amount == 0) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("OUT OF STOCK");
            delay(2000);
            lcd.clear();
          } else {
            selected[3] = !selected[3];
          }
        }
        else if(key == '#')
        {
          index=0;
          for(int i = 0 ; i < 4 ; i++) {
            if(selected[i]) {
              state = QUANTITY;
              delay(2000);
              lcd.clear();
            }
          }
        }
        else if(key == '*')
        {
          for(int i = 0 ; i < 4 ; i++) {
            selected[i] = false;
          }
        }
      }

      break;
    }
    case(QUANTITY):
    {
      if(!selected[index]) {
        index++;
        break;
      }

      if(index > 3) {
        state = PAYMENT;
        delay(2000);
        lcd.clear();
        break;
      }

      lcd.setCursor(0,0);
      lcd.print(currentProducts.products[index].name);
      
      lcd.setCursor(0, 1);
      lcd.print("AMOUNT: " + String(quantity[index]));

      lcd.setCursor(0, 2);
      lcd.print("A: + B: -");

      lcd.setCursor(0, 3);
      lcd.print("#: OK *: CANCEL");

      char key = keypad.getKey();

      if(key != NO_KEY)
      {
        if(key == 'A')
        {
          if(quantity[index] < currentProducts.products[index].amount)
          {
            quantity[index]++;
          }
        }
        else if(key == 'B')
        {
          if(quantity[index] > 1)
          {
            quantity[index]--;
          }
        }
        else if(key == '#')
        {
          totalPrice += quantity[index] * currentProducts.products[index].price;
          index++;

          if(index > 3) {
            state = PAYMENT;
            delay(2000);
            lcd.clear();
          }
        }
        else if(key == '*')
        {
          index = 0;
          totalPrice = 0;
          for(int i = 0 ; i < 4 ; i++) {
            quantity[i] = 1;
            selected[i] = false;
          }

          state = START;
          delay(2000);
          lcd.clear();
        }
      }

      break;
    }
    case(PAYMENT):
    {
      lcd.setCursor(0, 0);
      lcd.print("TOTAL: " + String(totalPrice) + "$");

      lcd.setCursor(0, 1);
      lcd.print("SCAN YOUR CARD");

      lcd.setCursor(0, 2);
      lcd.print("*: CANCEL");

      char key = keypad.getKey();

      if(key != NO_KEY)
      {
        if(key == '*')
        {
          index = 0;
          totalPrice = 0;
          for(int i = 0 ; i < 4 ; i++) {
            quantity[i] = 1;
            selected[i] = false;
          }

          state = START;
          delay(2000);
          lcd.clear();
          break;
        }
      }

      String id = scanCard();
      if(id != "-1") {
        Serial.println(id + "_" + String(totalPrice));
        Serial.flush();
        delay(1000);

        while(!Serial.available())
        {
          delay(1);
        }

        String res = Serial.readStringUntil('\n');

        lcd.clear();
        lcd.setCursor(0, 0);

        if(res.startsWith("S"))
        {
          lcd.print("SUCCESS");
          state = DESPENSE;
          break;
        }
        else if(res.startsWith("F"))
        {
          lcd.print("INSUFFICIENT BALANCE");
        }
        else if(res.startsWith("U"))
        {
          lcd.print("UNKNOWN CARD");
        }

        index = 0;
        totalPrice = 0;
        for(int i = 0 ; i < 4 ; i++) {
          quantity[i] = 1;
          selected[i] = false;
        }

        state = START;
        delay(2000);
        lcd.clear();
      }

      break;
    }
    case(DESPENSE):
    {
      initialPosition();

      for(int i = 0 ; i < 4 ; i++)
      {
        if(selected[i])
        {
          stepperMovement(V_MOTOR_DIR,V_MOTOR_STEP,LOW,y[i]);
          stepperMovement(H_MOTOR_DIR,H_MOTOR_STEP,HIGH,x[i]);

          for(int j = 0 ; j < quantity[i] ; j++)
          {
            despense(servos[i]);
            delay(1000);
          }

          currentProducts.products[i].amount = currentProducts.products[i].amount - quantity[i];
          initialPosition();
        }
      }

      digitalWrite(BUZZER, HIGH);
      Serial.println(String(currentProducts.products[0].amount) + "," + String(currentProducts.products[1].amount) + "," + String(currentProducts.products[2].amount) + "," + String(currentProducts.products[3].amount));
      Serial.flush();
      
      index = 0;
      totalPrice = 0;
      for(int i = 0 ; i < 4 ; i++) {
        quantity[i] = 1;
        selected[i] = false;
      }

      delay(5000);
      digitalWrite(BUZZER, LOW);

      lcd.clear();
      state = START;

      break;
    }
  }
}

void initialPosition()
{
	while(digitalRead(H_SWITCH))
		stepperMovement(H_MOTOR_DIR,H_MOTOR_STEP,LOW,1);
	
	while(digitalRead(V_SWITCH))
		stepperMovement(V_MOTOR_DIR,V_MOTOR_STEP,HIGH,1);
}

void stepperMovement(int dirPin,int stepPin,int dir,int steps)
{
  digitalWrite(dirPin,dir);

  for(int i = 0 ; i < steps ; i++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1000);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(1000);
  }
}

void despense(Servo servo)
{
  servo.write(180);
  delay(1200); 
  servo.write(90);
  delay(2000);
}

long msrDistance() 
{
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long duration = pulseIn(ECHO, HIGH);
  long distance = duration * 0.034 / 2;
  return distance;
}

int splitString(String str, char separator, String *resultArray, int maxItems) 
{
  int currentIndex = 0;
  int previousIndex = 0;
  int itemCount = 0;

  while (currentIndex < str.length() && itemCount < maxItems) {
    currentIndex = str.indexOf(separator, previousIndex);
    if (currentIndex == -1) {
      resultArray[itemCount++] = str.substring(previousIndex);
      break;
    }
    resultArray[itemCount++] = str.substring(previousIndex, currentIndex);
    previousIndex = currentIndex + 1;
  }

  return itemCount;
}

String scanCard()
{
  if(rfid.isCard()) 
  {
    if(rfid.readCardSerial()) 
    {
      String cardId = String(rfid.serNum[0]) + String(rfid.serNum[1]) + String(rfid.serNum[2]) + String(rfid.serNum[3]);
      return cardId;
    }
    rfid.halt();
  }

  return "-1";
}