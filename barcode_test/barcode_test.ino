#include <usbhid.h>
#include <usbhub.h>
#include <hiduniversal.h>
#include <hidboot.h>
#include <SPI.h>
#include<LiquidCrystal.h> //LCD 라이브러리를 추가
#include<String.h>
LiquidCrystal lcd(2,3,4,5,6,7);

char barcode_number[14];
int count=0;
int total_price=0;
int price=0;

class MyParser : public HIDReportParser {
  public:
    MyParser();
    void Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);
  protected:
    uint8_t KeyToAscii(bool upper, uint8_t mod, uint8_t key);
    virtual void OnKeyScanned(bool upper, uint8_t mod, uint8_t key);
    virtual void OnScanFinished();
};

MyParser::MyParser() {}

void MyParser::Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
  // If error or empty, return
  if (buf[2] == 1 || buf[2] == 0) return;

  for (uint8_t i = 7; i >= 2; i--) {
    // If empty, skip
    if (buf[i] == 0) continue;

    // If enter signal emitted, scan finished
    if (buf[i] == UHS_HID_BOOT_KEY_ENTER) {
      OnScanFinished();
    }

    // If not, continue normally
    else {
      // If bit position not in 2, it's uppercase words
     OnKeyScanned(i > 2, buf[i], buf[i]);
    }

    return;
  }
}

uint8_t MyParser::KeyToAscii(bool upper, uint8_t mod, uint8_t key) {
  // Letters
  if (VALUE_WITHIN(key, 0x04, 0x1d)) {
    if (upper) return (key - 4 + 'A');
    else return (key - 4 + 'a');
  }

  // Numbers
  else if (VALUE_WITHIN(key, 0x1e, 0x27)) {
    return ((key == UHS_HID_BOOT_KEY_ZERO) ? '0' : key - 0x1e + '1');
  }

  return 0;
}

void MyParser::OnKeyScanned(bool upper, uint8_t mod, uint8_t key) {
  uint8_t ascii = KeyToAscii(upper, mod, key);
  
  barcode_number[count] = (char)ascii;
  count++;
}

void MyParser::OnScanFinished() {
  barcode_number[count] = NULL;
  lcd.clear();
  count =0;
  price = pricelist(barcode_number);
  total_price = total_price + price; 
  lcd.setCursor(0, 0);
  lcd.print("Product: ");
  lcd.setCursor(9, 0);
  lcd.print(price);
  lcd.setCursor(0,1);
  lcd.print("Total: ");
  lcd.setCursor(7,1);
  lcd.print(total_price);

}

int pricelist(String barcode_number){
  int tmp_price=0;
  if(barcode_number.equals("8801117798208"))
      tmp_price =1200;
 
  return tmp_price;
}

USB          Usb;
USBHub       Hub(&Usb);
HIDUniversal Hid(&Usb);
MyParser     Parser;


void setup() {
  Serial.begin( 115200 );
  lcd.begin(16, 2); //16열 2행짜리 LCD를 사용하겠다.
  Serial.println("Start");

  if (Usb.Init() == -1) {
    Serial.println("OSC did not start.");
  }
  delay( 200 );

  Hid.SetReportParser(0, &Parser);

  lcd.setCursor(5, 0);  //커서를 (5, 0)으로 보내라
  lcd.print("Hello!!"); //(5, 0)부터 Hello!!를 찍어라
  lcd.setCursor(1,1);   //커서를 (1,1)로 옮겨라
  lcd.print("Team 6 : B.B.B"); //(1,1)부터 Codingrun.com을 찍어라.
}

void loop() {
  Usb.Task();
}
