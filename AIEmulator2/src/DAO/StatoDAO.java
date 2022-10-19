package DAO;

import model.Stato;

public interface StatoDAO {
	public Stato getStato(int id);
	public void updateStato(Stato state);
	public void createStato(Stato state);
}
