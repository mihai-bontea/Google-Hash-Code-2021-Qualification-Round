package sol2;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class Component {
	public final List<Car> cars;
	public final Set<Integer> intersections;
	
	public Component(Car car) {
		this.cars = new ArrayList<Car>();
		this.intersections = new HashSet<Integer>();
		
		cars.add(car);
		
		for (Street street : car.streets)
			intersections.add(street.endIntersect);
	}
	
	public void addCar(Car car) {
		cars.add(car);
		for (Street street : car.streets)
			intersections.add(street.endIntersect);
	}
	
	public boolean containsIntersect(int intersect) {
		return intersections.contains(intersect);
	}
}