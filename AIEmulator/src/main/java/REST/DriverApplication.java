package REST;

import java.util.HashSet;
import java.util.Set;

import javax.ws.rs.ApplicationPath;
import javax.ws.rs.core.Application;


@ApplicationPath("/DriverRest")
public class DriverApplication extends Application{

	public Set<Class<?>> getClasses() {
		Set<Class<?>> s = new HashSet<Class<?>>();
		s.add(RestDriverImplementation.class);
		return s;
	}
	
	public Set<Object> getSingletons() {
		Set<Object> s = new HashSet<Object>();
		s.add(RestDriverImplementation.class);
		return s;

	}
}
