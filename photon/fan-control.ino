volatile unsigned int numPulses;

static int lastRPM;

static unsigned long startTime, currentTime;

static bool changeDuty;

const static int TACH_PIN = 1;
const static int DUTY_PIN = 3;

const static void isrReadTach(void) {
    numPulses++;
}

void setup() {

    numPulses = 0;
    lastRPM = 0;
    
    pinMode(DUTY_PIN, OUTPUT);
    pinMode(TACH_PIN, INPUT_PULLUP);
    attachInterrupt(TACH_PIN, isrReadTach, FALLING);

    changeDuty = false;
    startTime = millis();

}

void loop() {
    
    while ((currentTime = millis()) - startTime <= 40000) {
        noInterrupts();
        lastRPM = (numPulses / 2) * 30;
        numPulses = 0;
        Particle.publish("RPM", String(lastRPM), 60, PRIVATE);
        interrupts();
        delay(2000);
    }
    
    if(changeDuty) {
        Particle.publish("50%", NULL, 60, PRIVATE);
        analogWrite(DUTY_PIN, 128, 25000);
        changeDuty = !changeDuty;
    }
    else {
        Particle.publish("100%", NULL, 60, PRIVATE);
        analogWrite(DUTY_PIN, 255, 25000);
        changeDuty = !changeDuty;
    }

    startTime = millis();
}

