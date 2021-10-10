package sol2;

import java.util.Map;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class Task implements Runnable{
	public final Data data;
	public final Component component;
	public final Map<Integer, List<OutputPair>> outputInfo;
	public static final double MULTI = 0.5;
	
	public Task(Data data, Component component, Map<Integer, List<OutputPair>> outputInfo) {
		this.data = data;
		this.component = component;
		this.outputInfo = outputInfo;
	}
	
	public void run() {
		// Calculate the traffic for each street
		Map<Street, Integer> street_traffic = new HashMap<Street, Integer>();
		for (Car car : component.cars) {
			// Going through all streets except the last one(since the cars at last intersection
			// don't queue and just disappear)
			for (int i = 0; i < car.nrStreets - 1; ++i) {
				// If street does not appear yet, assign the traffic for it to 1
				if (!street_traffic.containsKey(car.streets.get(i)))
					street_traffic.put(car.streets.get(i), 1);
				// Otherwise, increase the traffic
				else
					street_traffic.put(car.streets.get(i), street_traffic.get(car.streets.get(i)) + 1);
			}
		}
		
		// For each intersection, assign to each street a green light time proportional to the traffic
		for (Car car : component.cars) {
			for (int i = 0; i < car.nrStreets - 1; ++i) {
				// Add the intersection if not added before
				if (!outputInfo.containsKey(car.streets.get(i).endIntersect))
					outputInfo.put(car.streets.get(i).endIntersect, new ArrayList<OutputPair>());
				
				// Add street with traffic * MULTI green time
				OutputPair op = new OutputPair(car.streets.get(i).name, (int)Math.round(street_traffic.get(car.streets.get(i)) * MULTI));
				
				if (!outputInfo.get(car.streets.get(i).endIntersect).contains(op))
					outputInfo.get(car.streets.get(i).endIntersect).add(op);
			}	
		}
	}
}