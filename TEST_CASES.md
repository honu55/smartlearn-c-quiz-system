# SmartLearn System - Test Cases

This document outlines the test cases used to verify the functionality, edge handling, and persistent file management of the SmartLearn Quiz and Practice System.

---

### Test Case 1: First-Time Initialization and Login
* **Objective:** Verify default question bank generation and new user registration.
* **Pre-conditions:** Delete any existing `.txt` files (`questions.txt`, `students.txt`, etc.) from the working directory.
* **Input Steps:**
  1. Launch the executable (`./smartlearn`).
  2. When prompted for Student ID, enter: `  STU001  ` (with surrounding spaces).
  3. When prompted for Student Name, enter: `John Doe`.
* **Expected Result:**
  - System reports creating `questions.txt` with sample default questions.
  - Spaces in `  STU001  ` are trimmed to save ID as `STU001`.
  - Profile is saved to `students.txt`.
  - Main menu displays welcome message for "John Doe".

---

### Test Case 2: Adding a Custom Question
* **Objective:** Ensure user-added questions persist without corrupting the file structure.
* **Pre-conditions:** User `STU001` is logged in at Main Menu.
* **Input Steps:**
  1. Select option `4` (Add New Question).
  2. Enter Subject: `Computer Science`
  3. Enter Topic: `Pointers`
  4. Enter Difficulty: `2`
  5. Enter Question Text: `What is a dereference operator in C?`
  6. Enter Option A: `&`
  7. Enter Option B: `*`
  8. Enter Option C: `->`
  9. Enter Option D: `.`
  10. Enter Correct Option: `2`
* **Expected Result:**
  - Screen prints: `Question added successfully. Total questions: 25`.
  - `questions.txt` is updated cleanly with 9 distinct, sequential lines for the new question.

---

### Test Case 3: Practice Mode Navigation and Input Validation
* **Objective:** Test input validation, immediate feedback, and early exit in Practice Mode.
* **Pre-conditions:** Questions are loaded; user is at Main Menu.
* **Input Steps:**
  1. Select option `1` (Practice Mode).
  2. Select Subject: `Math`
  3. Select Topic: `Algebra`
  4. Select Difficulty: `1` (Easy)
  5. On Question 1, enter invalid answer: `Z`
  6. On re-prompt for Question 1, enter valid option: `c` (lowercase)
  7. On Question 2, enter: `Q` (Quit)
* **Expected Result:**
  - Typing `Z` triggers an `Invalid input` warning without breaking execution.
  - Lowercase `c` is automatically converted to `C`, accepted, and displays immediate feedback (`Correct!`).
  - Typing `Q` terminates practice mode safely and prints correct answer counts.

---

### Test Case 4: Graded Quiz and Performance Logging
* **Objective:** Verify scoring calculations, grade remark output, and result persistence across files.
* **Pre-conditions:** User is at Main Menu.
* **Input Steps:**
  1. Select option `2` (Quiz Mode).
  2. Select Subject: `Computer Science`, Topic: `C Basics`, Difficulty: `Mixed`.
  3. Enter desired number of questions: `2`.
  4. Answer both questions correctly (`A`-`D`).
* **Expected Result:**
  - Quiz complete screen outputs `Score: 2 / 2`, `Percentage: 100.00%`, and Remark `Excellent work!`.
  - Appends 9 lines of quiz summary to `results.txt`.
  - Appends 5 lines of topic data to `topicstats.txt`.

---

### Test Case 5: Performance Analysis Categorization
* **Objective:** Ensure performance analysis correctly reads past data and flags strong vs. weak topics.
* **Pre-conditions:** User `STU001` has completed at least one quiz.
* **Input Steps:**
  1. Select option `3` (View Performance Analysis).
* **Expected Result:**
  - System reads `topicstats.txt` and aggregates all attempts/correct marks for `STU001`.
  - Formats output table cleanly displaying Subject, Topic, Attempted, Correct, and Accuracy.
  - Displays `[STRONG]` flag beside topics with accuracy $\ge 70\%$.
  - Displays `[WEAK]` flag beside topics with accuracy $< 50\%$.
