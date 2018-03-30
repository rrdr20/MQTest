# MQTest

Testing paho.mqtt.golang library

## Docker

- (complete) Build out Dockerfiles for each application.
- (complete) Update go applications to use docker networking
- (complete) Build docker-compose.yml

## GO

- Binaries are statically compilied
   CGO_ENABLED=0 GOOS=linux go build -ldflags "-w -s" -o publisher 
- Binaries are included in repo for docker-compose
