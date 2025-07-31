#define ESP01_INIT_PIN PC6   // 31
#define ESP01_CLIENT_PIN PC7  // 30

/*
Hardik Kalasua OV7670 Code
modified by me to support Arduino Mega embedded with ESP8266
*/


#include <Wire.h>
#include <SD.h>
int CS_Pin = 13;
long file_counter = 0;

void Init_YUV422() {
  WriteOV7670(0x12, 0x00);  //COM7
  WriteOV7670(0x8C, 0x00);  //RGB444
  WriteOV7670(0x04, 0x00);  //COM1
  WriteOV7670(0x40, 0xC0);  //COM15
  WriteOV7670(0x14, 0x1A);  //COM9
  WriteOV7670(0x3D, 0x40);  //COM13
}

void Init_QVGA() {
  WriteOV7670(0x0C, 0x04);  //COM3 - Enable Scaling
  WriteOV7670(0x3E, 0x19);  //COM14
  WriteOV7670(0x72, 0x11);  //
  WriteOV7670(0x73, 0xF1);  //
  WriteOV7670(0x17, 0x16);  //HSTART
  WriteOV7670(0x18, 0x04);  //HSTOP
  WriteOV7670(0x32, 0xA4);  //HREF
  WriteOV7670(0x19, 0x02);  //VSTART
  WriteOV7670(0x1A, 0x7A);  //VSTOP
  WriteOV7670(0x03, 0x0A);  //VREF
}

void Init_OV7670() {
  //Reset All Register Values
  WriteOV7670(0x12, 0x80);
  delay(100);
  WriteOV7670(0x3A, 0x04);  //TSLB

  WriteOV7670(0x13, 0xC0);  //COM8
  WriteOV7670(0x00, 0x00);  //GAIN
  WriteOV7670(0x10, 0x00);  //AECH
  WriteOV7670(0x0D, 0x40);  //COM4
  WriteOV7670(0x14, 0x18);  //COM9
  WriteOV7670(0x24, 0x95);  //AEW
  WriteOV7670(0x25, 0x33);  //AEB
  WriteOV7670(0x13, 0xC5);  //COM8
  WriteOV7670(0x6A, 0x40);  //GGAIN
  WriteOV7670(0x01, 0x40);  //BLUE
  WriteOV7670(0x02, 0x60);  //RED
  WriteOV7670(0x13, 0xC7);  //COM8
  WriteOV7670(0x41, 0x08);  //COM16
  WriteOV7670(0x15, 0x20);  //COM10 - PCLK does not toggle on HBLANK
}

void WriteOV7670(byte regID, byte regVal) {
  // Slave 7-bit address is 0x21.
  // R/W bit set automatically by Wire functions
  // dont write 0x42 or 0x43 for slave address
  Wire.beginTransmission(0x21);
  // Reset all register values
  Wire.write(regID);
  Wire.write(regVal);
  Wire.endTransmission();
  delay(1);
}

void ReadOV7670(byte regID) {
  // Reading from a register is done in two steps
  // Step 1: Write register address to the slave
  // from which data is to be read.
  Wire.beginTransmission(0x21);  // 7-bit Slave address
  Wire.write(regID);             // reading from register 0x11
  Wire.endTransmission();

  // Step 2: Read 1 byte from Slave
  Wire.requestFrom(0x21, 1);
  Serial.print("Read request Status:");
  Serial.println(Wire.available());
  Serial.print(regID, HEX);
  Serial.print(":");
  Serial.println(Wire.read(), HEX);
}

void XCLK_SETUP(void) {
  pinMode(11, OUTPUT);  //Set pin 11 to output

  //Initialize timer 1

  //WGM13, WGM12, WGM11 & WGM10 bits SET- Fast PWM mode
  //COM1A0 SET- Toggle OC1A on compare match
  TCCR1A = (1 << COM1A0) | (1 << WGM11) | (1 << WGM10);
  //SET CS10 bit for clock select with no prescaling
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);
  //Output Compare Register 1A(OCR1A) = 0
  //This will lead to a match on every clock cycle
  //Toggle OC1A output pin on every match instance
  //Therefore, the generated waveform will have half
  //the frequency of the driving clock i.e. 8Mhz
  //OC1A pin- PB1 (alternate functn) pin i.e. Arduino pin 9
  OCR1A = 0;
}

// Two Wire Interface Setup
// Sets the frequency of the SCL line
// Default is 100KHz so we won't use this function
void TWI_SETUP(void) {
  //Set prescaler bits in TWI Status Register (TWSR) to 0
  TWSR &= ~3;
  //Set SCL frequency to 100KHz
  //SCLfreq = CPUfreq/(16 + 2(TWBR) - 4^(TWPS))
  //TWBR = 72, TWPS(prescaler) = 0
  TWBR = 72;
}

void OV7670_PINS(void) {
  // Full Port A.
  DDRA &= ~0xFF;

  DDRE &= ~(1 << PE4);  // PCLK
  DDRE &= ~(1 << PE5);  // VSync
}



void QVGA_Image(String title) {
  int h, w;

  File dataFile = SD.open(title, O_WRITE | O_CREAT);
  while (!(PINE & 32))
    ;  //wait for high
  while ((PINE & 32))
    ;  //wait for low

  h = 240;
  while (h--) {
    w = 320;
    byte dataBuffer[320];
    while (w--) {
      while ((PINE & 16))
        ;  //wait for low
      dataBuffer[319 - w] = PINA;
      while (!(PINE & 16))
        ;  //wait for high
      while ((PINE & 16))
        ;  //wait for low
      while (!(PINE & 16))
        ;  //wait for high
    }
    dataFile.write(dataBuffer, 320);
  }

  dataFile.close();
  delay(100);
}

inline void send_data_uart3(uint8_t data) __attribute__((always_inline));
void send_data(uint8_t data) {
  while (!(UCSR3A & (1 << UDRE3)));
  UDR3 = data;
}

void send_log(const char* log) {
  send_data(0x02);
  for (int i = 0; i < strlen(log); i++) {
    send_data(log[i]);
  }
  send_data(0xFF);
}

void setup() {
  DDRC &= ~(1 << ESP01_INIT_PIN);
  DDRC &= ~(1 << ESP01_CLIENT_PIN);

  noInterrupts();  //Disable all interrupts
  XCLK_SETUP();    //Setup 8MHz clock at pin 11
  OV7670_PINS();   // Setup Data-in and interrupt pins from camera
  delay(1000);
  TWI_SETUP();  // Setup SCL for 100KHz
  interrupts();
  Wire.begin();


  Init_OV7670();
  Init_QVGA();
  Init_YUV422();
  WriteOV7670(0x11, 0x1F);  //Range 00-1F
  noInterrupts();
  Serial3.begin(2000000);
  pinMode(CS_Pin, OUTPUT);
  SD.begin(CS_Pin);
}

void loop() {
  String fileName = String(file_counter) + ".RID";
  QVGA_Image(fileName);
  file_counter++;

  // We will check for a client now and send data to it.
  if ((PINC & (1 << ESP01_CLIENT_PIN)) != 0) {
    File dataFile = SD.open(fileName, O_READ);
    send_log("Sending Image....");
    send_data(0x01);
    for (uint32_t image_byte = 0; image_byte < 76800; image_byte++) {
      send_data((uint8_t) dataFile.read());
      if (image_byte % 10000 == 0) _delay_ms(1);
    }
    dataFile.close();
  }
}