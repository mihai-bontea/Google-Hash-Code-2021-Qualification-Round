package sol1;

public class Street {
	public final int startIntersect;
	public final int endIntersect;
	public final String name;
	public final int length;
	
	public Street(int startIntersect, int endIntersect, String name, int length) {
		this.startIntersect = startIntersect;
		this.endIntersect = endIntersect;
		this.name = name;
		this.length = length;
	}
}
