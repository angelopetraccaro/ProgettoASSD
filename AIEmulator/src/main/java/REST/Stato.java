package REST;

import javax.xml.bind.annotation.XmlRootElement;

@XmlRootElement
public class Stato {

	private String code;
	private String description;
	
	
	public Stato(String c, String desc) {
		this.code = c;
		this.description = desc;

	}
	public Stato() {}
	
	public String getCode() {
		return code;
	}

	public void setCode(String code) {
		this.code = code;
	}

	public String getDescription() {
		return description;
	}

	public void setDescription(String description) {
		this.description = description;
	}
	@Override
	public String toString() {
		return "Stato [code=" + code + ", description=" + description + "]";
	}

	
}
