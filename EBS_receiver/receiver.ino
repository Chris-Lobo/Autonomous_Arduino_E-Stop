#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <avr/io.h>
#include <util/delay.h>

// Arduino Nano RF Stop Transmitter

RF24 radio(7, 8);  // CE, CSN
const byte address[6] = "49055"; //last digits of tofuu in ascii lol (probably safer than before which was 00001)

#define LED_GO 4 
#define LED_STOP 3
#define SIGNAL_LOSS_TIMEOUT 1000

bool go = 0x00;
volatile unsigned long last_packet_millis = 0;

//SETUP--------------------------------------------------------------------------------------------------------------------------
void SetupRadio() {
  while (!radio.begin()) {
    asm(nop);
  }

  radio.setAutoAck(true);

  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW); //maybe comment this out to increase range if it still doesnt work
  radio.startListening();
}
void SetupLEDs() {
  pinMode(LED_GO, OUTPUT);
  pinMode(LED_STOP, OUTPUT);
}
//::SETUP::--------------------------------------------------------------------------------------------------------------------------

void Looper() {
  if (radio.available()) {
    radio.read(&go, sizeof(go));
    last_packet_millis = millis();//update timestamp upon receiving a message

    digitalWrite(LED_GO, go ? HIGH : LOW);
    digitalWrite(LED_STOP, go ? LOW : HIGH);
  } 
  else {
    //if no message is received in x ms, go safe mode
    if (millis() - last_packet_millis > SIGNAL_LOSS_TIMEOUT) {
      digitalWrite(LED_GO, LOW);
      digitalWrite(LED_STOP, HIGH);
    }
  }
}


int main() {
  cli();
  SetupRadio();
  SetupLEDs();
  sei();

  while (1) {
    Looper();
  }
}