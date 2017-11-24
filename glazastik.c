ubyte data[100];
int u = 0;
void BT()
{
	while(true)
  {
    while (cCmdMessageGetSize(mailbox1) < 1) wait1Msec(1);
    cCmdMessageRead(data, 2, mailbox1);
    u = data[0] - 100;
    wait1Msec(30);
  }
}

/*task PregCamera()
{
	int mA = 0;
	nMotorEncoder[motorA] = 0;
	while(true)
	{
		mA = ((data[1] - 100) - nMotorEncoder[motorA]) * 0.1;
		motor[motorA] = mA;
		nxtDisplayTextLine(3, "%d", mA);
		wait1Msec(10);
	}
}*/
void omni_go(float v,int a)
{
  a=180+a;
  int m1_sp = v*sinDegrees(90-a);
  if(m1_sp>100) m1_sp=100;
  if(m1_sp<-100) m1_sp=-100;

  int m2_sp = v*sinDegrees(210-a);
  if(m2_sp>100) m2_sp=100;
  if(m2_sp<-100) m2_sp=-100;

  int m3_sp = v*sinDegrees(330-a);
  if(m3_sp>100) m3_sp=100;
  if(m3_sp<-100) m3_sp=-100;

  motor[motorA]=m1_sp;
  motor[motorB]=m2_sp;
  motor[motorC]=m3_sp;
}
void motor_stop()
{
	motor[motorA]=0;
  motor[motorB]=0;
  motor[motorC]=0;
}
task main()
{
	float kp=2;
	float kd=0;
	while(true)
  {
    while (cCmdMessageGetSize(mailbox1) < 1) wait1Msec(1);
    cCmdMessageRead(data, 3, mailbox1);
    int err = data[0];
    int angle = data[1];
    if(data[2]==1) angle = -angle; //negative angle
    float u = kp*(float)err+
  }

}
