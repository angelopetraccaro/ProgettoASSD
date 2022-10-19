package model;

import javax.xml.bind.annotation.XmlRootElement;

@XmlRootElement
public class Stato {

	private int id;
	private int capienza;

	@Override
	public String toString() {
		return "Stato [id=" + id + ", capienza=" + capienza + "]";
	}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public Stato() {
	}

	public Stato(int id, int capienza) {
		super();
		this.id = id;
		this.capienza = capienza;
	}

	public int getCapienza() {
		return capienza;
	}

	public void setCapienza(int capienza) {
		this.capienza = capienza;
	}
}
