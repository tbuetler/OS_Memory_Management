# BTI1341 Lab -- Assignment 01: Memory Allocation Simulation

## Task Description

The goal of this project is to write a simulation of a memory management system
for **demand paging**. The system obtains requests to virtual addresses and has
to translate them into physical addresses using paging and a TLB as explained
during the course.

The system only simulates *a single process* and works as follows:

1. Memory accesses to multiple **virtual addresses** are simulated.
2. For every access, the virtual address has to be split into a **virtual page
   number (VPN)** and an **offset** within that page.
3. A **translation lookaside buffer (TLB)** has to be consulted, to check if the
   **physical frame number (PFN)** of that page has already been cached. On *TLB
   hit,* the **physical address** is computed using the PFN found and the
   computed offset.
4. On *TLB miss,* a **page table** has to be consulted, to check if the page is
   present in memory. If yes, the VPN and PFN are stored in the TLB and the
   **physical address** is computed as described above.
5. If the VPN is neither present in the TLB, nor in the page table, a new frame
   must be allocated for it. For this, a **free frame list** (physical frames!)
   has to be maintained. If it contains any free frames, one of them has to be
   allocated for the page and the **physical address** is again computed as
   above. *In addition, the page table, as well as the TLB, have to be properly
   updated.*
6. If there are no free frames left, translation is not possible. In this case,
   an error occurs.

## Implementation Guidelines

To complete this lab assignment, the files provided in this directory have to be
used as follows:

* ```mem_mgmt.h```: Defines the API of the memory management system. See
  comments in the file itself, as well as simulation flow below to understand
  how it works. **THIS FILE MUST NOT BE MODIFIED!**
* ```mem_mgmt.c```: Contains your implementation of the functions from
  ```mem_mgmt.h```. Of course, you may also create additional functions.
* ```simulator.c```: Example simulator which calls the functions from
  ```mem_mgmt.h```. Your implementation will be tested in a similar way.
* ```Makefile```: Used to build the simulator. **THIS FILE MUST NOT BE
  MODIFIED!**

### Simulation Flow

The simulation consists of three phases (see also *simulator.c*):

1. **Setup phase**: a call to ```setup()``` is performed, passing all relevant
   parameters of the simulation to your code.
2. **Simulation phase**:
   * A number of calls to ```translate()```, with varying virtual addresses is
     made. This function either returns the physical address if successful, or
     an error if a new free frame was required but not available (out of
     memory). Additionally, on success, the function indicates if a new frame
     had to be allocated, and if not, if the PFN was found in the TLB.
   * The simulator may call ```status()``` at any time, to obtain the number of
     free frames, allocated pages and valid TLB entries.
5. At the end of the simulation, the simulator calls ```teardown()```, which
   must free all allocated memory (of the simulation process itself - not
   simulated memory!).

### Data Structures To Be Implemented

Memory management for the simulator may be implemented in many different ways,
however we suggest that you use at least the three independent data structures
described in the following:

#### Free Frame List

The free frame list maintains a list of all **physical frames** not mapped to
any page. I.e. after setup (and before any call to ```translate()```), the
number of free frames is equal to the number of all physical frames in the
system.

For the implementation, different data structures might be considered, e.g.:

* Linked list (complexity: *O(1)*)
* Array (complexity: *O(m)* with *m* being the total number of frames)
* Bitmap (complexity same as for array, more space efficient)

#### Translation Lookaside Buffer (TLB)

Physically this part is normally realized by an *associative memory,* which can
be thought of as a hardware hash table. It is typically *very* small compared to
the page table.

*If all TLB entries are in use, your memory management system must use a
first-in, first-out (FIFO) replacement strategy!*

For the implementation, different data structures might be considered, e.g. a
linked list or a circular buffer.

#### Page Table

For this simulation, only a single-level page table is required. As the page
table is not sparse, it might simply be represented as an array (array element
index is VPN, element value is PFN). Clearly, other data structures are possible
as well.

## Submission

To submit your solution, commit your final version to the *master* branch of
your Git repository. Then, apply and push a *Git tag* marking your solution:

```
git tag -a lab-1 -m "tagging lab-1"
git push --all
git push --tags
```

## Evaluation

To obtain the full points in this lab, the following must be fulfilled:

* In order to be graded, submission must be done before the announced deadline
  and with the proper tag applied, as described above.
* Your code must compile *with the provided Makefile* and *without errors or
  warnings.*
* Tests will be run in similar fashion as in *simulator.c*, but with *randomized
  addresses.*
* Your code should not contain any memory leaks. It will be tested using
  ```valgrind```:

```
valgrind --tool=memcheck --leak-check=yes --show-reachable=yes -s ./simulator
```