#include "SPI.h"

/*#include "pio_usb.h"
#define HOST_PIN_DP 16  // Pin used as D+ for host, D- = D+ + 1*/

#include "EZ_USB_MIDI_HOST.h"

Adafruit_USBH_Host USBHost;
USING_NAMESPACE_MIDI
USING_NAMESPACE_EZ_USB_MIDI_HOST

#define SPI_SCK 2
#define SPI_MOSI 3
#define SPI_MISO 4
#define SPI_CS 5
#define VS_DREQ 6
#define VS_RESET 7
#define VS_DCS 8

RPPICOMIDI_EZ_USB_MIDI_HOST_INSTANCE(usbhMIDI, MidiHostSettingsDefault)

/*bool core0_booting = true;
bool core1_booting = true;

void setup1() {
  while (!Serial1)
    ;
  Serial1.println("Starting core 1");

  pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
  pio_cfg.pin_dp = HOST_PIN_DP;
  USBHost.configure_pio_usb(1, &pio_cfg);
  Serial1.println("USB initialised");

  core1_booting = false;
  while (core0_booting)
    ;
}

void loop1() {
  USBHost.task();
}*/

void setup() {
  Serial1.begin(115200);
  while (!Serial1)
    ;
  Serial1.println("Starting core 0");

  SPI.setSCK(SPI_SCK);
  SPI.setMOSI(SPI_MOSI);
  SPI.setMISO(SPI_MISO);
  SPI.begin();
  Serial1.println("SPI initialised");

  vs_Begin();
  vs_loadMidiCode();
  Serial1.printf("VS1053b version %d initialised\n", vs_getVersion());

  /*core0_booting = false;
  while (core1_booting)
    ;*/

  //usbhMIDI.begin(&USBHost, 1, onMIDIconnect, onMIDIdisconnect);
  usbhMIDI.begin(&USBHost, 0, onMIDIconnect, onMIDIdisconnect);

  vs_sendMidi(0xC0, 20);  // Instrument
  // 20, 69, 60
  vs_sendMidi(0xB0, 7, 127);  // Channel volume
  Serial1.println("MIDI initialised");
}

void loop() {
  USBHost.task();
  usbhMIDI.readAll();
}

void onNoteOff(Channel channel, byte note, byte velocity) {
  Serial1.printf("C%u: Note off#%u v=%u\r\n", channel, note, velocity);
  vs_sendMidi(0x80, note, velocity);
}

void onNoteOn(Channel channel, byte note, byte velocity) {
  Serial1.printf("C%u: Note on#%u v=%u\r\n", channel, note, velocity);
  vs_sendMidi(0x90, note, velocity);
}

void registerMidiInCallbacks(uint8_t devAddr) {
  auto intf = usbhMIDI.getInterfaceFromDeviceAndCable(devAddr, 0);
  if (intf == nullptr)
    return;
  intf->setHandleNoteOff(onNoteOff);  // 0x80
  intf->setHandleNoteOn(onNoteOn);    // 0x90
}

void onMIDIconnect(uint8_t devAddr, uint8_t nInCables, uint8_t nOutCables) {
  Serial1.printf("MIDI device at address %u has %u IN cables and %u OUT cables\r\n", devAddr, nInCables, nOutCables);
  registerMidiInCallbacks(devAddr);
}

void onMIDIdisconnect(uint8_t devAddr) {
  Serial1.printf("MIDI device at address %u unplugged\r\n", devAddr);
}