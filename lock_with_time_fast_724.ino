//LumiKey receiver source code on Arduino UNO
//Developed by Yufeng Wang, Tian Zhang
//Jul.31th,2014
//-------------------Libraries required-----------------//
#include <Wire.h>
#include "RTClib.h"
#include <HexConversionUtils.h>
#include <SipHash_2_4.h>
#include <siphash_2_4_asm.h>
//-------------const variables config---------------------//
const int PIN_NO=13;
unsigned char key[] PROGMEM = "HKUST_Harvard_SP";
const int DATA_LENGTH=32;
RTC_DS1307 RTC;
#define RELAY1 6
#define RELAY2 5
const int VALID_DURATION=8500; 
const int MAX_DURATION=60000;
const int END_DURATION = 80000;
//------------------global variables----------------------//
unsigned long duration;
int data_bits[512];
int count=0;
int decode_start=0;
int decode_end=0;
char data[9];
int data_count=0;
char tmp[17];
char info[2];
//---------------PROGRAM----------------//
void setup(){
  pinMode(PIN_NO, INPUT);
  pinMode(RELAY1, OUTPUT);  
  pinMode(RELAY2, OUTPUT);
  Wire.begin();
  RTC.begin();
  Serial.begin(9600);
}
void loop(){
  duration = pulseIn(PIN_NO, HIGH);
  //Serial.println(duration);
  if((duration>VALID_DURATION&&duration<MAX_DURATION)||duration>END_DURATION){
      PWM_decode();
    //-------Output binary bits--------//
    //Serial.println(data_bits[count]);  
    //---------------------------------//
  if(decode_start==0){
      decode_start=findStart();}
  if (decode_start!=0&&data_bits[count]==3&&decode_end==0){
      decode_end=count-1;
        //--------Output beginning index of decoding(The next bit after 222)----------//
//         Serial.println("Start:");
//         Serial.println(decode_start); 
         //-------------------------------------------------------------------------------//
           //-----------------Output ending index of decoding(The previous bit of 3)-------------//
//             Serial.println("End:");
//             Serial.println(decode_end); 
           //----------------------------------------------------------------------------------------//
         }
   if(decode_start!=0&&decode_end==0&&count-decode_start>DATA_LENGTH-1){
     initial();
   }
   if(decode_start!=0&&decode_end!=0){
      if((decode_end-decode_start)==DATA_LENGTH-1){
         decode();
         hash(info,2);
         validate_password();
      }      
      else{
         Serial.println("Bit-Length != 32");
         Serial.println(decode_end-decode_start);
       }
       initial();
   }
          count++;
  }
}
void validate_password(){
     for(int i=0;i<data_count;i++){
         if(tmp[i]!=data[i]){
           Serial.println("Password not match...");
            break;
          }
         if(i==7&&tmp[i]==data[i]){
             Serial.println("--------------------DOOR OPEN!!------------------");
             dooropen();
             break;
         }
        }
}
int findStart(){
  //find the start index of decoding by checking current bit and previous 6 bits
    if(count<2){
        return 0;}
    else{
        if(data_bits[count]!=2&&data_bits[count-1]==2&&data_bits[count-2]==2&&data_bits[count-3]==2){
        return count;
    }
    else
        return 0;
  }
  
    return 0;
}
void decode(){
  //Decode 4 bits each time and map it to a HEX number(0-F)
  char fourbits[4];
  for (int j=decode_start;j<=decode_end;j=j+4){
      fourbits[0]=data_bits[j];
      fourbits[1]=data_bits[j+1];
      fourbits[2]=data_bits[j+2];
      fourbits[3]=data_bits[j+3];
      if(fourbits[0]==0&&fourbits[1]==0&&fourbits[2]==0&&fourbits[3]==0){
       data[data_count]='0';}
      else if(fourbits[0]==0&&fourbits[1]==0&&fourbits[2]==0&&fourbits[3]==1){
       data[data_count]='1';}
      else if(fourbits[0]==0&&fourbits[1]==0&&fourbits[2]==1&&fourbits[3]==0){
       data[data_count]='2';}
      else if(fourbits[0]==0&&fourbits[1]==0&&fourbits[2]==1&&fourbits[3]==1){
       data[data_count]='3';} 
       else if(fourbits[0]==0&&fourbits[1]==1&&fourbits[2]==0&&fourbits[3]==0){
       data[data_count]='4';}
       else if(fourbits[0]==0&&fourbits[1]==1&&fourbits[2]==0&&fourbits[3]==1){
       data[data_count]='5';}
       else if(fourbits[0]==0&&fourbits[1]==1&&fourbits[2]==1&&fourbits[3]==0){
       data[data_count]='6';}
       else if(fourbits[0]==0&&fourbits[1]==1&&fourbits[2]==1&&fourbits[3]==1){
       data[data_count]='7';}
       else if(fourbits[0]==1&&fourbits[1]==0&&fourbits[2]==0&&fourbits[3]==0){
       data[data_count]='8';}
       else if(fourbits[0]==1&&fourbits[1]==0&&fourbits[2]==0&&fourbits[3]==1){
       data[data_count]='9';}
       else if(fourbits[0]==1&&fourbits[1]==0&&fourbits[2]==1&&fourbits[3]==0){
       data[data_count]='A';}
       else if(fourbits[0]==1&&fourbits[1]==0&&fourbits[2]==1&&fourbits[3]==1){
       data[data_count]='B';}
       else if(fourbits[0]==1&&fourbits[1]==1&&fourbits[2]==0&&fourbits[3]==0){
       data[data_count]='C';}
       else if(fourbits[0]==1&&fourbits[1]==1&&fourbits[2]==0&&fourbits[3]==1){
       data[data_count]='D';}
       else if(fourbits[0]==1&&fourbits[1]==1&&fourbits[2]==1&&fourbits[3]==0){
       data[data_count]='E';}
       else if(fourbits[0]==1&&fourbits[1]==1&&fourbits[2]==1&&fourbits[3]==1){
       data[data_count]='F';}
       else{
         Serial.println("Decode not match");
         break;
       } 
      data_count++;
  }
  //-----------Print out received data from smartphone-----------//
  Serial.println("---------RECEIVED DATA-----------");
  for (int k=0;k<data_count;k++){
    Serial.print(data[k]);}
  Serial.println();
  Serial.println("---------------------------------");
}
 void dooropen(){
   //Trigger a pulse to control the relay
   digitalWrite(RELAY1,HIGH);           // Turns ON Relays OPEN DOOR 
   digitalWrite(RELAY2,HIGH);
   delay(5000);                        // Keep the door open for 5 seconds
   digitalWrite(RELAY1,LOW);          // Turns Relay Off
   digitalWrite(RELAY2,LOW);
 }
void initial(){
  for (int i=0;i<count;i++){
    data_bits[i]=0;
  }
//    for (int i=0;i<17;i++){
//    data[i]=0;tmp[i]=0;
//  }
  info[0]=0;
  info[1]=0;
  count=0;
  decode_start=0;
  decode_end=0;
  data_count=0;
}
void hash(char info[],int length){
    gettime();
    sipHash.initFromPROGMEM(key);
    for (int i=0;i<length;i++){
    sipHash.updateHash((byte)info[i]);}
    sipHash.finish(); // result in BigEndian format
    reverse64(sipHash.result); // chage to LittleEndian to match
    hexToAscii(sipHash.result,8,tmp,9);
    //Print out hashed value calculated on Arduino
    Serial.println("-------------HASH VALUE--------------");
    Serial.println(tmp);
    Serial.println("-------------------------------------");
}
void gettime() {
  // Get the current time
  DateTime now = RTC.now();   
  //Save the time in the array info for later hashing
  int temp=now.minute();
  info[0]=now.year()/100+now.year()%100+temp;
  info[1]=now.month()+now.day()+now.hour();
  //-------------Display the current time---------//
  Serial.print("Current time: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(temp, DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
} 
void PWM_decode(){
  if(decode_start!=0&&(count-decode_start<DATA_LENGTH-1)&&(duration>40000)){
    duration-=30000;Serial.println("-----GC------");Serial.println(count-decode_start);
  }
  if(duration<24000){//0 is (8500,24000)
      data_bits[count]=0;
    }
  else if (duration <40000){ //1 is (24000,40000)
      data_bits[count]=1;
  }
   else if(duration<70000){ //2-start bit- is (40000,70000)
       data_bits[count]=2;
    }
   else if (duration>80000){ //end bit is >80000
       data_bits[count]=3;
   }
}
