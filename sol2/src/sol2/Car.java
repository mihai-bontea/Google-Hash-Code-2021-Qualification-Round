package sol2;

import java.util.List;

public class Car {
	public final int nrStreets;
	public final List<Street> streets;
	
	public Car(int nrStreets, List<Street> streets){
		this.nrStreets = nrStreets;
		this.streets = streets;
	}
}