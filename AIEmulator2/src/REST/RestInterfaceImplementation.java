package REST;

import javax.ws.rs.core.Response;

import DAO.StatoDAO;
import DAO.StatoDAOImpl;
import model.Stato;


public class RestInterfaceImplementation implements Rest_interface {
	@Override
	public Response getStatus(String id) {
	 try {
		 System.out.println("vengo chiamato REST");
		 // legge dal db
		 StatoDAO daoImpl = new StatoDAOImpl();
		 Stato state = daoImpl.getStato(Integer.parseInt(id));
		 if(state==null)
			 return Response.status(404).build();
		
		 else if (state.getCapienza()==-1)
			 return Response.status(404).build();
		 
		 else 
			 return Response.status(200).entity(state).build(); //crea un msg di risp con lo stato nel body, come status code mette 200 ok
	 }catch (Exception e) {
	 }
	return null;  
	}
}
