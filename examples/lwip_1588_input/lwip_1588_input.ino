#include "lwip_t41.h"

// hardware: GPS on RX1 (pin 0), TX1 (pin 1), and PPS on pin 35

// 9600 and 4800 are other common GPS bitrates
#define GPS_BITRATE 115200

elapsedMillis msec;

#define ENET_TCSR_TPWC(n)          (((n) & 0b11111)<<11)
#define ENET_TCSR_TF               (1<<7)
#define ENET_TCSR_TIE              (1<<6)
#define ENET_TCSR_TMODE_MASK       (0b1111<<2)
#define ENET_TCSR_TMODE_DISABLED   0
#define ENET_TCSR_TMODE_IC_RISING  (0b0001<<2)
#define ENET_TCSR_TMODE_IC_FALLING (0b0010<<2)
#define ENET_TCSR_TMODE_IC_BOTH    (0b0011<<2)
#define ENET_TCSR_TMODE_OC_SOFT    (0b0100<<2)
#define ENET_TCSR_TMODE_OC_TOGGLE  (0b0101<<2)
#define ENET_TCSR_TMODE_OC_CLEAR   (0b0110<<2)
#define ENET_TCSR_TMODE_OC_SET     (0b0111<<2)
#define ENET_TCSR_TMODE_OC_CCSO    (0b1010<<2)
#define ENET_TCSR_TMODE_OC_SCCO    (0b1011<<2)
#define ENET_TCSR_TMODE_OC_PULSE0  (0b1110<<2)
#define ENET_TCSR_TMODE_OC_PULSE1  (0b1111<<2)

static uint32_t irq_count = 0;
static void interrupt_1588_timer() {
  irq_count++;
  if(ENET_TCSR0 & ENET_TCSR_TF) ENET_TCSR0 = ENET_TCSR0;
  asm volatile ("dsb"); // allow write to complete so the interrupt doesn't fire twice
}

// initialize the ethernet hardware
void setup()
{
  Serial1.begin(GPS_BITRATE);

  while (!Serial) ; // wait for usb serial
  Serial.println("Ethernet 1588 Input Capture");
  Serial.println("------------------------\n");

  enet_init(NULL, NULL, NULL);

  attachInterruptVector(IRQ_ENET_TIMER, interrupt_1588_timer);
  NVIC_ENABLE_IRQ(IRQ_ENET_TIMER);

  // disable channel 0 to configure it
  ENET_TCSR0 = 0;

  // GPS PPS
  // peripherial: ENET_1588_EVENT0_IN
  // IOMUX: ALT3
  // teensy pin: 35
  IOMUXC_SW_MUX_CTL_PAD_GPIO_B1_12 = 3;
  // route B1_12 to 1588 timer
  IOMUXC_ENET0_TIMER_SELECT_INPUT = 0b10;

  // wait for disable to cross clock domains
  while(ENET_TCSR0 & ENET_TCSR_TMODE_MASK) {}
  // enable rising edge and interrupts from GPS PPS
  ENET_TCSR0 = ENET_TCSR_TMODE_IC_RISING | ENET_TCSR_TIE;

  msec = 0;
}

void sample() {
    Serial.print(read_1588_timer());
    Serial.print(" at ");
    Serial.println(msec);
}

void loop()
{
  if (msec >= 1000) {
    sample();
    Serial.print(irq_count);
    Serial.print(" C=");
    Serial.print(ENET_TCCR0);
    Serial.print(" S=");
    Serial.print(ENET_TCSR0);
    Serial.print(" G=");
    Serial.print(ENET_TGSR);
    Serial.print(" E=");
    Serial.println(ENET_EIR);
    msec = 0;
  }

  // copy GPS uart to USB
  char incomingByte;
  if (Serial1.available() > 0) {
    incomingByte = Serial1.read();
    Serial.print(incomingByte);
  }
}
