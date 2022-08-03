package REST;


import javax.ws.rs.core.Response;

import org.apache.cxf.jaxrs.client.WebClient;

public class RestClient {

	public static void main(String[] args) {
		// TODO Auto-generated method stub

		WebClient client = WebClient.create("http://localhost:8081/AIEmulator/DriverRest");
		
		client.accept("application/json").type("application/json");
		client.path("/status");
		Response res = client.get();
		System.out.println("ho ricevuto : " + res.readEntity(String.class));
		System.out.println();
		
		 
		//GET /string/id
		client.reset();
	}

}
