package REST;

import org.codehaus.jettison.json.JSONObject;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttMessage;


public class MyCallbackREST implements MqttCallback
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

	}
}