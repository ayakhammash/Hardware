#define BLYNK_TEMPLATE_ID "TMPL6f5LfsBGz"
#define BLYNK_TEMPLATE_NAME "vending machine"
#define BLYNK_AUTH_TOKEN "E7fkqxU6jgvA0CWmQ5xY6KpPRYRq8sBd"

#include <FS.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

int currentSlot = -1;
int currentQuantity = -1;
int currentPrice = -1;
String currentName = "";

int currentBalance = -1;
String currentCard = "";

const char SSID[] = "vending_machine";
const char PASS[] = "123456789";

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

BLYNK_WRITE(V0)
{
  currentSlot = param[0].asInt();
}

BLYNK_WRITE(V1)
{
  currentQuantity = param[0].asInt();
}

BLYNK_WRITE(V2)
{
  currentPrice = param[0].asInt();
}

BLYNK_WRITE(V3)
{
  currentName = param.asStr();
}

BLYNK_WRITE(V4)
{
  if(param[0].asInt() == 1)
  {
    if(currentSlot == -1)
      return;

    if(currentQuantity == -1)
      return;

    if(currentPrice == -1)
      return;
    
    currentName.trim();
    if(currentName.length() == 0)
      return;

    String newProduct = String(currentSlot) + "," + String(currentQuantity) + "," + String(currentPrice) + "," + currentName;

    replaceLine("/products.txt", currentSlot, newProduct.c_str());
    fetch("/products.txt");
  }
}

BLYNK_WRITE(V5)
{
  currentCard = param.asStr();
}

BLYNK_WRITE(V6)
{
  currentBalance = param[0].asInt();
}

BLYNK_WRITE(V7)
{
  if(param[0].asInt() == 1)
  {
    if(currentBalance == -1)
      return;
    
    currentCard.trim();
    if(currentCard.length() == 0)
      return;

    String fileName = currentCard;
    File file = LittleFS.open(fileName, "r");

    int fileBalance = 0;
    if (file) {
      file.seek(0);
      fileBalance = file.parseInt();
      file.close();
    } 

    file = LittleFS.open(fileName, "w");
    if (file) {
      file.println(fileBalance + currentBalance);
      file.close();
    }
  }
}

BLYNK_WRITE(V8)
{
  if(param[0].asInt() == 1)
  {
    Serial.println("first");
    Serial.flush();
  }
}

BLYNK_WRITE(V9)
{
  if(param[0].asInt() == 1)
  {
    Serial.println("second");
    Serial.flush();
  }
}

BLYNK_WRITE(V10)
{
  if(param[0].asInt() == 1)
  {
    Serial.println("third");
    Serial.flush();
  }
}

BLYNK_WRITE(V11)
{
  if(param[0].asInt() == 1)
  {
    Serial.println("fourth");
    Serial.flush();
  }
}

int i = 0;

void setup() 
{
  Serial.begin(115200);
  LittleFS.begin();

  if(!LittleFS.exists("/products.txt")) 
  {
    File file = LittleFS.open("/products.txt", "w");
    file.println("1,0,0,no_name1");
    file.println("2,0,0,no_name2");
    file.println("3,0,0,no_name3");
    file.println("4,0,0,no_name4");
    file.close();
  }

  Blynk.begin(BLYNK_AUTH_TOKEN, SSID, PASS);
  fetch("/products.txt");

  Blynk.syncVirtual(V0);
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
  Blynk.syncVirtual(V5);
  Blynk.syncVirtual(V6);

  delay(10000);
}

void loop() 
{
  Blynk.run();

  Product p = currentProducts.products[i++];
  Serial.println(String(p.slot) + "," + String(p.amount) + "," + String(p.price) + "," + String(p.name));
  Serial.flush();
  
  if(i == 4)
    i = 0;

  delay(1500);

  if (Serial.available() > 0) 
  {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if(input.indexOf("_") != -1)
    {
      char separator = '_';
      const int maxItems = 2;
      String resultArray[2];
      splitString(input, separator, resultArray, maxItems);

      String card = resultArray[0];
      int total = resultArray[1].toInt();

      File file = LittleFS.open(card, "r");

      int fileBalance = 0;
      if (file) {
        file.seek(0);
        fileBalance = file.parseInt();
        file.close();

        if(fileBalance < total)
        {
          Serial.println("F");
          Serial.flush();
        }
        else
        {
          int newBalance = fileBalance - total;
          file = LittleFS.open(card, "w");
          if (file) {
            file.println(newBalance);
            file.close();
          }

          Serial.println("S");
          Serial.flush();
        }
      }
      else
      {
        delay(2000);
        Serial.println("U");
        Serial.flush();
      }
    }
    else if (input.indexOf(",") != -1)
    {
      char separator = ',';
      const int maxItems = 4;
      String resultArray[4];
      splitString(input, separator, resultArray, maxItems);

      if(!resultArray[0].toInt() || !resultArray[1].toInt() || !resultArray[2].toInt() || !resultArray[3].toInt())
      {
        Blynk.logEvent("vending_machine", "some products out of stock!");
      }

      String s0 = String(currentProducts.products[0].slot) + "," + resultArray[0] + "," + String(currentProducts.products[0].price) + "," + currentProducts.products[0].name;
      replaceLine("/products.txt", currentProducts.products[0].slot, s0.c_str());
      delay(300);
      String s1 = String(currentProducts.products[1].slot) + "," + resultArray[1] + "," + String(currentProducts.products[1].price) + "," + currentProducts.products[1].name;
      replaceLine("/products.txt", currentProducts.products[1].slot, s1.c_str());
      delay(300);
      String s2 = String(currentProducts.products[2].slot) + "," + resultArray[2] + "," + String(currentProducts.products[2].price) + "," + currentProducts.products[2].name;
      replaceLine("/products.txt", currentProducts.products[2].slot, s2.c_str());
      delay(300);
      String s3 = String(currentProducts.products[3].slot) + "," + resultArray[3]+ "," + String(currentProducts.products[3].price) + "," + currentProducts.products[3].name;
      replaceLine("/products.txt", currentProducts.products[3].slot, s3.c_str());
      delay(500);
      fetch("/products.txt");
    }
  }
}

void fetch(const char * path) 
{
  int count = 0;
  int index = 0;
  File file = LittleFS.open(path, "r");

  if (!file) 
    return;
  
  while (file.available() && count < 6) 
  {
    String strs[4] = {};
    int stringCount = 0;
    String line = file.readStringUntil('\n');
    
    while (line.length() > 0)
    {
      int index = line.indexOf(',');
      if (index == -1)
      {
        strs[stringCount++] = line;
        break;
      }
      else
      {
        strs[stringCount++] = line.substring(0, index);
        line = line.substring(index+1);
      }
    }

    Product p;
    p.slot = strs[0].toInt();
    p.amount = strs[1].toInt();
    p.price = strs[2].toInt();
    p.name = strs[3];

    count++;
    currentProducts.products[index++] = p;
  }

  file.close();
}

void replaceLine(const char * path, int lineNum, const char * newLine) 
{
  File file = LittleFS.open(path, "r");

  if (!file) 
    return;

  String fileContent;
  while (file.available()) 
    fileContent += (char)file.read();

  file.close();

  String line;
  std::vector<String> lines;

  for (int i = 0; i < fileContent.length(); i++) 
  {
    if (fileContent[i] == '\n') 
    {
      lines.push_back(line);
      line = "";
    } 
    else 
    {
      line += fileContent[i];
    }
  }

  if (line.length() > 0)
    lines.push_back(line);

  if (lineNum > 0 && lineNum <= lines.size()) 
    lines[lineNum - 1] = newLine;
  else
    return;

  String newFileContent;
  for (int i = 0; i < lines.size(); i++) 
  {
    newFileContent += lines[i];

    if (i < lines.size() - 1) 
      newFileContent += '\n';
  }

  file = LittleFS.open(path, "w");
  if (!file)
    return;
  
  file.print(newFileContent);

  file.close();
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