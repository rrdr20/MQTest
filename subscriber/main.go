/*
 * subscribe - Test connection to MQTT server and prints messages received on speificed topic.
 *
 * Loops indefinitely and receives messages on the topic from the server.
 */

package main

import (
    "fmt"
    "flag"
    "os"
    "os/signal"
    "syscall"
    "github.com/eclipse/paho.mqtt.golang"
)

func test_message_handler(client mqtt.Client, msg mqtt.Message) {
    rec_msg := fmt.Sprintf("Topic: %s -- Message: %s", string(msg.Topic()), string(msg.Payload()))
    fmt.Println(rec_msg)
}

func main() {
    host := flag.String("host","localhost","Host name of the MQTT server")
    topic := flag.String("topic", "#", "The topic to subscribe")
    flag.Parse()

    c := make(chan os.Signal, 1) 
    signal.Notify(c, os.Interrupt, syscall.SIGTERM)

    fmt.Println("MQTest: Subscribe")
    fmt.Println()

    opt := mqtt.NewClientOptions()
    opt.AddBroker(fmt.Sprintf("tcp://%s:1883", *host))

    client := mqtt.NewClient(opt)

    if token:= client.Connect(); token.Wait() && token.Error() != nil {
        fmt.Println(token.Error())
    }

    token := client.Subscribe(*topic, 0, test_message_handler)
    token.Wait()

    <-c
}
