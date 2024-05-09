// Arduino polyphonic FM sound
// * 31250 Hz sampling rate
// * 9-bit resolution
// * 4-fold polyphony (4 different tones can play simulatenously)
// * FM-synthesis with time-varying modulation amplitude
// * ADSR envelopes
// * 12 preset instruments
// Through PWM with timer1, sound is generated on pin 9
// instrument-select button on A5
// 18 sound keys on the remaining i/o pins
// by Rolf Oldeman Feb 2019
// Licence CC BY-NC-SA 2.5 https://creativecommons.org/licenses/by-nc-sa/2.5/

#include "synth.h"
#include "math.h"
#include "main.h"

//instrument definitions
#define ninstr 12           //   piano xlphn guitar cmbll bell funky vibr metal violin bass trumpt harm
unsigned int ldness[ninstr]  = {   64,   64,   64,   64,   64,   64,   64,   64,   64,   64,   64,   64}; // loudness
unsigned int pitch0[ninstr]  = {   12,   12,   12,   12,   24,   24,    0,   12,   24,   12,   12,   24}; // pitch of key0
unsigned int ADSR_a[ninstr]  = { 4096, 8192, 8192, 8192, 4096,  512,  512, 8192,  128,  128,  256,  256}; // attack parameter
unsigned int ADSR_d[ninstr]  = {    8,   32,   16,   16,    8,   16,   16,    8,   16,   16,   64,   32}; // decay parameter
unsigned int ADSR_s[ninstr]  = {    0,    0,    0,    0,    0,    0,    0,    0,  240,  240,  192,  192}; // sustain parameter
unsigned int ADSR_r[ninstr]  = {   64,  128,   32,   32,   16,   32,   32,   32,   32,   32,   64,   64}; // release parameter
unsigned int FM_inc[ninstr]  = {  256,  512,  768,  400,  200,   96,  528,  244,  256,  128,   64,  160}; // FM frequency wrt pitch
unsigned int FM_a1[ninstr]  =  {  128,  512,  512, 1024,  512,    0, 1024, 2048,  256,  256,  384,  256}; // FM amplitude start
unsigned int FM_a2[ninstr]  =  {   64,    0,  128,  128,  128,  512,  768,  512,  128,  128,  256,  128}; // FM amplitude end
unsigned int FM_dec[ninstr]  = {   64,  128,  128,  128,   32,  128,  128,  128,  128,  128,   64,   64}; // FM decay

//define the pitch2key mapping
#define keyC4   0
#define keyC4s  1
#define keyD4   2
#define keyD4s  3
#define keyE4   4
#define keyE4s  5
#define keyF4   6
#define keyF4s  7
#define keyG4   8
#define keyG4s  9
#define keyA4   10
#define keyA4s 	11
#define keyB4  	12
#define keyB4s  13
#define keyC5  	14
#define keyC5s 	15
#define keyD5  	16
#define keyD5s 	17
#define keyE5  	18
#define keyE5s  19
#define keyF5  	20
#define keyF5s  21
#define keyG5   22

#define nokey 255
#define instrkey 254

//define the pin to key mapping for 22-key keyboard
#define pinB0 keyC4
#define pinB1 keyC4s
#define pinB2 keyD4
#define pinB3 keyD4s
#define pinB4 keyE4
#define pinB5 keyE4s
#define pinB6 keyF4
#define pinB7 keyF4s
#define pinC0 keyG4
#define pinC1 keyG4s
#define pinC2 keyA4
#define pinC3 keyA4s
#define pinC4 keyB4
#define pinC5 keyB4s
#define pinC6 keyC5
#define pinC7 keyC5s
#define pinD0 keyD5
#define pinD1 keyD5s
#define pinD2 keyE5
#define pinD3 keyE5s
#define pinD4 keyF5
#define pinD5 instrkey
#define pinD6 nokey
#define pinD7 nokey

//set up array with sine values in signed 8-bit numbers
const float pi = 3.14159265;
int8_t sine[256];
void setsine() {
  for (uint16_t i = 0; i < 256; ++i) {
    sine[i] = (sin(2 * 3.14159265 * (i + 0.5) / 256)) * 128;
  }
}

//setup frequencies/phase increments, starting at C3=0 to B6. (A4 is defined as 440Hz)
unsigned int tone_inc[48];
void settones() {
  for (uint8_t i=0; i<48; i++){
    tone_inc[i]= 440.0 * pow(2.0, ( (i-21) / 12.0)) * 65536.0 / (16000000.0/512) + 0.5;
  }
}

uint8_t butstatD=0;
uint8_t butstatB=0;
uint8_t butstatC=0;
uint8_t prevbutstatD=0;
uint8_t prevbutstatB=0;
uint8_t prevbutstatC=0;

uint8_t instr=0;

#define CURRENT		(0)
#define PREVIOUS	(1)

uint32_t keys[2] = {0};
uint32_t keys_changed = 0;
bool keys_updated = false;

void synth_init(void){
	//setup the array with sine values
	setsine();

	//setup array with tone frequency phase increments
	settones();

	/* Enable PWM output */
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

//initialize the main parameters of the pulse length setting
#define nch 4 //number of channels that can produce sound simultaneously
unsigned int phase[nch]  = {0,0,0,0};
int          inc[nch]    = {0,0,0,0};
uint8_t      amp[nch]    = {0,0,0,0};
unsigned int FMphase[nch]= {0,0,0,0};
unsigned int FMinc[nch]  = {0,0,0,0};
unsigned int FMamp[nch]  = {0,0,0,0};

// main function (forced inline) to update the pulse length
//inline void setPWM(void) __attribute__((always_inline));
/*inline*/ void setPWM(void) {
  //wait for the timer to complete loop
  while((TIM1->SR & TIM_SR_UIF) == 0);

  //Clear(!) the overflow bit by writing a 0 to it
  TIM1->SR &= ~TIM_SR_UIF;

  //increment the phases of the FM
  FMphase[0] += FMinc[0];
  FMphase[1] += FMinc[1];
  FMphase[2] += FMinc[2];
  FMphase[3] += FMinc[3];

  //increment the phases of the note
  phase[0] += inc[0];
  phase[1] += inc[1];
  phase[2] += inc[2];
  phase[3] += inc[3];

  //calculate the output value and set pulse width for timer2
  int val = sine[((phase[0]+sine[FMphase[0]>>8 & 0xFF]*FMamp[0]) >> 8) & 0xFF] * amp[0];
  val += sine[((phase[1]+sine[FMphase[1]>>8 & 0xFF]*FMamp[1]) >> 8) & 0xFF] * amp[1];
  val += sine[((phase[2]+sine[FMphase[2]>>8 & 0xFF]*FMamp[2]) >> 8) & 0xFF] * amp[2];
  val += sine[((phase[3]+sine[FMphase[3]>>8 & 0xFF]*FMamp[3]) >> 8) & 0xFF] * amp[3];

  //set the pulse length
  TIM1->CCR1 = (val/32) + (TIM1->ARR >> 1);
}

//properties of each note played
uint8_t      iADSR[nch]     = {0, 0, 0, 0};
unsigned int envADSR[nch]   = {0, 0, 0, 0};
unsigned int ADSRa[nch]     = {0, 0, 0, 0};
unsigned int ADSRd[nch]     = {0, 0, 0, 0};
unsigned int ADSRs[nch]     = {0, 0, 0, 0};
unsigned int ADSRr[nch]     = {0, 0, 0, 0};
uint8_t      amp_base[nch]  = {0, 0, 0, 0};
unsigned int inc_base[nch]  = {0, 0, 0, 0};
unsigned int FMa0[nch]      = {0, 0, 0, 0};
int          FMda[nch]      = {0, 0, 0, 0};
unsigned int FMinc_base[nch]= {0, 0, 0, 0};
unsigned int FMdec[nch]     = {0, 0, 0, 0};
unsigned int FMexp[nch]     = {0, 0, 0, 0};
unsigned int FMval[nch]     = {0, 0, 0, 0};
uint8_t      keych[nch]     = {0, 0, 0, 0};
unsigned int tch[nch]       = {0, 0, 0, 0};

void synth_tick(void){
	  uint8_t keypressed = nokey;
	  uint8_t keyreleased = nokey;

	  //read and interpret input buttons
	  if(keys_updated){
		  keys_updated = false;

		  if(pinB0 != nokey && (keys_changed & 0x01)) (keys[CURRENT] & 0x01)?(keypressed = pinB0):(keyreleased = pinB0);
		  if(pinB1 != nokey && (keys_changed & 0x02)) (keys[CURRENT] & 0x02)?(keypressed = pinB1):(keyreleased = pinB1);
		  if(pinB2 != nokey && (keys_changed & 0x04)) (keys[CURRENT] & 0x04)?(keypressed = pinB2):(keyreleased = pinB2);
		  if(pinB3 != nokey && (keys_changed & 0x08)) (keys[CURRENT] & 0x08)?(keypressed = pinB3):(keyreleased = pinB3);
		  if(pinB4 != nokey && (keys_changed & 0x10)) (keys[CURRENT] & 0x10)?(keypressed = pinB4):(keyreleased = pinB4);
		  if(pinB5 != nokey && (keys_changed & 0x20)) (keys[CURRENT] & 0x20)?(keypressed = pinB5):(keyreleased = pinB5);
		  if(pinB6 != nokey && (keys_changed & 0x40)) (keys[CURRENT] & 0x40)?(keypressed = pinB6):(keyreleased = pinB6);
		  if(pinB7 != nokey && (keys_changed & 0x80)) (keys[CURRENT] & 0x80)?(keypressed = pinB7):(keyreleased = pinB7);

		  if(pinC0 != nokey && (keys_changed & 0x100)) (keys[CURRENT] & 0x100)?(keypressed = pinC0):(keyreleased = pinC0);
		  if(pinC1 != nokey && (keys_changed & 0x200)) (keys[CURRENT] & 0x200)?(keypressed = pinC1):(keyreleased = pinC1);
		  if(pinC2 != nokey && (keys_changed & 0x400)) (keys[CURRENT] & 0x400)?(keypressed = pinC2):(keyreleased = pinC2);
		  if(pinC3 != nokey && (keys_changed & 0x800)) (keys[CURRENT] & 0x800)?(keypressed = pinC3):(keyreleased = pinC3);
		  if(pinC4 != nokey && (keys_changed & 0x1000)) (keys[CURRENT] & 0x1000)?(keypressed = pinC4):(keyreleased = pinC4);
		  if(pinC5 != nokey && (keys_changed & 0x2000)) (keys[CURRENT] & 0x2000)?(keypressed = pinC5):(keyreleased = pinC5);
		  if(pinC6 != nokey && (keys_changed & 0x4000)) (keys[CURRENT] & 0x4000)?(keypressed = pinC6):(keyreleased = pinC6);
		  if(pinC7 != nokey && (keys_changed & 0x8000)) (keys[CURRENT] & 0x8000)?(keypressed = pinC7):(keyreleased = pinC7);

		  if(pinD0 != nokey && (keys_changed & 0x10000)) (keys[CURRENT] & 0x10000)?(keypressed = pinD0):(keyreleased = pinD0);
		  if(pinD1 != nokey && (keys_changed & 0x20000)) (keys[CURRENT] & 0x20000)?(keypressed = pinD1):(keyreleased = pinD1);
		  if(pinD2 != nokey && (keys_changed & 0x40000)) (keys[CURRENT] & 0x40000)?(keypressed = pinD2):(keyreleased = pinD2);
		  if(pinD3 != nokey && (keys_changed & 0x80000)) (keys[CURRENT] & 0x80000)?(keypressed = pinD3):(keyreleased = pinD3);
		  if(pinD4 != nokey && (keys_changed & 0x100000)) (keys[CURRENT] & 0x100000)?(keypressed = pinD4):(keyreleased = pinD4);
		  if(pinD5 != nokey && (keys_changed & 0x200000)) (keys[CURRENT] & 0x200000)?(keypressed = pinD5):(keyreleased = pinD5);
		  if(pinD6 != nokey && (keys_changed & 0x400000)) (keys[CURRENT] & 0x400000)?(keypressed = pinD6):(keyreleased = pinD6);
		  if(pinD7 != nokey && (keys_changed & 0x800000)) (keys[CURRENT] & 0x800000)?(keypressed = pinD7):(keyreleased = pinD7);
	  }

	  setPWM(); //#1

	  //change instrument if instrument select button is pressed
	  if ( keypressed==instrkey) {
	    instr++;
	    if (instr>=ninstr) instr=0;
	    keypressed=keyA4;
	  }
	  if (keyreleased==instrkey) keyreleased=keyA4;

	  setPWM(); //#2

	  //find the best channel to start a new note
	  uint8_t nextch = 255;

	  //first check if the key is still being played
	  if ((iADSR[0] > 0) & (keypressed == keych[0])) nextch = 0;
	  if ((iADSR[1] > 0) & (keypressed == keych[1])) nextch = 1;
	  if ((iADSR[2] > 0) & (keypressed == keych[2])) nextch = 2;
	  if ((iADSR[3] > 0) & (keypressed == keych[3])) nextch = 3;

	  //then check for an empty channel
	  if (nextch == 255) {
	    if (iADSR[0] == 0)nextch = 0;
	    if (iADSR[1] == 0)nextch = 1;
	    if (iADSR[2] == 0)nextch = 2;
	    if (iADSR[3] == 0)nextch = 3;
	  }
	  //otherwise use the channel with the longest playing note
	  if (nextch == 255) {
	    nextch = 0;
	    if (tch[0] > tch[nextch])nextch = 0;
	    if (tch[1] > tch[nextch])nextch = 1;
	    if (tch[2] > tch[nextch])nextch = 2;
	    if (tch[3] > tch[nextch])nextch = 3;
	  }

	  setPWM(); //#3

	  //initiate new note if needed
	  if (keypressed != nokey) {
	    phase[nextch]=0;
	    amp_base[nextch] = ldness[instr];
	    inc_base[nextch] = tone_inc[pitch0[instr]+keypressed];
	    ADSRa[nextch]=ADSR_a[instr];
	    ADSRd[nextch]=ADSR_d[instr];
	    ADSRs[nextch]=ADSR_s[instr]<<8;
	    ADSRr[nextch]=ADSR_r[instr];
	    iADSR[nextch] = 1;
	    FMphase[nextch]=0;
	    FMinc_base[nextch] = ((long)inc_base[nextch]*FM_inc[instr])/256;
	    FMa0[nextch] = FM_a2[instr];
	    FMda[nextch] = FM_a1[instr]-FM_a2[instr];
	    FMexp[nextch]=0xFFFF;
	    FMdec[nextch]=FM_dec[instr];
	    keych[nextch] = keypressed;
	    tch[nextch] = 0;
	  }

	  setPWM(); //#4

	  //stop a note if the button is released
	  if (keyreleased != nokey) {
	    if (keych[0] == keyreleased)iADSR[0] = 4;
	    if (keych[1] == keyreleased)iADSR[1] = 4;
	    if (keych[2] == keyreleased)iADSR[2] = 4;
	    if (keych[3] == keyreleased)iADSR[3] = 4;
	  }

	  setPWM(); //#5

	  //update FM decay exponential
	  FMexp[0]-=(long)FMexp[0]*FMdec[0]>>16;
	  FMexp[1]-=(long)FMexp[1]*FMdec[1]>>16;
	  FMexp[2]-=(long)FMexp[2]*FMdec[2]>>16;
	  FMexp[3]-=(long)FMexp[3]*FMdec[3]>>16;

	  setPWM(); //#6

	  //adjust the ADSR envelopes
	  for (uint8_t ich = 0; ich < nch; ich++) {
	    if (iADSR[ich] == 4) {
	      if (envADSR[ich] <= ADSRr[ich]) {
	        envADSR[ich] = 0;
	        iADSR[ich] = 0;
	      }
	      else envADSR[ich] -= ADSRr[ich];
	    }
	    if (iADSR[ich] == 2) {
	      if (envADSR[ich] <= (ADSRs[ich] + ADSRd[ich])) {
	        envADSR[ich] = ADSRs[ich];
	        iADSR[ich] = 3;
	      }
	      else envADSR[ich] -= ADSRd[ich];
	    }
	    if (iADSR[ich] == 1) {
	      if ((0xFFFF - envADSR[ich]) <= ADSRa[ich]) {
	        envADSR[ich] = 0xFFFF;
	        iADSR[ich] = 2;
	      }
	      else envADSR[ich] += ADSRa[ich];
	    }
	    tch[ich]++;
	    setPWM(); //#7-10
	  }

	  //update the tone for channel 0
	  amp[0] = (amp_base[0] * (envADSR[0] >> 8)) >> 8;
	  inc[0] = inc_base[0];
	  FMamp[0] = FMa0[0] + ((long)FMda[0] * FMexp[0]>>16);
	  FMinc[0] = FMinc_base[0];
	  setPWM(); //#11

	  //update the tone for channel 1
	  amp[1] = (amp_base[1] * (envADSR[1] >> 8)) >> 8;
	  inc[1] = inc_base[1];
	  FMamp[1] = FMa0[1] + ((long)FMda[1] * FMexp[1]>>16);
	  FMinc[1] = FMinc_base[1];
	  setPWM(); //#12

	  //update the tone for channel 2
	  amp[2] = (amp_base[2] * (envADSR[2] >> 8)) >> 8;
	  inc[2] = inc_base[2];
	  FMamp[2] = FMa0[2] + ((long)FMda[2] * FMexp[2]>>16);
	  FMinc[2] = FMinc_base[2];
	  setPWM(); //#13

	  //update the tone for channel 3
	  amp[3] = (amp_base[3] * (envADSR[3] >> 8)) >> 8;
	  inc[3] = inc_base[3];
	  FMamp[3] = FMa0[3] + ((long)FMda[3] * FMexp[3]>>16);
	  FMinc[3] = FMinc_base[3];
	  setPWM(); //#14

	  //update counters
	  tch[0]++;
	  tch[1]++;
	  tch[2]++;
	  tch[3]++;

	  setPWM(); //#15
}

void synth_set_keys(uint32_t mask){
	keys[CURRENT] = mask;

	if(keys[CURRENT] != keys[PREVIOUS]){
		keys_updated = true;
		keys_changed = keys[CURRENT] ^ keys[PREVIOUS];
		keys[PREVIOUS] = keys[CURRENT];
	}
}
