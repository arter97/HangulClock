#include <DS1307RTC.h>
#include <Time.h>
#include <Wire.h>

const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

tmElements_t tm;

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    delay(200);

  Serial.println("DS1307 configured");
  readTime();

  // get the date and time the compiler was run
  if (getDate(__DATE__) && getTime(__TIME__)) {
    // and configure the RTC with this info
    RTC.write(tm);
    Serial.println("DS1307 set!");
  }
    
  Serial.println("DS1307 configured");
  readTime();
}

void loop()
{
  readTime();
  delay(1000);
}

void readTime()
{
  if (RTC.read(tm)) {
    Serial.print("Ok, Time = ");
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.println();
  } else {
    if (RTC.chipPresent()) {
      Serial.
          println
          ("The DS1307 is stopped.  Please run the SetTime");
      Serial.println();
    } else {
      Serial.
          println
          ("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
    delay(1000);
  }
}

void print2digits(int number)
{
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

bool getTime(const char *str)
{
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3)
    return false;
  tm.Hour = Hour;
  tm.Minute = Min;
  tm.Second = Sec + 5; // Assume 5 seconds delay from build, upload and execute
  return true;
}

bool getDate(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3)
    return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0)
      break;
  }
  if (monthIndex >= 12)
    return false;
  tm.Day = Day;
  tm.Month = monthIndex + 1;
  tm.Year = CalendarYrToTm(Year);
  
  return true;
}
