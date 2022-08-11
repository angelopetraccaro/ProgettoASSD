package REST;

import javax.xml.bind.annotation.XmlRootElement;

@XmlRootElement
public class Stato {

	private int postiLiberi;

	public Stato() {
	}
	
	public Stato(int postiLiberi) {
		super();
		this.postiLiberi = postiLiberi;
	}

	@Override
	public String toString() {
		return "Stato [postiLiberi=" + postiLiberi + "]";
	}

	public int getPostiLiberi() {
		return postiLiberi;
	}

	public void setPostiLiberi(int postiLiberi) {
		this.postiLiberi = postiLiberi;
	}
	
}
