#include <ArduinoJson.h>
#include "Task.h"
#include "Command.h"

int sensorReadInterval = 1000;
int sensorAvgCount = 30;
int LedBlinkInterval = 3000;
int LedBlinkDuration = 10;
int SensorReportInterval = sensorAvgCount * sensorReadInterval;
long lightLevelAccumulator = 0;
int lightLevelAccCount = 0;

long loopCount = 0;   //Test code

void ReadSensors();
void ReportSensors();
void SetLED();
void ClearLED();

Task ReadSensorTask(sensorReadInterval, ReadSensors);
Task ReportSensorTask(SensorReportInterval, ReportSensors);
Task BlinkLedTask(LedBlinkInterval, SetLED, LedBlinkDuration, ClearLED);

Task* tasks[]
{
  &ReadSensorTask,
  &ReportSensorTask,
  &BlinkLedTask
};

void UpdateLedDutyCycle(String args);
void UpdateSensorDutyCycle(String args);

Command commands[]
{
  Command("sl", UpdateLedDutyCycle),
  Command("ss", UpdateSensorDutyCycle),
  Command("setled ", UpdateLedDutyCycle),
  Command("setsensor ", UpdateSensorDutyCycle)
};
const int NUM_COMMANDS = sizeof(commands)/sizeof(commands[0]);

void setup()
{
  Serial.begin(9600);
  pinMode(2, OUTPUT);
}

void loop()
{
  for (Task* t : tasks) t->Run();
  Command::ReadCommand(commands, NUM_COMMANDS);

  // loopCount++;                        //Test code begin
  // if (loopCount % 100000 == 0)
  // {
  //   Serial.print("Loop count = ");
  //   Serial.print(loopCount / 100000);
  //   Serial.print(" hundred thousand");
  //   Serial.println();                 //Test code end
  // }
}

//***********************
// Task Methods
//***********************

void ReadSensors()
{
  lightLevelAccumulator += analogRead(A0);
  lightLevelAccCount++;
}

void ReportSensors()
{
  char buf[30];
  StaticJsonDocument<100> jsonBuffer;

  if (lightLevelAccCount==0) ReadSensors();

  jsonBuffer["lightlevel"] = lightLevelAccumulator / lightLevelAccCount;
  lightLevelAccumulator = 0;
  lightLevelAccCount = 0;
  jsonBuffer["Sensor"]["Interval"] = sensorReadInterval;
  jsonBuffer["Sensor"]["AvgSamples"] = sensorAvgCount;
  jsonBuffer["LedBlink"]["Interval"] = LedBlinkInterval;
  jsonBuffer["LedBlink"]["Duration"] = LedBlinkDuration;

  serializeJson(jsonBuffer, Serial);
  Serial.println();
}

void SetLED() { SetLED(true); }
void ClearLED() { SetLED(false); }

//***********************
// Command Methods
//***********************

void UpdateLedDutyCycle(String args)
{
  int n1, n2;
  Get2Numbers(args, n1, n2);
  if (n1 > 0 && n2 > 0 && n1 > n2)
  {
    LedBlinkInterval = n1;
    LedBlinkDuration = n2;
    BlinkLedTask.SetInterval(n1);
    BlinkLedTask.SetDuration(n2);
    ReportSensors();
  }
}

void UpdateSensorDutyCycle(String args)
{
  int n1, n2;
  Get2Numbers(args, n1, n2);
  if (n1 > 0 && n2 > 0)
  {
    sensorReadInterval = n1;
    sensorAvgCount = n2;
    SensorReportInterval = n2 * n1;
    ReadSensorTask.SetInterval(n1);
    ReportSensorTask.SetInterval(SensorReportInterval);
    ReportSensors();
  }
}

//***********************
// Private Methods
//***********************

void SetLED(bool on)
{
    digitalWrite(2, on ? HIGH : LOW);
}

void Get2Numbers(String s, int& n1, int& n2)
{
  n1 = atoi(s.c_str());
  n2 = atoi(s.c_str()+s.indexOf(",")+1);
}
