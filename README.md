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
  - Need to develop a better way of building and then including with Docker as necessary. (in progress)

## ESP32

- Must install and configure the Xtensa toolkit and setup the build environment.
- mcu-mq sets up the wireless network, establishes a connection with a MQTT server and then publishes messages about once every three seconds.
- pwm-test has two parts:
  - First part reads tach signal from GPIO pin and calculates the RPM of a fan (2 pulses per revolution).
  - Second part will implement functions to control fan speed via PWM. (in progress)
- During testing the PWM generated signal would cause incorrect results in the tach signal readings and was rectified by placing a 10pF capacitor between the tach pin and ground.
- Schematic for circuits to be prepared and uploaded.

## Photon

- The code setups up the pins for reading tach signal and for outputting the PWM for control.
- The main loop will publish the RPM value on the cloud interface and flips between 50 and 100% duty cycles. 

## Future plans

- Complete the design for circuits.
- Implement system for logs and data reporting (Kibana? Grafana?)

