//NETWORKING 3201
//LAB 5
//IAN WOODLY, DALTON FOX, THOMAS GILMAN
// 2/20/18
#define BPS 9600
#define TIME 1000.0f / BPS
#define CE 7
#define CSN 8
#define SCK 13
#define MOSI 11
#define MISO 12

void OGDelay(float t)
{
  int mi = (int)t;
  if (mi) delay(mi);

  t -= mi;
  float mc = t * 1000;
  if (mc) delayMicroseconds(mc);
}

void config_tx(void)
{
  Serial.println("Configuring Tx");
  write_register(0x00, 2);
  Serial.print("Register:0x00 = ");Serial.println(read_register(0x00));
  
  write_register(0x05, 3);
  Serial.print("Register:0x05 = ");Serial.println(read_register(0x05));
  
  write_register(0x01, 0);
  Serial.print("Register:0x01 = ");Serial.println(read_register(0x01));
  
  write_register(0x11, 32);
  Serial.print("Register:0x11 = ");Serial.println(read_register(0x11));
  
  write_tx_address("poddy");
  write_rx_address("poddy");
}

void config_rx(void)
{
  Serial.println("Configuring Rx");
  write_register(0x00, 3);
  Serial.print("Register:0x00 = ");Serial.println(read_register(0x00));
  write_register(0x05, 3);
  Serial.print("Register:0x05 = ");Serial.println(read_register(0x05));
  write_register(0x01, 0);
  Serial.print("Register:0x01 = ");Serial.println(read_register(0x01));
  write_register(0x11, 32);
  Serial.print("Register:0x11 = ");Serial.println(read_register(0x11));
  
  write_tx_address("poddy");
  write_rx_address("poddy");
}

void flush_tx(void)
{
  digitalWrite(CSN, LOW);
  soft_spi(0xE1);
  digitalWrite(CSN, HIGH);
}

void flush_rx(void)
{
  digitalWrite(CSN, LOW);
  soft_spi(0xE2);
  digitalWrite(CSN, HIGH);
}

unsigned char read_register(unsigned char reg)
{
  digitalWrite(CSN, LOW);
  soft_spi(reg);
  unsigned char retval = soft_spi(reg);
  digitalWrite(CSN, HIGH);
  
  return retval;
}

void write_tx_address(const char * ch_array)
{
  digitalWrite(CSN, LOW);

  soft_spi(0x20 | 0x10); // Write starting at 0x10;

  for (int i = 0; i < 5; i++) // Sending to a 5 Byte register
  {
    soft_spi(ch_array[i]);
  }
  
  digitalWrite(CSN, HIGH);
}

void write_rx_address(const char * ch_array)
{
  digitalWrite(CSN, LOW);
  
  soft_spi(0x20 | 0x0A); // Write Starting at 0x0A;

  for (int i = 0; i < 5; i++) // Sending to a 5 Byte register
  {
    soft_spi(ch_array[i]);
  }
  
  digitalWrite(CSN, HIGH);
}

void write_payload(unsigned char * payload)
{
  digitalWrite(CSN, LOW);
  
  soft_spi(0xA0);
  for (int i = 0; i < 32; i++) // Write the 32 payload bytes over SPI
  {
    soft_spi(payload[i]);
  }
  
  digitalWrite(CSN, HIGH);
}

void read_payload(unsigned char * payload)
{
  digitalWrite(CSN, LOW);
  soft_spi(0x61);
  for (int i = 0; i < 32; i++)
  {
    payload[i] = soft_spi(0);    
  }
  digitalWrite(CSN, HIGH);
}

void write_register(unsigned char reg, unsigned char value)
{
  digitalWrite(CSN, LOW);

  
  soft_spi(0x20|reg); // Send values starting at register reg
  soft_spi(value);
  
  digitalWrite(CSN, HIGH);
}

unsigned char soft_spi(unsigned char send_byte) {
  unsigned char recv_byte = 0;
  int i;
  for (i=0; i < 8; i++) {
      digitalWrite(MOSI, (send_byte >> (7-i)) & 1);
      
      OGDelay(TIME); digitalWrite(SCK, HIGH);
      recv_byte |= (digitalRead(MISO) << (7 - i) );
      OGDelay(TIME); digitalWrite(SCK, LOW);
  }
  return recv_byte;
}

void setup() { //sets the pin in and out including the 
  Serial.begin(9600);
  pinMode(CE, OUTPUT); //Chip enable (for the radio signal chip)
  pinMode(CSN, OUTPUT); //Chip Select Not /SS
  pinMode(SCK, OUTPUT); //clock
  pinMode(MOSI, OUTPUT); //data in
  pinMode(MISO, INPUT); //data out

  digitalWrite(MOSI,HIGH); digitalWrite(SCK,LOW);
  digitalWrite(CSN,HIGH); digitalWrite(CE,HIGH);
}

void loop()
{
  unsigned char payload[32]; // array for payload data
  unsigned char status_reg = 0; // array to store status reg value.
  static int count = 0; // variable to store packet number.
  static int sending = false; // variable to store send state.
  static int listening = false; // variable to store listen state.
  unsigned char ch = 0; // variable to store key from serial.

 if(Serial.available()) 
  ch=Serial.read();
 if(ch == 'g')
 {
   config_tx();
   sending = true;
   Serial.println("Sender Go!");
   count=0;
 }
 else if(ch == 'l') //lower case L for listener
 {
  config_rx();
  listening = true;
  Serial.println("Listener Go!");
  count=0;
 }
 else if(ch == 's')
 {
  
   sending = false;
   listening = false;
   Serial.println("Stop!");
 }
 
 if(sending)
 {
  listening = false;

  sprintf((char*)payload,"MSG:%d\0",count++); //read in the string
  write_payload(payload);
  
  for(short i = 0; i < 32; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  flush_tx();
  delay(2000); // delay for 2 seconds.
 }
 
 if(listening)
 {
  sending = false;
  
  status_reg = read_register(0x07);
  if(status_reg & 0x40)
  {
    read_payload(payload);
    for(short i = 0; i < 32; i++)
    {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    flush_rx();
    write_register(0x07, 0x40);
  }
 }
}
