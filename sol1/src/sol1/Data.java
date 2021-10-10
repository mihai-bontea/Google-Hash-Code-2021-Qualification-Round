package sol1;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Scanner;
import java.util.Set;

public class Data {
	public final int totalTime;
	public final int nrIntersections;
	public final int nrStreets;
	public final int nrCars;
	public final int finishBonus;
	public final Map<String, Street> streets;
	public final Set<String> usedStreets;
	public final List<Car> cars;
	public final List<Component> connectedComponents;
	
	
	public Data(String filename) throws FileNotFoundException{
		File inFile = new File(filename);
		Scanner scanner = new Scanner(inFile);
		
		// Reading the general information
		this.totalTime = scanner.nextInt();
		this.nrIntersections = scanner.nextInt();
		this.nrStreets = scanner.nextInt();
		this.nrCars = scanner.nextInt();
		this.finishBonus = scanner.nextInt();
		
		// Reading the streets
		streets = new HashMap<String, Street>();
		for (int i = 0; i < this.nrStreets; ++i) {
			int startIntersect = scanner.nextInt();
			int endIntersect = scanner.nextInt();
			String name = scanner.next();
			int length = scanner.nextInt();
			
			streets.put(name, new Street(startIntersect, endIntersect, name, length));
			
		}
		
		// Reading the cars and keeping track of the streets that are actually used
		usedStreets = new HashSet<String>();
		cars = new ArrayList<Car>();
		for (int i = 0; i < this.nrCars; ++i) {
			int nrStreets = scanner.nextInt();
			List<Street> path = new ArrayList<Street>();
			
			for (int j = 0; j < nrStreets; ++j) {
				String aux = scanner.next();
				usedStreets.add(aux);
				path.add(streets.get(aux));
			}
			cars.add(new Car(nrStreets, path));
		}
		scanner.close();
		//System.out.printf("We have %d used streets out of %d total streets\n", used_streets.size(), streets.size());
		
		// Breaking the paths that the cars follow into connected components
		connectedComponents = new ArrayList<Component>();
		for (Car car : cars) {
			boolean hasCommonIntersect = false;
			
			for (Street street : car.streets) {
				for (Component component : connectedComponents) {
					if (component.containsIntersect(street.endIntersect)) {
						hasCommonIntersect = true;
						component.addCar(car);
						break;
					}	
				}
				if (hasCommonIntersect == true)
					break;
			}
			if (hasCommonIntersect == false)
				connectedComponents.add(new Component(car));
		}
		
		System.out.printf("We have %d connected components.\n", connectedComponents.size());
	}
	
	public void writeToFile(String filename, Map<Integer, List<OutputPair>> outputInfo) throws IOException {
		File outFile = new File(filename);
		FileWriter fw = new FileWriter(outFile);
		
		fw.write(String.format("%d\n", outputInfo.size()));
		for (int key : outputInfo.keySet()) {
			fw.write(String.format("%d\n%d\n", key, outputInfo.get(key).size()));
			
			for (OutputPair op : outputInfo.get(key)) {
				fw.write(String.format("%s\n", op.toString()));
			}
		}
		fw.close();
	}
}
