#include <iostream>
#include <iomanip>
#include <fstream>
#include <signal.h>
//#include <set>
#include <string>

// Voice Message (VM_)
#define VM_NOTEOFF 0x80
#define VM_NOTEON  0x90
#define VM_POLYAFTER   0xa0
#define VM_CC 0xb0
#define VM_PC 0xc0
#define VM_MONOAFTER 0xd0
#define VM_PITCHBEND 0xe0
// Mode Message (MM_) = sub-mode of VM_CC
// Nothing
// System Exclusive (SE_)
#define SE_START 0xf0
#define SE_END 0xf7
#define SYSEX 0xf0
//#define ENDEX 0xf7
// System Common (SC_)
#define SC_MTCQF 0xf1
#define SC_SONGPOSPTR 0xf2
#define SC_SONGSELECT 0xf3
#define SC_TUNEREQU 0xf6
// Real-Time code (RT_)
#define RT_CLOCK 0xf8
#define RT_START 0xfa
#define RT_CONT 0xfb
#define RT_STOP 0xfc
#define RT_SENSING 0xfe
#define RESET 0xff
// MIDI STATE
#define WAIT_FOR_MESSAGE 0
#define WAIT_FOR_3_VALUES 1
#define WAIT_FOR_2_VALUES 2
#define WAIT_FOR_1_VALUE 3
#define WAIT_FOR_SYSEX_MAN_ID 4
#define WAIT_FOR_SYSEX_2ND_ID 5
#define WAIT_FOR_SYSEX_3RD_ID 6
#define WAIT_FOR_SYSEX_DEV_ID 7
#define WAIT_FOR_VALUE_OR_ENDEX 8
#define WAIT_FOR_SYSEX_SUBID1 9
#define WAIT_FOR_SYSEX_SUBID2 10
#define WAIT_FOR_SYSEX_MX49 11
bool done;
static void finish(int ignore) { done = true; }

std::string port = "/dev/stdin";
std::ifstream device;

char dec2hex[] = {'0', '1', '2', '3', '4', '5', '6', '7',
		  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

char BUFFER[8] = "1234567";

char* hex2(unsigned int v) {
  v &= 0xff;
  BUFFER[0]=dec2hex[v>>4]; BUFFER[1]=dec2hex[v&15]; BUFFER[2]='\0';
  return BUFFER;
}

char * hex(unsigned int v, unsigned char w) {
  v=v & ( (1<<(w*4))-1); // keep only the 4*w lower bits of v
  int i=1;
  while(i<=w) {
    BUFFER[w-i]=dec2hex[v&15];
    v = v>>4;
    i++;
  }
  BUFFER[w]='\0';
}

class MX49AddressInfo
{
  // MX49 Address Info
public:
  int topAddress;
  int byteCount;
  std::string info;
  MX49AddressInfo(int addr, int size, std::string text) :
    topAddress(addr),
    byteCount(size),
    info(text) {};
};

MX49AddressInfo mxInfo[] = {
  { ( 0x00 << 8 + 0x00 ) << 8 + 0x00, 0x44, "SYSTEM:System"},
  { ( 0x40 << 8 + 0x00 ) << 8 + 0x00, 0x50, "VOICE (NORMAL):COMMON:Common"}
};

void setup() {
  int i=0;

  device.exceptions ( std::ifstream::failbit | std::ifstream::badbit |
		      std::ifstream::eofbit);  
  try {
    device.open(port.c_str(), std::ios::binary);
  }
  catch (std::ifstream::failure e) {
    std::cerr << "Exception opening/reading/closing file:" << port << "\n";
    exit(1);
  }
}
 
void loop() {
  char buffer[1];
  unsigned char byte;
  static unsigned long address = 0;
  static unsigned char checksum = 0;
  static unsigned char current_state = 0;
  static unsigned char channel = 0;
  
  try {
    device.read(buffer, 1);
  }
  catch (std::ifstream::failure &e) {
    if (! device.eof() ) {
      std::cerr << "Exception reading file:" << port << "\n";
      std::cerr << e.code().message() << std::endl;
    }
    std::cout << "\n";
    done=true;
    return;
  }
  byte = buffer[0];

  if (address) { std::cout << "\n"; }
  std::cout << std::setw(3) << std::hex << (int)address << " - ";
  std::cout << std::setw(2) << std::hex << (int)byte <<     "  ";
  if (byte & 0x80) {
    if (
	(byte == RT_CLOCK) ||
	(byte == RT_START) ||
	(byte == RT_CONT) ||
	(byte == RT_STOP) || 
	(byte == RT_SENSING)
	) { // Real-time messages
      ; // ignore, do nothing
    }
    else if (byte == SE_START) { // Sysex start
      // Check if prev message done (ignored for the moment)
      current_state = SYSEX;
    }
    else if (byte == SE_END) { // Sysex end
      if (current_state != SYSEX) {
	std::cout << "ERROR: SE_END received when not in sysex mode\n";
      }
      current_state = WAIT_FOR_MESSAGE;
    }
    else {
      unsigned char message = byte & 0xf0;
      if (message == VM_NOTEOFF) {current_state = message;}
      else if (message == VM_NOTEON) {current_state = message;}
      else if (message == VM_POLYAFTER) {current_state = message;}
      else if (message == VM_CC) {current_state = message;}
      else if (message == VM_PC) {current_state = message;}
      else if (message == VM_MONOAFTER) {current_state = message;}
      else if (message == VM_PITCHBEND) {current_state = message;}
      else {
	std::cout << "ERROR: unexpected message (" <<
	  std::hex << (int)address << "-" << std::hex << (int)message;
	std::cout << ")";
	current_state = 0;
      }
    }
  }
  else {
    std::cout << "VAL " << std::setw(2) << std::hex << (int)byte;
  }
  address++;
}

int main(int argc, char *argv[]) {

  for (int i=0; i<sizeof(mxInfo)/sizeof(MX49AddressInfo); i++) {
    std::cout << "mxinfo[" << i << "]=" << mxInfo[i].info << " " <<
      mxInfo[i].byteCount << " " <<
      hex(mxInfo[i].topAddress, 6) << "\n";
  }

  setup();
  done = false;
  (void) signal(SIGINT, finish);

  while(!done) {
    loop();
  }
  device.close();
}

