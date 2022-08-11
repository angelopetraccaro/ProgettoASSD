package MQTT;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

import REST.Stato;


public class MyCallbackStatus implements MqttCallback
{

 public void connectionLost(Throwable arg0) {
  // TODO Auto-generated method stub
  
 }

 public void deliveryComplete(IMqttDeliveryToken arg0) {
  // TODO Auto-generated method stub
  
 }

 public void messageArrived(String topic, MqttMessage message) throws Exception {

	 //App.myClient.publish("/ESP-01_MQTT_client2/Output_Switch/On_Off_Value = ", message.getPayload(), 0, false);
	 String s = new String(message.getPayload());
	 System.out.println("Ho ricevuto dal topic: " + topic + " il seguente stato: " + s );
  
 }
 
}