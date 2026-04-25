# Google Hash Code 2021 Qualification-Round(Traffic Signaling)

Multiple classic optimization algorithms could be used to solve this problem, but the extent to which it would work depends on whether we can create a fast, lightweight module for evaluating a schedule.

"state machine"
-Over simplified, it involves pushing and popping a min-heap containing events (car reaches end of a street) and resolving how much each car waits at an intersection(depends on green light schedule and number of cars queued), when they reach the end, and how many seconds they have to spare.

## Notable speed improvements
* the problem statement and input/output files use strings to refer to streets. Internally, due to the constant need to fetch information about streets and model relationships between streets, cars, and intersections, a variable **street_id_to_name** is used to translate strings to a unique integer between {0, nr_streets}. This gets rid of the need for hashing operations on strings and greatly improves cache locality
* a very common operation is get_next_green_light_time(street, current_time). By precomputing the green light intervals for each street(it's a cycle, therefore we  can use MOD) it becomes O(1) ~ 30-40% speed improvement
* BucketQueue instead of MinHeap

## Solution 1

| Input | Score |
|---|---|
| a_example | 1,001 |
| b_ocean | 4,566,576 |
| c_checkmate | 1,299,357 |
| d_daily_commute | 1,573,100 |
| e_etoile | 684,769 |
| f_forever_jammed | 819,083 |
| **Total** | **8,943,886** |