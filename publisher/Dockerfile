FROM alpine:latest
RUN mkdir -p /app
WORKDIR /app
ADD ./publisher /app
CMD ["./publisher", "--host", "mqtt-server", "--topic", "test/go"]
