/*
 * publish - Test connection to MQTT server and publishes messages to the topic.
 *
 * Loops indefinitely and publishes a message every 3 seconds to specified topic.
 *
 * TODO:
 *   - Add flag for message and repeat count
 *   - Add control statements for clean shutdown
 */

package main

import (
    "os"
    "fmt"
    "flag"
    "time"
    "github.com/eclipse/paho.mqtt.golang"
)

func main() {
    host := flag.String("host","localhost","Host name of the MQTT server")
    topic := flag.String("topic", "#", "The topic where messages are published")
    flag.Parse()

    hostname, err := os.Hostname();
    if err != nil {
        fmt.Println(err)
    }

    fmt.Println("Publisher:", hostname)

    opt := mqtt.NewClientOptions()
    opt.AddBroker(fmt.Sprintf("tcp://%s:1883", *host))

    client := mqtt.NewClient(opt)

    if token:= client.Connect(); token.Wait() && token.Error() != nil {
        fmt.Println(token.Error())
    }

    for i := 0; ; i++ {
	token := client.Publish(*topic, 0, false, fmt.Sprintf("publish test messages %d", i))
        token.Wait()

	time.Sleep(3 * time.Second)
    }
}
