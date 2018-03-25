/*
 * publish - Test connection to MQTT server and publishes messages to the topic.
 *
 * TODO:
 *   - Add flag for specifying servier connection details
 *   - Add flag for topic
 *   - Add flag for message and repeat count
 */

package main

import (
    "fmt"
    "github.com/eclipse/paho.mqtt.golang"
)

func main() {
    fmt.Println("MQTest: Publish")
    fmt.Println()

    opt := mqtt.NewClientOptions()
    opt.AddBroker("tcp://localhost:1883")

    client := mqtt.NewClient(opt)

    if token:= client.Connect(); token.Wait() && token.Error() != nil {
        fmt.Println(token.Error())
    }

    for i := 0; i < 5; i++ {
        pub_msg := fmt.Sprintf("publish test messages %d", i)
        token := client.Publish("test/go", 0, false, pub_msg)
        token.Wait()
    }

    client.Disconnect(250)
}
