package WEBHOOK;


import java.util.UUID;

import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttAsyncClient;
import org.eclipse.paho.client.mqttv3.MqttException;

public class ParkingAgentWebhook 
{
    public static MqttAsyncClient myClient;

 public static void main( String[] args ) throws MqttException
    {

     myClient = new MqttAsyncClient("tcp://127.0.0.1:1883", UUID.randomUUID().toString());
     MyCallbackWebhook myCallback = new MyCallbackWebhook();
     myClient.setCallback(myCallback);
     
     
     IMqttToken token = myClient.connect();
     token.waitForCompletion();
     
     myClient.subscribe("topic/test", 0);
    }
}