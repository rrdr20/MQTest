FROM alpine:latest
RUN mkdir -p /app
WORKDIR /app
ADD ./subscriber /app
CMD ["./subscriber", "--host", "mqtt-server", "--topic", "test/go"]
