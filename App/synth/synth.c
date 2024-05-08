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
#define keyF4   5
#define keyF4s  6
#define keyG4   7
#define keyG4s  8
#define keyA4   9
#define keyA4s 10
#define keyB4  11
#define keyC5  12
#define keyC5s 13
#define keyD5  14
#define keyD5s 15
#define keyE5  16
#define keyF5  17

#define nokey 255
#define instrkey 254

//define the pin to key mapping for 18-key keyboard
#define pinD0 keyC5    //Arduino pin D0
#define pinD1 keyB4    //Arduino pin D1
#define pinD2 keyA4s   //Arduino pin D2
#define pinD3 keyA4    //Arduino pin D3
#define pinD4 keyG4s   //Arduino pin D4
#define pinD5 keyG4    //Arduino pin D5
#define pinD6 keyF4s   //Arduino pin D6
#define pinD7 keyF4    //Arduino pin D7
#define pinB0 keyE4    //Arduino pin D8
#define pinB1 nokey    //Arduino pin D9  used for audio out
#define pinB2 keyD4s   //Arduino pin D10
#define pinB3 keyD4    //Arduino pin D11
#define pinB4 keyC4s   //Arduino pin D12
#define pinB5 keyC4    //Arduino pin D13
#define pinB6 nokey    //Arduino pin D14 inexistent
#define pinB7 nokey    //Arduino pin D15 inexistent
#define pinC0 keyC5s   //Arduino pin A0
#define pinC1 keyD5    //Arduino pin A1
#define pinC2 keyD5s   //Arduino pin A2
#define pinC3 keyE5    //Arduino pin A2
#define pinC4 keyF5    //Arduino pin A3
#define pinC5 instrkey //Arduino pin A4
#define pinC6 nokey    //Arduino pin A5 inexistent
#define pinC7 nokey    //Arduino pin A6 inexistent

//set up array with sine values in signed 8-bit numbers
const float pi = 3.14159265;
char sine[256];
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

void synth_init(void){
	//setup the array with sine values
	setsine();

	//setup array with tone frequency phase increments
	settones();
}

void synth_tick(void){

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
inline void setPWM() __attribute__((always_inline));
inline void setPWM() {
  //wait for the timer to complete loop
  //while ((TIFR1 & 0B00000001) == 0);

  //Clear(!) the overflow bit by writing a 1 to it
  //TIFR1 |= 0B00000001;

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
  int val = sine[(phase[0]+sine[FMphase[0]>>8]*FMamp[0]) >> 8] * amp[0];
  val += sine[(phase[1]+sine[FMphase[1]>>8]*FMamp[1]) >> 8] * amp[1];
  val += sine[(phase[2]+sine[FMphase[2]>>8]*FMamp[2]) >> 8] * amp[2];
  val += sine[(phase[3]+sine[FMphase[3]>>8]*FMamp[3]) >> 8] * amp[3];

  //set the pulse length
   TIM1->CCR1 = val/128 + 256;
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
