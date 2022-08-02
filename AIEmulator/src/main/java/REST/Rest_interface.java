package REST;


import javax.ws.rs.Consumes;
import javax.ws.rs.GET;
import javax.ws.rs.POST;
import javax.ws.rs.Path;
import javax.ws.rs.PathParam;
import javax.ws.rs.Produces;
import javax.ws.rs.core.Response;

@Consumes("application/json")
@Produces("application/json")
@Path("/status")
public interface Rest_interface {

	
	@GET
	public Stato getStatus();
	
	// public Response getStatus();//@PathParam("id") int id);
	/*
	 @GET
	 @Path("/{id}")
	 public String getString(@PathParam("id") int id);
	 
	 */
}
