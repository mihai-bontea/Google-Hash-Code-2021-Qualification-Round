package sol1;

import java.util.Map;
import java.util.ArrayList;
import java.util.List;

public class Task implements Runnable{
	public final Data data;
	public final Component component;
	public final Map<Integer, List<OutputPair>> outputInfo;
	
	public Task(Data data, Component component, Map<Integer, List<OutputPair>> outputInfo) {
		this.data = data;
		this.component = component;
		this.outputInfo = outputInfo;
	}
	
	public void run() {
		for (Car car : component.cars) {
			// Going through all streets except the last one(since last intersection)
			for (int i = 0; i < car.nrStreets - 1; ++i) {
				// Add the intersection if not added before
				if (!outputInfo.containsKey(car.streets.get(i).endIntersect)) {
					outputInfo.put(car.streets.get(i).endIntersect, new ArrayList<OutputPair>());
				}
				// Add street with 1 second green light if not added before
				OutputPair op = new OutputPair(car.streets.get(i).name, 1);
				
				if (!outputInfo.get(car.streets.get(i).endIntersect).contains(op))
					outputInfo.get(car.streets.get(i).endIntersect).add(op);
			}	
		}
	}
}
