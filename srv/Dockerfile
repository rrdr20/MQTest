FROM eclipse-mosquitto
ADD ./mosquitto.conf /mosquitto/config
RUN chown -R mosquitto:mosquitto /mosquitto
EXPOSE 1883
CMD ["/usr/sbin/mosquitto", "-c", "/mosquitto/config/mosquitto.conf"]

