package sol2;

import java.io.Serializable;

public class OutputPair implements Serializable{
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	public String name;
	public int greenDuration;
	
	public OutputPair(String name, int greenDuration) {
		this.name = name;
		this.greenDuration = greenDuration;
	}
	
	public String toString() {
		return String.format("%s %d", this.name, this.greenDuration);
	}
	
	@Override
	public boolean equals(Object other) {
		boolean retVal = false;

	    if (other instanceof OutputPair){
	    	OutputPair ptr = (OutputPair) other;
	        retVal = ptr.name == this.name;
	    }

	    return retVal;
	}
	
	@Override
    public int hashCode() {
		int hash = 7;
        hash = 17 * hash + (this.greenDuration | this.name.hashCode());
        return hash;
    }

}