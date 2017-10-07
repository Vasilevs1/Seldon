#define MODE_AUTO 0
#define MODE_MANUAL 1

#define MOTOR_LEFT 0
#define MOTOR_RIGHT 1

#define DIR_FORW 2
#define DIR_BACKW 1
#define BRAKEGND 3 //остановка по gnd

byte cont_byte;

uint8_t inApin[2] = {7, 4};  // INA: по часовой стрелке.
uint8_t inBpin[2] = {8, 9}; // INB: против часовой стрелки.
uint8_t pwm_pin[2] = {5, 6}; // PWM: Шим
uint8_t encA_pin[2] = {10, 11}; // пины энкодера 1
uint8_t encB_pin[2] = {12, 13};  //пины энкодера 2

bool rand_move_active = true; //режим псевдо-случайных движений

bool going[4] = {false,false,false,false};

uint16_t degreeA = 0, degreeB = 0, turn_amount = 15;
unsigned long kek = 0;
int cont = 0;
int reg_t = 0;
uint32_t tim =0; 
void setup()
{
  for (int i = 0; i < 2; i++)
  {
    pinMode(inApin[i], OUTPUT);
    pinMode(inBpin[i], OUTPUT);
    pinMode(pwm_pin[i], OUTPUT);
    pinMode(encA_pin[i], INPUT);
    pinMode(encB_pin[i], INPUT);
  }
  pinMode(3,OUTPUT); 
  motor_off(MOTOR_LEFT);
  motor_off(MOTOR_RIGHT);
  Serial.begin(9600);
  Serial1.begin(9600);
  tim = millis();
}

void loop() {
  
    /*if(Serial.available())
    {
      cont_byte = Serial.read(); //считать контролирующий байт: последний разряд - режим, предпоследний - выбор мотора, след. - направление вращ., остальные - мощность
      if(set_mode(cont_byte) == MODE_MANUAL) //если был включен ручной режим
      {
        if(rand_move_active) rand_move_active = false; // выключить случаный режим
        uint8_t power = set_power(cont_byte);
        power = map(power,0,31,0,255);
        motor_go(set_motor(cont_byte),set_direction(cont_byte),power); //подать воздействие на моторы
      }
      else rand_move_active = true; //иначе включить режим рандомных движений
    }*/
   if(Serial1.available())
  {
    //1-вперед
    //2-назад
    //3-влево
    //4-вправо
    uint8_t dir = Serial1.read();
    switch(dir)
    {
      case 1:
        if(!going[0])
        {
          motor_off(MOTOR_LEFT);
          motor_off(MOTOR_RIGHT);
          m_move(MOTOR_LEFT,180);
          m_move(MOTOR_RIGHT,180);
          going[0] = true;
        }
      break;
      case 2:
        if(!going[1])
        {
          motor_off(MOTOR_LEFT);
          motor_off(MOTOR_RIGHT);
          m_move(MOTOR_LEFT,-180);
          m_move(MOTOR_RIGHT,-180);
          going[1]=true;
        }
      break;
      case 3:
        if(!going[2])
        {
            motor_off(MOTOR_LEFT);
            motor_off(MOTOR_RIGHT);
            m_move(MOTOR_LEFT, -180);
            m_move(MOTOR_RIGHT, 180);
            going[2] = true;
        }
      break;
      case 4:
        if(!going[3])
        {
          motor_off(MOTOR_LEFT);
          motor_off(MOTOR_RIGHT);
          m_move(MOTOR_RIGHT, -180);
          m_move(MOTOR_LEFT, 180);
          going[3]=true;
        }
      break;
      case 5:
        cont = 0;
        reg_t = 0;
        digitalWrite(3,HIGH);
      break;
      case 6:
        motor_off(MOTOR_LEFT);
        motor_off(MOTOR_RIGHT);
        going[0]=false;
      break;
      case 7:
        motor_off(MOTOR_LEFT);
        motor_off(MOTOR_RIGHT);
        going[1]=false;      
      break;
      case 8:
        motor_off(MOTOR_LEFT);
        motor_off(MOTOR_RIGHT);
        going[2]=false;      
      break;
      case 9:
        motor_off(MOTOR_LEFT);
        motor_off(MOTOR_RIGHT);
        going[3]=false;      
      break;
    }
    //m_move(MOTOR_RIGHT, cont + reg_t);
    //m_move(MOTOR_LEFT, cont - reg_t);
    //Serial.print(cont+reg_t);
    kek = 0;
  }
  else
  {
    kek++;
    if(kek > 400)
    {
      Serial.println("sosat+leshat");
      motor_off(MOTOR_LEFT);
      motor_off(MOTOR_RIGHT);
      for(int i=0;i<4;i++)
      {
        going[i] = false;
      }
    }
    delay(2);
  }
  
  

}

uint8_t set_mode(byte mode_byte) //установка режима
{
  if ((mode_byte & B10000000) != 0) // в разряде 1 - ручной режим
  {
    return MODE_MANUAL;
  }
  else  // в разряде 0 - автоматический режим
  {
    return MODE_AUTO;
  }
}
uint8_t set_motor(byte mot_byte) //установка мотора
{
  if ((mot_byte & B01000000) != 0 ) //в разряде 1 - правый мотор
  {
    return MOTOR_RIGHT;
  }
  else  //в разряде 0 - левый мотор
  {
    return MOTOR_LEFT;
  }
}
uint8_t set_direction(byte dir_byte) //установка направления
{
  if ((dir_byte & B00100000) != 0) //в разряде 1 - назад
  {
    return DIR_BACKW;
  }
  else //в разряде 0 - вперед
  {
    return DIR_FORW;
  }
}
uint8_t set_power(byte pow_byte) //установка мощности
{
  return (pow_byte & B00011111);
}

void motor_off(int motor)
{
  digitalWrite(inApin[motor], LOW); // отключаем питание по ноге А конкретного мотора
  digitalWrite(inBpin[motor], LOW); // отключаем питание по ноге В конкретного мотора
  analogWrite(pwm_pin[motor], 0);    // принудительно подаем 0 на шим конкретного мотора
}
void motor_go(uint8_t motor, uint8_t direct, int pwm)
{
  if ((motor <= 1) && (direct <= 3)) // предохранитель, только мотор 0 или 1, а направление 0,1,2,3.
  {
    // Set inA[motor]
    if (direct <= 1)
      digitalWrite(inApin[motor], HIGH);
    else
      digitalWrite(inApin[motor], LOW);

    // Set inB[motor]
    if ((direct == 0) || (direct == 2))
      digitalWrite(inBpin[motor], HIGH);
    else
      digitalWrite(inBpin[motor], LOW);

    analogWrite(pwm_pin[motor], pwm);
  }
}
void forw_45()
{
  degreeA = 0;
  degreeB = 0; //обнуляем градусные переменные
  uint8_t encA_prev = 0;
  uint8_t encB_prev = 0;
  while(degreeA < 45 && degreeB < 45)
  {
    uint8_t encA = digitalRead(encA_pin[0]);//считать первый энкодер
    uint8_t encB = digitalRead(encB_pin[0]);//считать второй энкодер
    if((!encA)&&(encA_prev))
    {
      if(degreeA+turn_amount <= 32752) degreeA+=turn_amount;//прибавить градус поворота одного мотора
    }
    if((!encB)&&(encB_prev))
    {
      if(degreeB+turn_amount <= 32752) degreeB+=turn_amount;//прибавить градус поворота второго мотора
    }
    motor_go(MOTOR_LEFT,DIR_FORW,160);
    motor_go(MOTOR_RIGHT,DIR_FORW,160);
    encA_prev = encA;
    encB_prev = encB;
  }
  motor_off(MOTOR_LEFT);
  motor_off(MOTOR_RIGHT);
  delay(5000);
}
void m_move(uint8_t m,int val)
{
  if(m <= 1)
  {
    if(val < 0)
    {
      motor_go(m,DIR_BACKW,constrain(-val,0,255));
    }
    else
    {
      motor_go(m,DIR_FORW,constrain(val,0,255));
    }
  }
}

