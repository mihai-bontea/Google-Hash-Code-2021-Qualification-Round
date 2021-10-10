class Street:
    def __init__(self, info_str):
        info = info_str.strip().split(" ")
        self.intersect = (int(info[0]), int(info[1]))
        self.name = info[2]
        self.length = int(info[3])

  
class Car:
    def __init__(self, info_str, streets):
        info = info_str.strip().split(" ")
        street_names = info[1:]
        self.streets = []
        for street_name in street_names:
            self.streets.append(streets[street_name])

        # Step at which the car currently is: index of street, distance driven from that street
        # If distance = street.length, then it is at the intersection
        self.step = (0, self.streets[0].length)

        # Every car starts out at an intersection
        self.at_intersect = True
        self.finished = False
        # If car just moved out of an intersection, we won't advance it again right away
        self.just_out = False

        self.time_finished = None

    def leave_intersect(self):
        if self.streets[self.step[0]].length == self.step[1]:
            self.just_out = True
            self.at_intersect = False
            self.step = (self.step[0] + 1, 0)
        else:
            raise Exception("This car should not leave intersection!")

    def advance(self, time, intersect_queues):
        self.just_out = False
        # Case 1: Reaching intersection(or finishing)
        if self.streets[self.step[0]].length == self.step[1] + 1:
            self.step = (self.step[0], self.step[1] + 1)
            # If car finished stop here
            if self.step[0] == len(self.streets) - 1:
                self.finished = True
                self.time_finished = time
            # Otherwise, add it to the intersection queue
            else:
                self.at_intersect = True
                intersect_queues[self.streets[self.step[0]].intersect[1]].add_to_queue(self.streets[self.step[0]].name, self)
        # Case 2: Making progress on the road
        else:
            self.step = (self.step[0], self.step[1] + 1)


class Intersection:
    def __init__(self, light_schedule):
        self.light_schedule = light_schedule
        # Computing the length of a cycle
        self.cycle_time = sum(map(lambda x: x[1], self.light_schedule))
        assert(self.cycle_time != 0)
        # Mapping each second of a cycle to a street
        self.street_at_second = {}
        current_time = 0
        for i in range(len(self.light_schedule)):
            for j in range(current_time, current_time + self.light_schedule[i][1]):
                self.street_at_second[j] = i
            current_time += self.light_schedule[i][1]
        
        # Creating a queue for each street that appears in the light schedule
        # street name : []
        self.street_queues = {}
        for street in map(lambda x : x[0], self.light_schedule):
            self.street_queues[street] = []

    def get_green_street_queue(self, time):
        second_from_cycle = time % self.cycle_time
        return self.street_queues[self.light_schedule[self.street_at_second[second_from_cycle]][0]]
    
    def let_first_pass(self, time):
        gsq = self.get_green_street_queue(time)
        if len(gsq) > 0:
            gsq[0].leave_intersect()
            gsq.pop(0)
    
    def add_to_queue(self, street_name, car):
        if street_name in self.street_queues.keys():
            self.street_queues[street_name].append(car)
        else:
            print("Street " + street_name + " receives no green light ever, car won't finish!")

solutions = ["sol1", "sol2"]
input_files = ["a.txt", "b.txt", "c.txt", "d.txt", "e.txt", "f.txt"]          
for solution in solutions:
    print("For " + solution + ":")
    score_for_solution = 0
    for input_file in input_files:
        print("\tFor input file " + input_file + ":")
        with open("input_files/" + input_file, "r") as fin:

            first_line = fin.readline()
            info = first_line.strip().split(" ")

            # Read the general information
            total_time = int(info[0])
            nr_intersect = int(info[1])
            nr_streets = int(info[2])
            nr_cars = int(info[3])
            finish_bonus = int(info[4])

            # Read the streets
            streets = {}
            for _ in range(nr_streets):
                info_str = fin.readline()
                street = Street(info_str)
                streets[street.name] = street

            # Read the cars
            cars = []
            for _ in range(nr_cars):
                info_str = fin.readline()
                cars.append(Car(info_str, streets))

            # Read the schedule for every intersection
            intersect_schedules = {}
            with open("output_files/" + solution + "/" + input_file) as fino:
                # Number of intersections for which we have a schedule
                first_line = fino.readline()
                nr_intersect_sched = int(first_line.strip())
                
                for i in range(nr_intersect_sched):
                    intersect = int(fino.readline().strip())
                    nr_street_sched = int(fino.readline().strip())
                    schedule = []
                    # Read light duration for each street in that intersection
                    for j in range(nr_street_sched):
                        street, duration = fino.readline().strip().split(" ")
                        duration = int(duration)
                        schedule.append((street, duration))
                                       
                    intersect_schedules[intersect] = Intersection(schedule)

            # Simulating the events to determine the score

            # Add all cars to the intersection queues
            for car in cars:
                intersect_schedules[car.streets[car.step[0]].intersect[1]].add_to_queue(car.streets[car.step[0]].name, car)
            
            for time in range(total_time):
                # Letting the first car from each intersection's green street pass
                for intersect in intersect_schedules.values():
                    intersect.let_first_pass(time)
                # Advance the rest of the cars
                for car in cars:
                    if not car.finished and not car.at_intersect and not car.just_out:
                        car.advance(time, intersect_schedules)
                    # Let it pass next time
                    if car.just_out:
                        car.just_out = False

            # Calculating the score
            score_for_file = 0
            cars_finished = 0
            for car in cars:
                if car.finished:
                    cars_finished += 1
                    assert(total_time >= car.time_finished)
                    score_for_file += finish_bonus
                    score_for_file += (total_time - car.time_finished)                   

            # Displaying score for file, and nr of cars that finished
            assert(score_for_file >= cars_finished * finish_bonus)
            print("\t\tScore = " + str(score_for_file))
            print("\t\tCars finished: " + str(cars_finished) + "/" + str(len(cars)))

            score_for_solution += score_for_file
    
    print("\tTotal score for " + solution + " = " + str(score_for_solution))