#include <LiquidCrystal.h>



LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
const char* phoneNumber = "MyPhone)";
const int button_press_level = 10;

bool editting = false;

short unsigned int edditing_parameter_id = 0;

struct Camera{
  short unsigned int number;
  short unsigned int ntc_pin;
  short unsigned int relay_pin;

  bool working;
  bool inverted_relay;
  bool heating;

  long unsigned int required_time;

  short int days_left;
  short int hours_left;
  short int minutes_left;

  float seconds_left;

  float actual_temperature;
  short int target_temperature;
  short int hysteresis;
};

struct Button{
  short unsigned int low_level;
  short unsigned int high_level;

  int iterations_counted;

  bool prev_state;
  bool cur_state;

  bool pressed;
  bool triggered;
};

byte CELSIUS[8] = {0b00011,0b00011,0b01100,0b10010,0b10000,0b10010,0b10010,0b01100};
byte DELTA[8] = {0b00000,0b00000,0b00000,0b00000,0b00100,0b01010,0b10001,0b11111};
byte FIRE[8] = {	0b00000,	0b00010,	0b00110,	0b01010,	0b10001,	0b10101,	0b11011,0b01110};
byte TEA[8] = {	0b01110,	0b01110,	0b10001,	0b10001,	0b10111,	0b11111,	0b11111,	0b01110};
byte WALL[8] = {	0b01110,	0b01110,	0b01110,	0b01110,	0b01110,	0b01110,	0b01110,	0b01110};
byte DAYS[8] = {	0b00000,	0b00000,	0b11001,	0b10100,	0b10100,	0b10101,	0b10100,	0b11000};
byte HOURS[8] = {	0b00000,	0b00000,	0b10001,	0b10000,	0b10000,	0b11101,	0b10100,	0b10100};
byte DONE[8] = {	0b00000,	0b00000,	0b00001,	0b00011,	0b10110,	0b11100,	0b01000,	0b00000};


Camera fermenter[4];
struct Camera *camera0 = &fermenter[0];

Button but_up;
Button but_down;
Button but_left;
Button but_right;
Button but_select;

Button *button_up = &but_up;
Button *button_down = &but_down;
Button *button_left = &but_left;
Button *button_right = &but_right;
Button *button_select = &but_select;

//OTHER--------------------------------------------------------
long unsigned int get_time_in_minutes(struct Camera *camera){
  return (camera -> days_left * 24 * 60 + camera -> hours_left * 60 + camera -> minutes_left);
}
/*
void sendSMS(String message) {
  Serial.println("AT");
  delay(100);
  Serial.println("AT+CMGF=1");
  delay(100);

  Serial.println("AT+CMGS=\"" + String(phoneNumber) + "\"");
  delay(100);
  Serial.println(message);
  delay(100);
  Serial.println((char)26);
}
*/
void print_decimal(int value){
  if(value < 10){
    lcd.print("0");
  }
  lcd.print(value);
}

void print_hundrets(int value){
  if(value < 100){
    lcd.print("0");
    lcd.print(value);
    return;
  }
  if(value < 10){
    lcd.print("00");
    lcd.print(value);
    return;
  }
  if(value < 1){
    lcd.print("000");
    return;
  }
}

int bound(int value, int from, int to){
  if(value > to){
    value = from;
  }
  if(value < from){
    value = to;
  }
  return value;
}
//OTHER--------------------------------------------------------



//CAMERAS -----------------------------------------------------


void update_actual_temperature(struct Camera *camera){
  float R1 = 10000;
  float logR2, R2, T;
  float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
  logR2 = log(R1 * (1023.0 / (float)analogRead(camera -> ntc_pin) - 1.0));
  camera -> actual_temperature = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)) - 273.15;
}

void heater_working_to(struct Camera *camera, bool to){
  camera -> heating = to;
  if(camera -> inverted_relay){
    if(to){
      digitalWrite(camera -> relay_pin, LOW);
    }
    else{
      digitalWrite(camera -> relay_pin, HIGH);
    }
  }
  else{
    if(to){
      digitalWrite(camera -> relay_pin, HIGH);
    }
    else{
      digitalWrite(camera -> relay_pin, LOW);
    }
  }
}

void manage_heating(struct Camera *camera){

  update_actual_temperature(camera);
  
  if(camera->working){
    if(camera -> actual_temperature < (float)(camera -> target_temperature - camera -> hysteresis)){
      heater_working_to(camera, true);
    }
    else if(camera -> actual_temperature > (float)(camera -> target_temperature + camera -> hysteresis)){
      heater_working_to(camera, false);
    }
  }
  else{
    heater_working_to(camera, false);
  }
}


void manage_timer_of(struct Camera *camera, unsigned long prev_time){
    camera -> seconds_left = (camera -> seconds_left * 1000 + float(millis() - prev_time))/1000;
        
    if(camera -> seconds_left > 60.0){
      camera -> minutes_left -= 1;
      camera -> seconds_left = 0.0;
    }
    if(camera -> minutes_left < 0){
      if(camera->hours_left > 0 || camera -> days_left > 0){
        camera -> minutes_left = 59;
        camera->hours_left -= 1;
      }
      else{
        camera -> minutes_left = 0;
      }
    }

    if(camera -> hours_left < 0){
      if(camera -> days_left > 0){
        camera -> hours_left = 23;
        camera -> days_left -= 1;
      }
      else{
        camera -> hours_left = 0;
      }
    }

    if(camera -> days_left < 0){
          camera -> days_left = 0;
    }
  
}

void manage_working(struct Camera *camera){
  if(get_time_in_minutes(camera) == 0){
//    if(!camera -> finished){
//      sendSMS("Walter White:\nФерментация сырья в ячейке номер "+String(camera -> number) + " завершена.");
//    }
    camera -> working = false;
  }
  else{
    camera -> working = true;
  }

}

void fermenter_initialisation(){
    for (int i = 0; i < 5; i++) {

      (camera0 + i) -> days_left = 0;
      (camera0 + i) -> hours_left = 0;
      (camera0 + i) -> minutes_left = 0;
      (camera0 + i) -> seconds_left = 0.0;
      (camera0 + i) -> required_time = 0;

      (camera0 + i) -> hysteresis = 2.0;
      (camera0 + i) -> heating = false;
      (camera0 + i) -> number = i + 1;
      (camera0 + i) -> working = false;

      update_actual_temperature(camera0 + i);
      (camera0 + i) -> target_temperature = (int)((camera0 + i) -> actual_temperature - 5.0);
    }
}

//CAMERAS -----------------------------------------------------



//BUTTONS-------------------------------------------------------

bool is_currently_pressed(struct Button *but){
  return but->low_level <= analogRead(A0) && analogRead(A0) < but->high_level;
}

void get_button_current_state(struct Button *but){
  but->cur_state = but->low_level <= analogRead(A0) && analogRead(A0) < but->high_level;
  but->triggered = but->cur_state && !but->prev_state;
}

bool was_triggered(struct Button *but){
  if(but->triggered){
    but->triggered = false;
    return true;
  }
  return false;
}


bool is_pressed(struct Button *but){

  if(but->cur_state && but->prev_state){
    but->iterations_counted ++;
  }
  else{
    but->iterations_counted = 0;
  }

  if(but->iterations_counted >= button_press_level){
    return true;
  }
  return false;
}

bool button_action(struct Button *but){
  return was_triggered(but) || is_pressed(but);
}

//BUTTONS------------------------------------------------------

//UI-----------------------------------------------------------
void show_parameter_menue_of(struct Camera *camera){
  lcd.setCursor(0,0);
  lcd.write(byte(0));
  lcd.write(byte(3));
  lcd.write(byte(0));
  lcd.print("-");
  lcd.print(camera -> number);
  lcd.print("-");
  lcd.setCursor(7,0);

  print_decimal(camera -> days_left);
  lcd.write(byte(4));

  print_decimal(camera -> hours_left);
  lcd.write(byte(5));
  
  print_decimal(camera -> minutes_left);
  lcd.print("m");



  lcd.setCursor(0,1);
  lcd.write(byte(0));
  if(camera -> heating){
    lcd.write(byte(2));
  }
  else{
    lcd.print("_");
  }
  lcd.write(byte(0));

  print_decimal((int)camera -> actual_temperature);
  lcd.write(1);


  lcd.setCursor(7,1);
  lcd.print("T");
  print_decimal((int)camera -> target_temperature);
  lcd.write(byte(1));

  lcd.setCursor(12,1);
  lcd.write(byte(7));
  print_decimal((int)camera -> hysteresis);
  lcd.write(byte(1));
}

void show_editor(struct Camera *camera){
  
  short unsigned int supliment = 0;

  if(button_action(button_right)){
    if(edditing_parameter_id < 4){
      edditing_parameter_id += 1;
    }
  }
  if(button_action(button_left)){
    if(edditing_parameter_id > 0){
      edditing_parameter_id -= 1;
    }
  }
  if(button_action(button_up)){
      supliment = 1;
  }
  if(button_action(button_down)){
      supliment = -1;
  }

  switch (edditing_parameter_id) {
    case 0:
      lcd.setCursor(7,1);
      camera -> target_temperature = bound(camera -> target_temperature + supliment, 25, 60);
      break;
    case 1:
      lcd.setCursor(12,1);
      camera -> hysteresis = bound(camera -> hysteresis + supliment, 1, 10);
      break;
    case 2:
      lcd.setCursor(9,0);
      camera -> days_left = bound(camera -> days_left + supliment, 0, 10);
      camera -> required_time = get_time_in_minutes(camera);
      break;
    case 3:
      lcd.setCursor(12,0);
      camera -> hours_left = bound(camera -> hours_left + supliment, 0, 23);
      camera -> required_time = get_time_in_minutes(camera);
      break;
    case 4:
      lcd.setCursor(15,0);
      camera -> minutes_left = bound(camera -> minutes_left + supliment, 0, 59);
      camera -> required_time = get_time_in_minutes(camera);
      break;
  }
}

void show_main_menue(){
    for (int i = 0; i < 5; i++) {
      switch(i){
        case 0:
          lcd.setCursor(0, 0);
          break;
        case 1:
          lcd.setCursor(0, 1);
          break;
        case 2:
          lcd.setCursor(9, 0);
          break;
        case 3:
          lcd.setCursor(9, 1);
          break;
      }
      
      Camera *camera0_p_copy = camera0 + i;
      
      lcd.print(camera0_p_copy -> number);
      lcd.print(":");

      if(camera0_p_copy -> required_time == 0){
        lcd.print("ready");
      }
      else{
        if(!camera0_p_copy -> working){
          lcd.print("done");
          lcd.write((byte)6);
        }

        else{
          print_hundrets( 100 - (int)( 100.0 * ((float)get_time_in_minutes(camera0_p_copy)) / ((float)(camera0_p_copy -> required_time)) ));
          lcd.print("%");
          if(camera0_p_copy -> heating){
            lcd.write((byte)2);
          }
          else{
            lcd.print("_");
          }
        }
      }

  }
}

//UI---------------------------------------------------------------







void setup() {
  Serial.begin(9600);

  lcd.begin(16, 2);              // start the library
  lcd.setCursor(0,0);
  
  lcd.print("Initialisation..."); // print a simple message



  button_up -> low_level = 50;
  button_up -> high_level = 150;
  button_up -> cur_state = false;
  button_up -> prev_state = false;

  button_down -> low_level = 150;
  button_down -> high_level = 300;
  button_down -> cur_state = false;
  button_down -> prev_state = false;
  
  button_left -> low_level = 300;
  button_left -> high_level = 500;
  button_left -> cur_state = false;
  button_left -> prev_state = false;
  
  button_right -> low_level = 0;
  button_right -> high_level = 50;
  button_right -> cur_state = false;
  button_right -> prev_state = false;
  
  button_select -> low_level = 500;
  button_select -> high_level = 750;
  button_select -> cur_state = false;
  button_select -> prev_state = false;

  lcd.createChar(0, WALL);
  lcd.createChar(1, CELSIUS);
  lcd.createChar(2, FIRE);
  lcd.createChar(3, TEA);
  lcd.createChar(4, DAYS);
  lcd.createChar(5, HOURS);
  lcd.createChar(6, DONE);
  lcd.createChar(7, DELTA);

  

  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  //pinMode!!!!

  fermenter[2].inverted_relay = true;
  fermenter[3].inverted_relay = true;
  
  fermenter[0].ntc_pin = A5;
  fermenter[1].ntc_pin = A3;
  fermenter[2].ntc_pin = A4;
  fermenter[3].ntc_pin = A2;

  fermenter[0].relay_pin = 2;
  fermenter[1].relay_pin = 3;
  fermenter[2].relay_pin = 11;
  fermenter[3].relay_pin = 12;

  fermenter_initialisation();

  lcd.clear();

}

const int waiting_wime = 500000;




int camera_p_offcet = 0;

int iterations_to_wait = 30;
int iterated_times = 0;
bool displaying_main_menue = true;

void loop() {
  long unsigned initial_time = millis();

  get_button_current_state(button_up);
  get_button_current_state(button_down);
  get_button_current_state(button_right);
  get_button_current_state(button_left);
  get_button_current_state(button_select);


  if(displaying_main_menue){
    camera_p_offcet = 0;
  }


  if(!displaying_main_menue && button_action(button_select)){
    if(!editting){
      editting = true;
      lcd.blink();
      show_editor(camera0 + camera_p_offcet);
    }
    else{
      editting = false;
      lcd.noBlink();
    }
  }


  if(is_currently_pressed(button_right) || is_currently_pressed(button_left)){
    iterated_times = iterations_to_wait;
    displaying_main_menue = false;
    lcd.clear();
  }
    

  if(!editting){

    if(iterated_times > 0){
      if(camera_p_offcet < 3 && button_action(button_right)){
        camera_p_offcet ++;
      }
      if(camera_p_offcet > 0 && button_action(button_left)){
        camera_p_offcet --;
      }

      show_parameter_menue_of(camera0 + camera_p_offcet);
      iterated_times --;
      if(iterated_times == 0){
        lcd.clear();
      }
    }
    else{
      show_main_menue();
      displaying_main_menue = true;
    }

  }
  else{
    show_parameter_menue_of(camera0 + camera_p_offcet);
    show_editor(camera0 + camera_p_offcet);
  }

 




  delay(160);

  for(int p = 0; p < 4; p++){
    if(!editting){
      manage_timer_of(camera0 + p, initial_time);
      manage_working(camera0 + p);
    }
    manage_heating(camera0 + p);
  }



  button_up -> prev_state = button_up -> cur_state;
  button_down -> prev_state = button_down -> cur_state;
  button_right -> prev_state = button_right -> cur_state;
  button_left -> prev_state = button_left -> cur_state;
  button_select -> prev_state = button_select -> cur_state;

}







