# MQTest

Testing paho.mqtt.golang library, paho.mqtt.embedded-C library and using the ESP32 Microcontroller to controll fans based on various criteria.

## Docker

- (complete) Build out Dockerfiles for each application.
- (complete) Update go applications to use docker networking
- (complete) Build docker-compose.yml

## GO

- Binaries are statically compilied
  - CGO_ENABLED=0 GOOS=linux go build -ldflags "-w -s" -o <executable_name>
- Binaries are included in repo for docker-compose
  - Need to develop a better way of building and then including with Docker as necessary. (in progress)

## ESP32

- Must install and configure the Xtensa toolkit and setup the build environment.
- mcu-mq sets up the wireless network, establishes a connection with a MQTT server and then publishes messages about once every three seconds.
- pwm-test has two parts:
  - First part reads tach signal from GPIO pin and calculates the RPM of a fan (2 pulses per revolution).
  - Second part will implement functions to control fan speed via PWM. (in progress)

## Future plans

- Complete the design for circuits.
- Implement system for logs and data reporting (Kibana? Grafana?)

