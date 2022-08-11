package REST;

import javax.ws.rs.core.Response;

public class RestDriverImplementation implements Rest_interface {

	@Override
	public Stato getStatus() {
		// preparare json con lo stato 
		
		return new Stato(6);
		//return Response.ok("ciao, sono lo stato").build();
	}

}
