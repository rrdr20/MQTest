/*
 * subscribe - Test connection to MQTT server and prints messages received on speificed topic.
 * 
 * TODO:
 *   - Add flag for specifying servier connection details
 *   - Add flag for specificy topic to subscribe
 */

package main

import (
    "fmt"
    "os"
    "os/signal"
    "syscall"
    "github.com/eclipse/paho.mqtt.golang"
)

func test_message_handler(client mqtt.Client, msg mqtt.Message) {
    rec_msg := fmt.Sprintf("Topic: %s, Message: %s", msg.Topic(), msg.Payload())
    fmt.Println(rec_msg)
}

func main() {
    c := make(chan os.Signal, 1) 
    signal.Notify(c, os.Interrupt, syscall.SIGTERM)

    fmt.Println("MQTest: Subscribe")
    fmt.Println()

    opt := mqtt.NewClientOptions()
    opt.AddBroker("tcp://localhost:1883")

    client := mqtt.NewClient(opt)

    if token:= client.Connect(); token.Wait() && token.Error() != nil {
        fmt.Println(token.Error())
    }

    token := client.Subscribe("test/go", 0, test_message_handler)
    token.Wait()

    client.Disconnect(250)

    <-c
}
