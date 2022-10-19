package WEBHOOK;

import java.io.IOException;

import org.apache.http.HttpResponse;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.HttpClients;
import org.codehaus.jettison.json.JSONObject;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttMessage;

import DAO.StatoDAO;
import DAO.StatoDAOImpl;
import REST.AI_Thread;
import model.Stato;

public class MyCallbackWebhook implements MqttCallback
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
	 System.out.println("Ho ricevuto dal topic: " + topic + " il seguente messaggio: " + s );
	 
	//estrarre contenuto del messaggio
	JSONObject obj = new JSONObject(message.toString());
 
	// crea thread
	AI_Thread thread=new AI_Thread(obj.getInt("id"), obj.getString("mod"));
	thread.run();  
	
	//lettura stato dal db
	StatoDAO daoImpl = new StatoDAOImpl();
	Stato state=daoImpl.getStato(obj.getInt("id"));
			
	String url="http://192.168.43.64";	//192.168.0.105		 
	
	HttpClient httpclient = HttpClients.createDefault();
    HttpPost post = new HttpPost(url);
	try {
		post.setEntity(new StringEntity(state.toString()));
		response = httpclient.execute(post);
		//quando 500 error, decrem/increm capienza
		System.out.println(response.getStatusLine());
		
		if(response.getStatusLine().toString().equals("HTTP/1.1 500 Internal Server Error")) {
			if(obj.getString("mod").equals("entra")&& StatoDAOImpl.capienza>0) {
				StatoDAOImpl.capienza+=1;
				System.out.println("aggiungo 1 a capienza");
			}
		
		System.out.println("capienza multi_thread: "+StatoDAOImpl.capienza);
		}
	} catch (IOException e) {
		// TODO Auto-generated catch block
		e.printStackTrace();
	}
 }
 private HttpResponse response;
}

