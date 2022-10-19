package MQTT;

import org.codehaus.jettison.json.JSONObject;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import com.google.gson.Gson;

import DAO.StatoDAO;
import DAO.StatoDAOImpl;
import REST.AI_Thread;
import model.Stato;


public class MyCallbackMQTT implements MqttCallback
{

 public void connectionLost(Throwable arg0) {
  // TODO Auto-generated method stub
  
 }

 public void deliveryComplete(IMqttDeliveryToken arg0) {
  // TODO Auto-generated method stub
  
 }

 public void messageArrived(String topic, MqttMessage message) throws Exception {
	 String s = new String(message.getPayload());
		System.out.println("Ho ricevuto dal topic: " + topic + " il seguente messaggio: " + s );
	 
		//estrarre contenuto del messaggio
		JSONObject obj = new JSONObject(message.toString());
	 
		// crea thread
		AI_Thread thread=new AI_Thread(obj.getInt("id"), obj.getString("mod"));
		thread.run();  
		
		//lettura stato dal db
		StatoDAO daoImpl = new StatoDAOImpl();
		Stato state = daoImpl.getStato(obj.getInt("id"));
		
		//publish stato letto dal db
		String status = new Gson().toJson(state);
		byte[] sendData = status.getBytes("utf-8");
		ParkingAgentMQTT.myClient.publish("topic/status", sendData, 0, false);  //QOS
 }
}