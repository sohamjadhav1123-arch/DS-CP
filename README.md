# SmartER: Hospital Emergency Queue & Patient History Management System

> **A Data Structures and Algorithms Project in C++17**  
> Designed for College Project Viva Examination.

---

## 1. Project Overview

**SmartER** is an emergency department management system engineered to solve two fundamental problems in healthcare administration:
1. **Clinical Triage & Emergency Prioritization**: Patients arriving in an Emergency Room (ER) cannot be served on a simple first-come-first-served (FIFO) basis. High-acuity patients (e.g., cardiac arrest, acute respiratory failure) must supersede lower-acuity patients (e.g., minor sprains, rashes), regardless of arrival order.
2. **Patient History & Record Management across Multi-Hospital Networks**: A healthcare system often comprises multiple hospital branches. Each branch operates its own ER and local patient registry, requiring scoped identifier management and fast history retrieval.

---

## 2. Key Architecture & Scoping Rules

### Multi-Hospital Hierarchy
* **Independent Hospitals**: Each hospital (e.g., `Metro Trauma Center`, `Oakwood Community Hospital`) has an isolated patient registry and triage queue.
* **Per-Hospital ID Scoping**:
  * Patient IDs (e.g., `P101`) are unique **within** a single hospital.
  * The **same ID** (`P101`) may be registered at another hospital without conflict.
  * Globally, records can be disambiguated using composite keys: `HospitalID:PatientID` (e.g., `HOSP-001:P101`).

```
                    +---------------------------+
                    |       SmartER System      |
                    +-------------+-------------+
                                  |
            +---------------------+---------------------+
            |                                           |
+-----------v-----------+                   +-----------v-----------+
| Hospital: HOSP-001    |                   | Hospital: HOSP-002    |
| (Metro Trauma Center) |                   | (Oakwood Community)   |
+-----------+-----------+                   +-----------+-----------+
            |                                           |
   +--------+--------+                         +--------+--------+
   |                 |                         |                 |
+--v---------+ +-----v------+               +--v---------+ +-----v------+
| Patient    | | Patient    |               | Patient    | | Patient    |
| ID: P101   | | ID: P102   |               | ID: P101   | | ID: P103   |
| (John Doe) | | (Carlos)   |               | (Jane S.)  | | (Mark W.)  |
+------------+ +------------+               +------------+ +------------+
   (Same ID "P101" exists in different hospitals without collision)
```

---

## 3. Data Structures Roadmap (Viva Defense)

Per project guidelines, standard container classes (`std::priority_queue`, `std::unordered_map`, `std::map`, `std::list`) are **prohibited** for implementing the core structures. Instead, custom-coded data structures will be implemented in subsequent stages:

| Component | Planned Custom Data Structure | Time Complexity | Viva Rationale |
| :--- | :--- | :--- | :--- |
| **Emergency Triage Queue** | **Min-Heap (Priority Queue)** | Insert: `O(log N)`<br>Extract-Min: `O(log N)`<br>Peek: `O(1)` | Guarantees that the highest-urgency patient (Triage Level 1) is immediately dispatched next, breaking ties by arrival time. |
| **Patient Registry Lookup** | **Hash Table** (Separate Chaining / Open Addressing) | Lookup: `O(1)` avg<br>Insert: `O(1)` avg | Enables instant patient retrieval by `patientId` within any hospital branch. |
| **Patient Directory / Index** | **Binary Search Tree (BST) / AVL** | Search: `O(log N)`<br>In-Order Traversal: `O(N)` | Facilitates alphabetical ordering, prefix search, and range queries over patient records. |
| **Visit History** | **Linked List** (Singly / Doubly) | Append: `O(1)`<br>Traversal: `O(K)` | Dynamically records sequential medical visits for each patient without fixed-size vector limitations. |

---

## 4. Triage Classification (ESI Model)

SmartER adopts the clinical **Emergency Severity Index (ESI)** 5-level scale:

* **Level 1 - Immediate**: Imminent life-threat (cardiac arrest, severe respiratory distress).
* **Level 2 - Emergent**: High risk, severe altered mental state, acute chest pain.
* **Level 3 - Urgent**: Stable vitals, requiring two or more diagnostic resources.
* **Level 4 - Less Urgent**: Minor injury, single resource needed (e.g. X-ray, sutures).
* **Level 5 - Non-Urgent**: Low complexity, clinic consultation, routine refill.

> **Min-Heap Mapping**: Lower numerical triage level = Higher priority in the min-heap.

---

## 5. Project Directory Structure

```
SmartER/
├── CMakeLists.txt              # CMake build configuration (C++17)
├── README.md                   # Project documentation & viva architecture
├── include/
│   ├── EmergencyQueue.h        # Root forwarding header
│   ├── PatientRegistry.h       # Root forwarding header
│   ├── PatientBST.h            # Root forwarding header
│   ├── VisitHistory.h          # Root forwarding header
│   ├── SmartERSystem.h         # Root forwarding header
│   └── smarter/
│       ├── Hospital.hpp        # Hospital entity (owns its scoped emergency queue)
│       ├── Patient.hpp         # Patient record header (demographics & visits)
│       ├── Visit.hpp           # Medical visit header (complaints, status, notes)
│       ├── Triage.hpp          # ESI triage enumeration and helper utilities
│       ├── EmergencyQueue.h    # Custom Min-Heap priority queue header
│       ├── PatientRegistry.h   # Custom HashTable patient registry header
│       ├── PatientBST.h        # Custom Binary Search Tree patient index header
│       ├── VisitHistory.h      # Custom Singly Linked List visit history header
│       └── SmartERSystem.h     # System Coordinator header
├── src/
│   ├── Hospital.cpp            # Hospital registry & queue binding
│   ├── Patient.cpp             # Patient model and visit binding
│   ├── Visit.cpp               # Visit tracking and summary formatting
│   ├── EmergencyQueue.cpp      # Min-Heap implementation (vector-backed)
│   ├── PatientRegistry.cpp     # Separate Chaining HashTable implementation
│   ├── PatientBST.cpp          # Binary Search Tree implementation
│   ├── VisitHistory.cpp        # Singly Linked List visit history implementation
│   ├── SmartERSystem.cpp       # System Coordinator implementation
│   └── main.cpp                # Main demonstration application
└── tests/
    ├── test_emergency_queue.cpp   # 8-suite unit test battery (50 assertions)
    ├── test_patient_registry.cpp  # 16-scenario unit test battery (74 assertions)
    ├── test_patient_bst.cpp       # 17-scenario unit test battery (57 assertions)
    ├── test_visit_history.cpp     # 12-scenario unit test battery (58 assertions)
    └── test_system_integration.cpp# 6-scenario integration test battery (52 assertions)
```

---

## 6. Stage 2: Custom Min-Heap Emergency Queue

### Min-Heap Mechanics & Invariants
The emergency queue is backed by a 0-indexed `std::vector<QueueEntry>`. For any entry at index `i`:
* `Parent Index = (i - 1) / 2`
* `Left Child Index = 2 * i + 1`
* `Right Child Index = 2 * i + 2`

**Heap Invariant**: Every parent node has a higher or equal clinical priority than its children.

### Priority Ordering Rule
The priority key is a 2-tuple `(triageLevel, arrivalSequence)`:
1. **Primary Key**: `triageLevel` (1 = Immediate/Highest Priority, 5 = Non-Urgent/Lowest Priority). Lower numerical value has higher priority.
2. **Secondary Key (Tie-Breaker)**: `arrivalSequence` (Lower counter = Arrived earlier). Patients with identical triage levels are strictly dispatched First-Come, First-Served (FCFS).

---

## 7. Stage 3: Custom Separate Chaining HashTable (PatientRegistry)

### Collision Resolution via Separate Chaining
* The registry is backed by an array/vector of bucket heads (`HashNode*`).
* Each bucket is a singly linked list of `HashNode` structures.
* Colliding keys are inserted at the head of their corresponding bucket in $\mathcal{O}(1)$ time.

### Composite Key & Multi-Hospital Isolation
* Key: `(hospitalId, patientId)`.
* Combining both attributes prevents cross-hospital key collisions (e.g., patient `1001` at CityCare and patient `1001` at GreenLife coexist seamlessly).
* **Hash Function**: Polynomial rolling hash (Horner's rule, base $31$) combining `hospitalId`, delimiter `':'`, and `patientId`, modulo `bucketCount`.

### Dynamic Resizing & Memory Pointer Stability
* When $\text{Load Factor} = \frac{N}{\text{Buckets}} > 0.75$, the table expands to $2 \times \text{Buckets} + 1$ and rehashes all elements.
* **Viva Note on Pointer Stability**: Nodes are heap-allocated; during resizing, node pointers are simply relinked into new bucket chains without reallocating `Patient` objects. Pointers (`Patient*`) remain strictly valid across resizings.

### Complexity Analysis (Viva Reference)

| Operation | Average Case | Worst Case | Space Complexity | Viva Explanation |
| :--- | :--- | :--- | :--- | :--- |
| `insertPatient()` | $\mathcal{O}(1)$ | $\mathcal{O}(N)$ | $\mathcal{O}(1)$ | Checks for duplicate, inserts at bucket head. Triggers $\mathcal{O}(N)$ amortized resize. |
| `findPatient()` | $\mathcal{O}(1)$ | $\mathcal{O}(N)$ | $\mathcal{O}(1)$ | Hashes composite key, scans bucket chain. |
| `updatePatient()` | $\mathcal{O}(1)$ | $\mathcal{O}(N)$ | $\mathcal{O}(1)$ | Locates record, mutates demographic fields, preserves composite key. |
| `deletePatient()` | $\mathcal{O}(1)$ | $\mathcal{O}(N)$ | $\mathcal{O}(1)$ | Unlinks node from bucket chain (head, middle, or tail) and deletes memory. |
| `displayStatistics()` | $\mathcal{O}(\text{Buckets})$ | $\mathcal{O}(N + \text{Buckets})$ | $\mathcal{O}(1)$ | Computes empty buckets, collisions, and max chain length. |

---

## 8. Stage 4: Custom Binary Search Tree (PatientBST)

### In-Order Traversal & Sorted Ordering
* The BST indexes patients by composite key: `(hospitalId, patientId)`.
* **Comparison Rule**:
  1. Compare `hospitalId` alphabetically.
  2. If `hospitalId` is equal, compare `patientId` alphabetically.
  3. If both match, it is a duplicate key and rejected.
* **In-Order Traversal** (`Left -> Root -> Right`) yields strictly ascending order across the entire multi-hospital network.

### Data Consistency with Canonical PatientRegistry
* **Ownership Design**: BST nodes store the immutable composite key `(hospitalId, patientId)` and child pointers.
* **No Stale Duplication**: When displaying patient records, `PatientBST` queries the canonical `PatientRegistry`. Any updates made to patient demographics in the registry are immediately reflected in BST displays.

### BST Deletion Mechanics (3 Cases)
1. **Leaf Node (0 children)**: Node is deleted and parent's pointer set to `nullptr`.
2. **Node with 1 Child**: Node is deleted and replaced directly by its child.
3. **Node with 2 Children**: The in-order successor (minimum node in right subtree) is located, its keys copied to the target node, and the successor recursively removed from the right subtree.

### Complexity Analysis (Viva Reference)

| Operation | Average Case | Worst Case (Degenerate Tree) | Space Complexity | Viva Explanation |
| :--- | :--- | :--- | :--- | :--- |
| `searchPatient()` | $\mathcal{O}(\log N)$ | $\mathcal{O}(N)$ | $\mathcal{O}(1)$ | Branching decision cuts search space in half on balanced tree; skewed tree degrades to linear list. |
| `insertPatient()` | $\mathcal{O}(\log N)$ | $\mathcal{O}(N)$ | $\mathcal{O}(1)$ | Descends tree to leaf location and attaches new `BSTNode`. |
| `removePatient()` | $\mathcal{O}(\log N)$ | $\mathcal{O}(N)$ | $\mathcal{O}(1)$ | Locates node and handles 0, 1, or 2 children cases. |
| `displaySortedPatients()` | $\mathcal{O}(N)$ | $\mathcal{O}(N)$ | $\mathcal{O}(H)$ stack | Visits every node exactly once via recursion; stack bounded by height $H$. |
| `clear()` | $\mathcal{O}(N)$ | $\mathcal{O}(N)$ | $\mathcal{O}(H)$ stack | Post-order traversal deallocates left, right, then root. |

> **Viva Note on Tree Balance**: As an ordinary, unbalanced BST, degenerate insertion order (e.g., sorted inputs) causes worst-case height $H = N$. Self-balancing trees (AVL or Red-Black) guarantee $\mathcal{O}(\log N)$ worst-case.

---

## 9. Stage 5: Custom Singly Linked List (VisitHistory)

### Chronological Linked Chain
* Rather than fixed arrays or prohibited standard containers (`std::list`), patient visit histories are modeled as a manually managed **Singly Linked List** of `VisitNode` instances.
* **Head & Tail Pointers**:
  - `head_` points to the oldest recorded visit.
  - `tail_` points to the newest recorded visit, enabling $\mathcal{O}(1)$ append.
  - Chronological arrival order is naturally preserved from head to tail (FIFO order).

### Multi-Hospital Scoping and Data Isolation
* Every visit is bound to `(hospitalId, patientId, visitId)`.
* Rahul at Hospital 101 (`101, 1001`) and Neha at Hospital 102 (`102, 1001`) maintain completely isolated visit records without cross-facility history leakage.
* Duplicate visit IDs within the same patient's history are strictly rejected.

### Complexity Analysis (Viva Reference)

| Operation | Time Complexity | Auxiliary Space | Viva Explanation |
| :--- | :--- | :--- | :--- |
| `addVisit()` | $\mathcal{O}(1)$ append / $\mathcal{O}(K)$ duplicate scan | $\mathcal{O}(1)$ | Tail pointer enables direct constant-time append after linear duplicate check across patient's $K$ visits. |
| `findVisit()` | $\mathcal{O}(K)$ | $\mathcal{O}(1)$ | Sequential traversal of linked list nodes comparing `visitId`. |
| `updateVisitStatus()`| $\mathcal{O}(K)$ | $\mathcal{O}(1)$ | Locates visit node and updates `status` field in-place. |
| `removeVisit()` | $\mathcal{O}(K)$ | $\mathcal{O}(1)$ | Unlinks node (`prev->next = curr->next`), updates tail if needed, and reclaims memory. |
| `displayHistory()` | $\mathcal{O}(K)$ | $\mathcal{O}(1)$ | Sequentially traverses and formats summaries for all $K$ visits. |
| `isEmpty()` / `size()` | $\mathcal{O}(1)$ | $\mathcal{O}(1)$ | Directly queries internal counter and head pointer. |
| `clear()` | $\mathcal{O}(K)$ | $\mathcal{O}(1)$ | Traverses and frees all heap-allocated `VisitNode` instances. |

---

## 10. Stage 6: Unified System Integration (SmartERSystem)

### System Coordinator Architecture
`SmartERSystem` orchestrates the interactions between all four core custom data structures without maintaining redundant data stores:
1. **Canonical Patient Store (`PatientRegistry`)**: Stores full demographic profiles keyed by `(hospitalId, patientId)`.
2. **Sorted Patient Directory (`PatientBST`)**: Maintains balanced tree indices of composite keys and queries `PatientRegistry` for live details.
3. **Emergency Triage Queue (`EmergencyQueue`)**: Each registered hospital branch owns an independent Min-Heap priority queue.
4. **Chronological Audit Trail (`VisitHistory`)**: Central singly linked list logging all patient admissions and updating clinical status.

### Atomic Dual-Index Synchronization with Rollback
When registering a patient:
```
1. Validate Hospital Existence
2. Check Duplicate Composite Key (hospitalId, patientId)
3. Insert into Canonical PatientRegistry (Hash Table)
4. Insert into PatientBST (Binary Search Tree)
   ↳ IF BST insertion fails -> ROLLBACK: Delete from PatientRegistry
```

### State Consistency During Emergency Service
When a patient is served from the emergency room:
1. The highest-priority patient is popped from the Min-Heap.
2. The patient's active `Waiting` visit in `VisitHistory` is automatically updated to `InTreatment` (avoiding premature `Completed` status).

---

## 11. Stage 7: Interactive Console Application

The console interface in `src/main.cpp` provides an interactive 12-option menu:
```
========================================
SmartER HOSPITAL SYSTEM
=======================
1.  Register Hospital
2.  Register Patient
3.  View Emergency Queue
4.  Serve Next Patient
5.  Search Patient
6.  Update Triage Level
7.  Add Patient Visit
8.  View Patient Visit History
9.  Update Visit Status
10. Display Sorted Patient Records
11. View Hospital Statistics
12. Exit
========================================
```

### Input Safety & Resilience
* **Crash Prevention**: Uses robust stream-clearing and numeric range-checking functions (`readInt`, `readString`). Non-numeric inputs (e.g. entering `"abc"` for a menu choice) are caught gracefully without leaving the console in an infinite loop.
* **Separation of Concerns**: User interaction and input parsing are strictly confined to `main.cpp`, with all operations routed through the coordinator [`SmartERSystem`](file:///d:/BTECH/SY_SEM1/DS/DS%20CP/include/smarter/SmartERSystem.h#L52).

---

## 12. Build and Run Instructions

### Prerequisites
* Any standard **C++17** compiler (`g++` >= 7, `clang++` >= 5, or MSVC >= 2017)
* **CMake** (version 3.15 or newer)

### Compiling with CMake
```bash
# Create and navigate to build directory
mkdir build
cd build

# Generate build files
cmake -G "MinGW Makefiles" ..

# Build project
cmake --build .

# Run demonstration
./bin/smarter_app.exe

# Run all test suites
./bin/test_emergency_queue.exe
./bin/test_patient_registry.exe
./bin/test_patient_bst.exe
./bin/test_visit_history.exe
./bin/test_system_integration.exe
ctest --output-on-failure
```

### Direct Compilation (without CMake)
```bash
g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp -o smarter_app.exe
./smarter_app.exe
```

---

## 7. Data Privacy Notice

All names, symptoms, and identifiers used in demonstration and testing are **completely fictional** and used exclusively for academic demonstration.

