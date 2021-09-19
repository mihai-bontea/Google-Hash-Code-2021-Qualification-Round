package sol1;

import java.io.FileNotFoundException;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadPoolExecutor;

public class Main {
	public static void main(String args[]) throws FileNotFoundException {
		String input_files[] = {"a.txt", "b.txt", "c.txt", "d.txt", "e.txt", "f.txt"};
		
		for(String input_file : input_files) {
			System.out.printf("Now working on %s.\n", input_file);
			Data data = new Data("../input_files/".concat(input_file));
			
			// Multiple components, use thread pool
			if (data.connectedComponents.size() > 1) {
				ThreadPoolExecutor executor = (ThreadPoolExecutor) Executors.newFixedThreadPool(6);
				for (Component component : data.connectedComponents)
					executor.execute(new Task(data, component));
			}
			else {
				Task task = new Task(data, data.connectedComponents.get(0));
				task.run();
			}
		}
	}
}
