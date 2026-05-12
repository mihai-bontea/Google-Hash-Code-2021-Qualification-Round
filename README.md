# Google Hash Code 2021 Qualification Round(Traffic Signaling)

<p align="center">
  <img width="863" height="263" alt="Image" src="https://github.com/user-attachments/assets/5db47478-eea6-42a5-9aa7-4ebaa4a85411" />
</p>

>Given the description of a city plan and planned paths for all cars in that city, optimize the schedule of traffic lights to minimize the total amount of time spent in traffic, and help as many cars as possible reach their destination before a given deadline.

## Schedule Evaluator
Multiple classic optimization algorithms could be used to solve this problem, but the extent to which it would work depends on whether we can create a fast, lightweight module for evaluating a schedule.

Over simplified, the schedule evaluator involves pushing and popping a min-heap containing events (car reaches end of a street) and resolving how much each car waits at an intersection(depends on green light schedule and number of cars queued), when they reach the end, and how many seconds they have to spare.

### Notable speed improvements
* the problem statement and input/output files use strings to refer to streets. Internally, due to the constant need to fetch information about streets and model relationships between streets, cars, and intersections, a variable **street_id_to_name** is used to translate strings to a unique integer between {0, nr_streets}. This gets rid of the need for hashing operations on strings and greatly improves cache locality
* a very common operation is **get_next_green_light_time(street, current_time)**. By precomputing the green light intervals for each street(it's a cycle, therefore we  can use MOD) it becomes O(1) ~ 30-40% speed improvement
* **BucketQueue instead of MinHeap**(5-8x speed improvement): A standard priority_queue (binary heap) gives O(log n) per push and O(log n) per pop. For this simulation, n grows to roughly the number of in-flight cars, which can be hundreds of thousands. Every car movement is one pop and one push, so heap overhead dominates the inner loop. BucketQueue is a std::vector<T> buckets[MAX_TIME] plus a std::bitset<MAX_TIME> marking which buckets are non-empty. Push is a single buckets[time].push_back(item) plus setting one bit — O(1) with no comparisons. Finding the minimum non-empty bucket is &&**__builtin_ctzll** on the bitset — also effectively O(1), one CPU instruction per word scanned.

## Solution 1

A basic solution in which each street that has cars passing through receives 1 second of green light time, in the order from the input file.

| Input | Score |
|---|---|
| a_example | 1,001 |
| b_ocean | 4,566,576 |
| c_checkmate | 1,299,357 |
| d_daily_commute | 1,573,100 |
| e_etoile | 684,769 |
| f_forever_jammed | 819,083 |
| **Total** | **8,943,886** |

## Solution 2

A parallel **genetic algorithm** that evolves traffic light schedules. Built on top of an aggressively-optimized ScheduleEvaluator and parallelized with OpenMP across a 16-core CPU. Afterwards, on the best specimen produced, hill climbing is run to further improve the solution.

### Design choices

* **Genome**: Each individual is a complete Schedule. Both the order of streets at each intersection and their green durations are part of the genotype
* **Mutation**: per-intersection probabilistic, with weighted operators (50% swap, 30% adjust duration, 10% shuffle, 10% add/remove)
* **Selection**: tournament selection (size 3) with 10% elitism
* **Seeding**: 5% trivial copies (1 sec green light for all) + 20% mutated trivial + 75% random
* **Parallelism**: offspring generation(crossover + mutation) and offspring evaluation split using **OpenMP**

| Input | Solution 1 (Basic) | Solution 2 (GA + HC) | Improvement |
|---|---:|---:|---:|
| a_example | 1,001 | 2,002 | +100.0% |
| b_ocean | 4,566,576 | 4,569,135 | +0.06% |
| c_checkmate | 1,299,357 | 1,311,884 | +0.96% |
| d_daily_commute | 1,573,100 | 1,717,265 | +9.2% |
| e_etoile | 684,769 | 739,095 | +7.9% |
| f_forever_jammed | 819,083 | 1,230,556 | +50.2% |
| **Total** | **8,943,886** | **9,569,937** | **+7.0%** |

## Input file visualization

For each .in file, **visualize_input.py** parses the problem (intersections, streets, cars, paths) and computes per-instance metrics: street usage frequency, car slack (deadline minus minimum travel time), intersection contention scores (Σ usage/length over incoming streets), and the distribution of incoming streets per intersection.

<img width="2578" height="1728" alt="Image" src="https://github.com/user-attachments/assets/ffbd9718-0f36-40ba-9ffc-c48b67f48853" />