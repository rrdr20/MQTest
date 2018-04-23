# MQTest

Testing paho.mqtt.golang library, paho.mqtt.embedded-C library and using the ESP32 Microcontroller to control fans based on various criteria.

## Docker

- (complete) Build out Dockerfiles for each application.
- (complete) Update go applications to use docker networking
- (complete) Build docker-compose.yml

## GO

- Binaries are statically compiled
  - CGO_ENABLED=0 GOOS=linux go build -ldflags "-w -s" -o <executable_name>
- Binaries are included in repo for docker-compose
  - (in progress) Need to develop a better way of building and then including with Docker as necessary.

## ESP32

- Must install and configure the Xtensa toolkit and setup the build environment.
- (in progress) Schematic for circuits to be prepared and uploaded.

### mcu-mq

  - Sets up the wireless network and establishes a connection with a MQTT server. 
  - Publishes test messages about once every three seconds.

### pwm-test

  - During testing the PWM generated signal would cause incorrect results in the tach signal readings and was rectified by placing a 10pF capacitor between the tach pin and ground.
  - First part reads tach signal from GPIO pin and calculates the RPM of a fan (2 pulses per revolution).
  - (in progress) Second part will implement functions to control fan speed via PWM. 

### th-test

  - A DHT-11 temperature and humidity sensor is connected to the ESP32 and uses the RMT features to collect readings.
  - Currently the ESP32 APB operates at 80MHz so the values have been adjusted for RMT.
  - The code use to parse the data is repetitive and should be refactored.
  - The DHT-11 sensor used for testing seems to be faulty and returns incorrect values for humidity although the temperature readings seem to fall within the margin of error.

## Photon

- The code setups up the pins for reading tach signal and for outputting the PWM for control.
- The main loop will publish the RPM value on the cloud interface every 2 seconds
- After 40 seconds the duty cycle is change from either 50% to 100% or vice versa.

## Future plans

- Implement system for logging and data reporting (Kibana, InfluxDB, etc).
- Build out website. 
