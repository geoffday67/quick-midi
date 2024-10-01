static const uint8_t COMMAND_WRITE = 0x02;
static const uint8_t COMMAND_READ = 0x03;

static const uint8_t REGISTER_MODE = 0x00;
static const uint8_t REGISTER_STATUS = 0x01;
static const uint8_t REGISTER_CLOCKF = 0x03;
static const uint8_t REGISTER_WRAM = 0x06;
static const uint8_t REGISTER_WRAM_ADDR = 0x07;
static const uint8_t REGISTER_VOL = 0x0B;

static const uint16_t SM_RESET = 0x0004;
static const uint16_t SM_CANCEL = 0x0008;
static const uint16_t SM_SDINEW = 0x0800;

void waitReady() {
  while (digitalRead(VS_DREQ) == 0)
    ;
}

uint16_t readRegister(uint8_t target) {
  uint8_t transmit[4], receive[4];
  uint16_t result;

  transmit[0] = COMMAND_READ;
  transmit[1] = target;
  transmit[2] = 0;
  transmit[3] = 0;

  waitReady();
  digitalWrite(SPI_CS, LOW);
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  SPI.transfer(transmit, receive, 4);
  SPI.endTransaction();
  digitalWrite(SPI_CS, HIGH);

  result = (receive[2] * 256) + receive[3];
  Serial1.printf("Reading 0x%04x from register 0x%02x\n", result, target);
  return result;
}

void writeRegister(uint8_t target, uint16_t value) {
  uint8_t transmit[4];

  transmit[0] = COMMAND_WRITE;
  transmit[1] = target;
  transmit[2] = value / 256;
  transmit[3] = value % 256;

  waitReady();
  digitalWrite(SPI_CS, LOW);
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  SPI.transfer(transmit, NULL, 4);
  SPI.endTransaction();
  digitalWrite(SPI_CS, HIGH);

  Serial1.printf("Writing 0x%04x to register 0x%02x\n", value, target);
}

void hardwareReset() {
  digitalWrite(VS_RESET, LOW);
  delay(10);
  digitalWrite(VS_RESET, HIGH);
  delay(10);
}

void softwareReset() {
  writeRegister(REGISTER_MODE, SM_SDINEW | SM_RESET);
  delay(10);
}

void vs_Begin() {
  pinMode(VS_DREQ, INPUT);

  pinMode(SPI_CS, OUTPUT);
  digitalWrite(SPI_CS, HIGH);

  pinMode(VS_RESET, OUTPUT);
  digitalWrite(VS_RESET, HIGH);

  pinMode(VS_DCS, OUTPUT);
  digitalWrite(VS_DCS, HIGH);

  Serial1.println("Pins initialised");

  hardwareReset();
  Serial1.println("Hardware reset");

  softwareReset();
  Serial1.println("Software reset");

  // Set the clock multiplier.
  writeRegister(REGISTER_CLOCKF, 0xC000);
  delay(10);
}

int vs_getVersion() {
  uint16_t value = readRegister(REGISTER_STATUS);
  return (value & 0x00F0) >> 4;
}

const uint16_t sVS1053b_Realtime_MIDI_Plugin[] = {
  0x0007,
  0x0001,
  0x8050,
  0x0006,
  0x0014,
  0x0030,
  0x0715,
  0xb080, /*    0 */
  0x3400,
  0x0007,
  0x9255,
  0x3d00,
  0x0024,
  0x0030,
  0x0295,
  0x6890, /*    8 */
  0x3400,
  0x0030,
  0x0495,
  0x3d00,
  0x0024,
  0x2908,
  0x4d40,
  0x0030, /*   10 */
  0x0200,
  0x000a,
  0x0001,
  0x0050,
};

void vs_loadMidiCode(void) {
  int i = 0;

  while (i < sizeof(sVS1053b_Realtime_MIDI_Plugin) / sizeof(sVS1053b_Realtime_MIDI_Plugin[0])) {
    uint16_t addr, n, val;
    addr = sVS1053b_Realtime_MIDI_Plugin[i++];
    n = sVS1053b_Realtime_MIDI_Plugin[i++];
    while (n--) {
      val = sVS1053b_Realtime_MIDI_Plugin[i++];
      writeRegister(addr, val);
    }
  }
}

void vs_sendMidi(uint8_t cmd, uint8_t data1, uint8_t data2) {
  uint8_t values[6];

  values[0] = 0;
  values[1] = cmd;
  values[2] = 0;
  values[3] = data1;
  values[4] = 0;
  values[5] = data2;

  waitReady();
  digitalWrite(VS_DCS, LOW);
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  SPI.transfer(values, NULL, 6);
  SPI.endTransaction();
  digitalWrite(VS_DCS, HIGH);
}

void vs_sendMidi(uint8_t cmd, uint8_t data1) {
  uint8_t values[4];

  values[0] = 0;
  values[1] = cmd;
  values[2] = 0;
  values[3] = data1;

  waitReady();
  digitalWrite(VS_DCS, LOW);
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  SPI.transfer(values, NULL, 4);
  SPI.endTransaction();
  digitalWrite(VS_DCS, HIGH);
}