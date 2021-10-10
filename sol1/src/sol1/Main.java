package sol1;

import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


public class Main {
	public static void main(String args[]) throws IOException {
		String input_files[] = {"a.txt", "b.txt", "c.txt", "d.txt", "e.txt", "f.txt"};
		
		for(String input_file : input_files) {
			System.out.printf("Now working on %s.\n", input_file);
			Data data = new Data("../input_files/".concat(input_file));
			
			Map<Integer, List<OutputPair>> outputInfo = new HashMap<Integer, List<OutputPair>>();
			
			for (Component component : data.connectedComponents) {
				Task task = new Task(data, component, outputInfo);
				task.run();
			}
			
			data.writeToFile("../output_files/sol1/".concat(input_file), outputInfo);
		}
	}
}
