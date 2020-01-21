/*
 * Source Code For RFID Based Attendance System,
 * written by Aniebiet I. Akpan
 * for EEE599: 2017 Final Year Project, Dept of EEE, UNIVERSITY OF MAIDUGURI.
 * this is a sample code for two students.
 * and can be modified for more students. 
 * 
 * see line comments for necessary modifications
 * for expanding the number of tags
 */

/* Include the standard Arduino SPI library */
#include <SPI.h>
/* Include the RFID library */
#include <RFID.h>

/* Define the DIO used for the SDA (SS) and RST (reset) pins. */
#define SDA_DIO 9
#define RESET_DIO 8
/* Create an instance of the RFID library */
RFID RC522(SDA_DIO, RESET_DIO);

/*include the SD lib*/
#include <SD.h>
File myFile;

#include <LiquidCrystal.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//get string from SD
char holder; //hold each character
String line;  //hold each line from the file
int num_lines = 0;

//today
//get_day_and_totals  variables
char day_c1, day_c2; //get day from file, save chars
int today;
//next day
int next_day;
String n_day_c;

//totals for each id
int total[] = {0, 0}; //add more values for more than two students
int total_ct = 0;

//numbers for charToInt
int num1, num2;

//characters from intToChar
char numb_ret[2];

//changeString variables
char a[2]; //conv char to String
char b[2]; //convert char to String
int f_index;
String _s;
String str_ret;

//strings to hold card number from serNUM
unsigned char temp1[5];
String temp2 = "";
String card_num = "";

int i = 0;

//master card id
String master_card = "382263999128";
int m_card_swiped = 0; //m_card swiped
int st_once = 0; //student swiped once

//card numbers are stored here
//card numbers can be seen on serial monitor, each time a card is swiped
String card_nums_holder[] = {"532498109169", "1808524411796", "1197522643245"}; //put card numbers here
String id_1 = "532498109169";
String id_2 = "1808524411796 "; //create more strings for the more card numbers or replace strings with an array
String res_card = "1197522643245"; //the reset card

//student attendance for today
int attendance[] = {0, 0}; //initialise with more values for more cards

int id_swiped = 0;
int prev_id_swiped;

void setup() {
  //buzzer pin
  pinMode(13, OUTPUT);
  
  Serial.begin(9600);
  SPI.begin(); //enable SPI interface
  RC522.init(); //Initialise the RFID reader

  //initialise the sd card
  init_sd();
  get_str(); //get line from file
  
  get_day_and_totals(); //get day and totals
  set_next_day(); //set next day

  Serial.println(line);
  
  lcd.begin(16, 2);
  lcd.print("FINAL YR PROJECT");
  lcd.setCursor(5,1);
  lcd.print("EEE599");
  delay(3000);
  lcd.clear();
  lcd.print("ID: 12/05/05/031");
  delay(3000);
  lcd.clear();
  lcd.setCursor(2,0);

  for(int k=0; k<2; k++){
    lcd.print("Welcme To Dept of EEE Attendance Systm");
    for (int positionCounter = 0; positionCounter < 30; positionCounter++) {
      lcd.scrollDisplayLeft();
      delay(200); //initially 200
    }
    lcd.clear();
  }
  delay(2000);
  lcd.print("initialising...");
  lcd.setCursor(2,1);
  lcd.print("please wait...");
  lcd.blink();
  delay(3000); //initially 3000

  //ready
  lcd.clear();
  lcd.print("Ready in 5secs>");
  for(int ct=5; ct>-1; ct--){
    lcd.setCursor(15,1);
    lcd.print(ct);
    lcd.cursor();
    delay(1000); //initially 1000
  }

  //today
  lcd.clear();
  lcd.print("today is Lecture ");
  lcd.setCursor(3,1);
  lcd.print("Day ");
  lcd.setCursor(7,1);
  lcd.print(day_c1);
  lcd.setCursor(8,1);
  lcd.print(day_c2);
  delay(5000); //initially 5000
  lcd.clear();

  //delay
  lcd.setCursor(2,1);
  lcd.print("please wait...");
  delay(3000); //initially 3000
  
  lcd.clear();
  lcd.print("Please Swipe the");
  lcd.setCursor(0,1);
  lcd.print("Master Card!");
}

void loop() {
  /* Has a card been detected? */
  if (RC522.isCard())
  {
    /* If so then get its serial number */
    RC522.readCardSerial();
    Serial.println("Card detected:");
    analogWrite(13, 150);
    delay(200);
    analogWrite(13, 0);
    delay(200);
    for(i=0;i<5;i++)
    {
      temp1[i] = RC522.serNum[i];
      temp2 += temp1[i];
      if(i == 4){
        card_num = temp2;
        temp2 = "";
      }
    }
    Serial.println(card_num);
    if(check_master_id(card_num) == true){
      m_card_swiped++;
      lcd.clear();
      if(m_card_swiped == 1){
        lcd.print("Access Grant!");
        Serial.print("master swiped!");
        lcd.setCursor(0,1);
        lcd.print("please Wait... ");
        delay(3000);
      }
      lcd.clear();
      lcd.print("swipe student");
      lcd.setCursor(0, 1);
      lcd.print("Card: ");
      
      if(m_card_swiped > 1 && st_once >= 1){
        write_att();
        set_new_total();
        del_n_rec_file();
        
        lcd.clear();
        for(int c=0; c<10; c++){
          lcd.print("**Thank you and goodbye!!!**");
         for (int positionCounter = 0; positionCounter < 24; positionCounter++) {
           lcd.scrollDisplayLeft();
            delay(500);
         }
         lcd.clear();
        }
      }
    }
    else if((check_reset_id(card_num) == true))
      reset_system();
      
    else if((check_card(card_num) != 0)  && (m_card_swiped >= 1)){ //not master_card(is a student card) or invalid
      st_once++;
      if(st_once == 1){
        prev_id_swiped = id_swiped;
        set_att();
      //prev_id_swiped = id_swiped;
        lcd.setCursor(6,1);
        Serial.print("saved");
        Serial.print(attendance[id_swiped-1]);
        lcd.print("saved      ");
        delay(1000);
        lcd.setCursor(6,1);
        lcd.print("     ");
        return;
      }
      else if(st_once > 1 && prev_id_swiped == id_swiped){
        lcd.setCursor(6,1);
        lcd.print("TOT ATT:");
        lcd.setCursor(14,1);
        if(id_swiped == 1){
          lcd.print(total[0] + 1);
        }
        else if(id_swiped == 2){
          lcd.print(total[1] + 1);
        } //add more lines for more IDs or change conditions structure
        delay(1000);
        lcd.setCursor(6,1);
        lcd.print("             ");
        return;
      }
      else if(st_once > 1 && prev_id_swiped != id_swiped){
        st_once = 1;
        prev_id_swiped = id_swiped;
        set_att();
        lcd.setCursor(6,1);
        Serial.print("saved");
        Serial.print(attendance[id_swiped-1]);
        lcd.print("saved      ");
        delay(1000);
        lcd.setCursor(6,1);
        lcd.print("     ");
        return;
      }
      
      Serial.print("STUDENT Card Swiped");
    }
    Serial.println();
    Serial.println();
  }
  delay(100);
}
//check if  master card is swiped
bool check_master_id(String card_numb){
    if(card_numb == master_card)
        return true;
    else
        return false;
}
//check if reset card is swiped
bool check_reset_id(String card_numb){
    if(card_numb == res_card)
        return true;
    else
        return false;
}
int check_card(String card_numb){ 
    for(int count=0; count<2; count++){ //replace 2 with number of students
        if(card_numb == card_nums_holder[count]){
            id_swiped = count+1;
            return id_swiped;
        }
    }
    return 0;
}
void set_att(){
    attendance[id_swiped - 1] = 1;
}
int charToInt(char char1, char char2){ //this function converts two characters into a numbers
    num1 = char1 - 48;
    num2 = char2 - 48;
    return (num1*10+num2);
}
char* intToChar(int number){ //this function returns an array of characters in place of a number
    numb_ret[0] = number/10+48;
    numb_ret[1] = number%10+48;
    return numb_ret;
}
String changeString(String strToChange, char fir, char sec, char value){ //this function will take first two characters and
    a[0] = fir;                                                          //return a string with the next character changed
    a[1] = '\0';
    b[0] = sec;
    b[1] = '\0';
    _s = String(a);
    _s += b;
    f_index = strToChange.indexOf(_s);
    strToChange.setCharAt(f_index+2, value);
    str_ret = strToChange;
    return str_ret;
}
void init_sd(){
  Serial.print("Initializing SD card...");

  if (!SD.begin(53)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
}
void get_str(){
 myFile = SD.open("att.txt", FILE_READ);

  if (myFile) {
    Serial.println("att.txt is open");
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      //Serial.write(myFile.read());
      holder = myFile.read();
      line += holder;
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening att.txt");
  }
}
void get_day_and_totals(){
  //get day
  day_c1 = line.charAt(4);
  day_c2 = line.charAt(5);
  today = charToInt(day_c1, day_c2);
  next_day = today+1;
  
  //get totals at specific character positions, know character positions, each total character is 145 characters from the previous
  total[0] = charToInt(line.charAt(177), line.charAt(178));
  total[1] = charToInt(line.charAt(322), line.charAt(323));//add more totals lines for more students
}
void set_next_day(){
  n_day_c = intToChar(next_day);
  line[4] = n_day_c[0];
  line[5] = n_day_c[1];
}
void write_att(){ //Write attendance to SD Card
  int ctr=0;
   for(int w=0; w<line.length(); w++){
        if((line[w] == day_c1) && (line[w+1] == day_c2) && (line[w-1] != 'L') && (line[w-2] != 'y')){
           line[w+3] = char(attendance[ctr] + 48);
           
           //fix the '#' bug at  the start of second students attendance
           if((today == 1) && (attendance[1] == 0) && (ctr == 2))
            line[w+3] = '0';
           else if((today == 1) && (attendance[1] == 1)  && (ctr == 2))
            line[w+3] = '1';
            ++ctr;
        }
    }
}
void set_new_total(){ //this block should be modified for more students
  int new_tot1, new_tot2; //add more totals for more students, or replace with an array
  String new_t1;
  String new_t2; //add more strings or replace with an array
    if(attendance[0] == 0){
      new_tot1 = total[0];
      new_t1 = intToChar(new_tot1);
      line[177] = new_t1[0];
      line[178] = new_t1[1];
    }
    else if(attendance[0] == 1){
      new_tot1 = total[0] + 1;
      new_t1 = intToChar(new_tot1);
      line[177] = new_t1[0];
      line[178] = new_t1[1];
    }
    if(attendance[1] == 0){
      new_tot2 = total[1];
      new_t2 = intToChar(new_tot2);
      line[322] = new_t2[0];
      line[323] = new_t2[1];
    }
    else if(attendance[1] == 1){
      new_tot2 = total[1] + 1;
      new_t2 = intToChar(new_tot2);
      line[322] = new_t2[0];
      line[323] = new_t2[1];
    }
}
void del_n_rec_file(){
  // delete the file:
  Serial.println("Removing att.txt...");
  SD.remove("att.txt");
  myFile = SD.open("att.txt", FILE_WRITE);
  if (myFile) {
    Serial.print("Writing to att.txt...");
    myFile.print(line);
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening att.txt");
  }
    // close the file:
    myFile.close();
}
void reset_system(){ //if reset card is swiped
  lcd.clear();
  lcd.print("resetting");
  lcd.setCursor(4,1);
  lcd.print("System...");
  delay(4000);
  // delete the file:
  Serial.println("Removing att.txt...");
  SD.remove("att.txt");
  myFile = SD.open("att.txt", FILE_WRITE);
  if (myFile) {
    Serial.print("Writing to att.txt...");
    myFile.print("day(01)\nID NUMBER\tCLASS(ATTENDANCE)\n1.       \t01(0),02(0),03(0),04(0),05(0),06(0),07(0),08(0),09(0),10(0),11(0),12(0),13(0),14(0),15(0),16(0),17(0),18(0),19(0),20(0)    TOTAL01(00)\n2.       \t01(0),02(0),03(0),04(0),05(0),06(0),07(0),08(0),09(0),10(0),11(0),12(0),13(0),14(0),15(0),16(0),17(0),18(0),19(0),20(0)    TOTAL02(00)*");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening att.txt");
  }
    // close the file:
    myFile.close();
    lcd.clear();
    lcd.print("RESET COMPLETE.");
    lcd.setCursor(1,1);
    lcd.print("PLEASE RESTART!");
    delay(10000);
}
