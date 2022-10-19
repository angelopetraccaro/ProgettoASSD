package REST;

import DAO.StatoDAOImpl;
import model.Stato;

public class AI_Thread extends Thread{

	public AI_Thread(int id,String mod) {
		super();
		this.id=id;
		this.mod=mod;
	}
	
	public void run() {
		try {
			//AI_Thread crea lo stato e poi lo salva sul DAO
			StatoDAOImpl DAOImpl = new StatoDAOImpl();
			DAOImpl.createStato(new Stato(id,-1));
			
			sleep(2000);
			 
			if(!mod.equals("esce") && StatoDAOImpl.capienza>=0 ) {
				DAOImpl.updateStato(new Stato(id,StatoDAOImpl.capienza));  
				if(StatoDAOImpl.capienza != 0) {
					StatoDAOImpl.capienza-=1;
				}
				else System.out.println("Tentato ingresso con capienza="+StatoDAOImpl.capienza);
			}
			else if(mod.equals("esce") && StatoDAOImpl.capienza<3) {
				 StatoDAOImpl.capienza+=1; 
				 DAOImpl.updateStato(new Stato(id,StatoDAOImpl.capienza));
		    }
		}catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
	
	private int id;
	private String mod;
}
