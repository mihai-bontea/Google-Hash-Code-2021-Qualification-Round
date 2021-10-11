input_files = ["a.txt", "b.txt", "c.txt", "d.txt", "e.txt", "f.txt"]

class Street:
    def __init__(self, info_str):
        info = info_str.strip().split(" ")
        self.intersect = (int(info[0]), int(info[1]))
        self.name = info[2]
        self.length = info[3]

  
class Car:
    def __init__(self, info_str, streets):
        info = info_str.strip().split(" ")
        street_names = info[1:]
        self.streets = []
        for street_name in street_names:
            self.streets.append(streets[street_name])


class Component:
    def __init__(self, car):
        self.cars = [car]
        self.intersect = set()
        for street in car.streets:
            self.intersect.add(street.intersect[1])

    def add_car(self, car):
        self.cars.append(car)
        for street in car.streets:
            self.intersect.add(street.intersect[1])
            

for input_file in input_files:
    print("For input file " + input_file + ":")
    with open("input_files/" + input_file, "r") as fin:
        first_line = fin.readline()
        info = first_line.strip().split(" ")

        # Read the general information
        total_time = int(info[0])
        nr_intersect = int(info[1])
        nr_streets = int(info[2])
        nr_cars = int(info[3])
        finish_bonus = int(info[4])

        # Reading the streets
        streets = {}
        for _ in range(nr_streets):
            info_str = fin.readline()
            street = Street(info_str)
            streets[street.name] = street

        # Reading the cars
        cars = []
        for _ in range(nr_cars):
            info_str = fin.readline()
            cars.append(Car(info_str, streets))
            
        # Counting the number of unused streets, if any
        used_streets = set()
        for car in cars:
            for street in car.streets:
                used_streets.add(street.name)
        print("\tWe have " + str(nr_streets - len(used_streets)) + " unused streets out of " + str(nr_streets))

        # Counting the number of independent intersections, if any
        # independent intersections = intersections which only lead to other intersections which are final
        # So, modifying an independent intersection has no side effects on the time cars arrive at other intersections
        pre_last_streets = set()
        for car in cars:
            if len(car.streets) >= 2:
                pre_last_streets.add(car.streets[-2])
        
        indep_intersect = set()
        for street in pre_last_streets:
            indep = True
            for car in cars:
                try:
                    idx = car.streets.index(street)
                    if len(car.streets) - idx > 2:
                        indep = False
                        break
                except ValueError as e:
                    pass
            if indep:
                indep_intersect.add(street.intersect[1])
        print("\tWe have " + str(len(indep_intersect)) + " independent intersections out of " + str(nr_intersect))

        # Counting the number of starting intersections, if any
        # starting intersections = intersections from which cars start but besides that, no other car has passage through them
        # (except as last intersection for which there is no queueing)
        # So, the time cars arrive at a starting intersection is not affected by other intersections' schedules
        starting_streets = set()
        for car in cars:
            starting_streets.add(car.streets[0])

        starting_intersect = set()
        for street in starting_streets:
            starting_only = True
            for car in cars:
                try:
                    idx = car.streets.index(street)
                    if idx != 0 and idx != len(car.streets) - 1:
                        starting_only = False
                        break
                except ValueError as e:
                    pass
            if starting_only:
                starting_intersect.add(street.intersect[1])
        print("\tWe have " + str(len(starting_intersect)) + " starting intersections out of " + str(nr_intersect))

        # Determining the number of connected components in the graph formed by the paths that the cars follow
        connected_components = []
        for car in cars:
            has_common_intersect = False
            for street in car.streets[:-1]:
                for component in connected_components:
                    if street.intersect[1] in component.intersect:
                        has_common_intersect = True
                        component.add_car(car)
                        break
                    
                if has_common_intersect == True:
                    break
            if has_common_intersect == False:
                connected_components.append(Component(car))

        print("\tWe have " + str(len(connected_components)) + " connected components")
        for component in connected_components:
              print(str(len(component.cars)), end=" ")
        print("\n")
    
