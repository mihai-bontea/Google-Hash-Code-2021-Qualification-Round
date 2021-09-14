input_files = ["a.txt", "b.txt", "c.txt", "d.txt.", "e.txt", "f.txt"]

class Car:
    def __init__(self, info_str):
        info = info_str.strip().split(" ")
        self.streets = info[1:]

    def min_finish_time(self, street_len):
        time = 0
        for street in self.streets[1:]:
            time += street_len[street]
        return time


class Component:
    def __init__(self, car):
        self.cars = [car]
        self.streets = set(car.streets)

    def add_car(self, car):
        self.cars.append(car)
        for street in car.streets:
            self.streets.add(street)
            
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

        # Remember the length of each street
        street_len = {}
        for _ in range(nr_streets):
            info = fin.readline().strip().split(" ")
            street_len[info[2]] = int(info[3])

        cars = []
        invalid_cars = 0
        no_wait = 0
        # Reading the path each car follows, counting cars that can't finish, if any
        for _ in range(nr_cars):
            info_str = fin.readline()
            new_car = Car(info_str)
            if new_car.min_finish_time(street_len) < total_time:
                cars.append(new_car)
            elif new_car.min_finish_time(street_len) == total_time:
                cars.append(new_car)
                no_wait += 1
            else:
                invalid_cars += 1

        print("\tWe have " + str(invalid_cars) + " invalid cars and " + str(no_wait) + " cars that need perfect time") 

        # Counting the number of unused streets, if any
        streets = set()
        for car in cars:
            for street in car.streets:
                streets.add(street)
        print("\tWe have " + str(nr_streets - len(streets)) + " unused streets out of " + str(nr_streets))

        # Determining the number of connected components in the graph formed by the paths that the cars follow
        connected_components = []
        for car in cars:
            any_street_appears = False
            for street in car.streets:
                street_appears = False
                for component in connected_components:
                    if street in component.streets:
                        street_appears = True
                        component.add_car(car)
                        break
                    
                if street_appears == True:
                    any_street_appears = True
                    break
            if any_street_appears == False:
                connected_components.append(Component(car))

        print("\tWe have " + str(len(connected_components)) + " connected components")
        for component in connected_components:
              print(str(len(component.cars)), end=" ")
        print("\n")
        
    
                        
        

        
        
        
