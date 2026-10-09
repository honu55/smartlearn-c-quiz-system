/*
 ==========================================================================
   SMARTLEARN  -  Quiz and Practice System in C   (Beginner Version)
 ==========================================================================

   WHAT THIS PROGRAM DOES
   1. Student logs in with an ID (new students are registered).
   2. Student can PRACTICE questions (answer shown immediately).
   3. Student can take a QUIZ (score and percentage shown at the end).
   4. Questions are chosen by subject, topic and difficulty (Easy/Medium/Hard).
   5. Question order is RANDOM every time.
   6. The program shows which topics are STRONG and which are WEAK.
   7. Everything is saved in text files (file handling).

   C TOPICS USED IN THIS PROJECT
   - Variables and data types ..... int, double, char
   - Conditional statements ....... if / else if / else, switch
   - Loops ........................ for, while, do-while style menus
   - Arrays ....................... arrays of numbers, arrays of structures
   - Strings ...................... char arrays, strcpy, strcmp, strlen
   - Structures ................... Question, Student, TopicStat
   - Functions .................... many small functions, each does one job
   - Randomization ................ rand(), srand(), time()
   - File handling ................ fopen, fclose, fprintf, fgets

   HOW TO COMPILE AND RUN
        gcc smartlearn_beginner.c -o smartlearn
        ./smartlearn            (on Windows:  smartlearn.exe)

   FILES THAT THE PROGRAM CREATES (in the same folder)
        questions.txt   -> all questions        (9 lines for each question)
        students.txt    -> student profiles     (2 lines for each student)
        results.txt     -> quiz results         (9 lines for each quiz)
        topicstats.txt  -> topic-wise marks     (5 lines per topic per quiz)
   Every value is stored on its OWN LINE. This makes reading very easy.
 ==========================================================================
*/

#include <stdio.h>    /* printf, fgets, fopen, fprintf ...            */
#include <stdlib.h>   /* rand, srand, atoi, atof, exit                */
#include <string.h>   /* strcpy, strcmp, strlen, strncpy              */
#include <time.h>     /* time (for random seed and for the date)      */
#include <ctype.h>    /* toupper (turn 'a' into 'A')                  */


/* ==========================================================================
   PART 1 : CONSTANTS
   #define gives a name to a fixed value. If we want to change a size later,
   we change it only in ONE place.
   ========================================================================== */

#define MAX_Q       200     /* maximum number of questions in the bank      */
#define MAX_TEXT    256     /* maximum length of a question text            */
#define MAX_OPT     100     /* maximum length of one option                 */
#define MAX_NAME    50      /* maximum length of a student name             */
#define MAX_ID      20      /* maximum length of a student ID               */
#define MAX_LABEL   40      /* maximum length of a subject / topic name     */
#define MAX_TOPICS  30      /* maximum number of different topics           */
#define MAX_STATS   100     /* maximum records in performance analysis      */

#define Q_FILE "questions.txt"
#define S_FILE "students.txt"
#define R_FILE "results.txt"
#define T_FILE "topicstats.txt"

#define STRONG_LIMIT 70.0   /* percentage >= 70  -> STRONG topic            */
#define WEAK_LIMIT   50.0   /* percentage <  50  -> WEAK topic              */


/* ==========================================================================
   PART 2 : STRUCTURES
   A structure groups different pieces of data under one name.
   ========================================================================== */

/* One multiple-choice question */
struct Question
{
    char subject[MAX_LABEL];     /* example: Math                           */
    char topic[MAX_LABEL];       /* example: Algebra                        */
    int  difficulty;             /* 1 = Easy, 2 = Medium, 3 = Hard          */
    char text[MAX_TEXT];         /* the question itself                     */
    char option[4][MAX_OPT];     /* four options: option[0] is A, [1] is B..*/
    int  correct;                /* correct option number: 1, 2, 3 or 4     */
};

/* One student */
struct Student
{
    char id[MAX_ID];
    char name[MAX_NAME];
};

/* Total marks of ONE topic (used in performance analysis) */
struct TopicStat
{
    char subject[MAX_LABEL];
    char topic[MAX_LABEL];
    int  attempted;              /* how many questions were asked           */
    int  correct;                /* how many were answered correctly        */
};


/* ==========================================================================
   PART 3 : GLOBAL VARIABLES
   Global variables are declared outside all functions, so EVERY function
   can use them. We keep the main shared data here to make the code simple.
   ========================================================================== */

struct Question questions[MAX_Q];   /* the question bank (array of structures) */
int qCount = 0;                     /* how many questions are in the bank      */

struct Student current;             /* the student who is logged in now        */

/* The choices of the student for practice / quiz */
char selSubject[MAX_LABEL];         /* chosen subject                          */
char selTopic[MAX_LABEL];           /* chosen topic ("" means all topics)      */
int  selDifficulty;                 /* 1, 2, 3  or 0 for mixed                 */
int  pool[MAX_Q];                   /* numbers of the matching questions       */
int  poolCount = 0;                 /* how many questions matched              */


/* ==========================================================================
   PART 4 : SMALL HELPER FUNCTIONS
   ========================================================================== */

/* Draws a line on the screen (only for decoration). */
void drawLine()
{
    printf("------------------------------------------------------------\n");
}

/*
   removeNewline
   When we read a line with fgets, the Enter key ('\n') is also stored at
   the end of the text. This function deletes it.
   (On Windows files, a '\r' may also be there, so we remove that too.)
*/
void removeNewline(char text[])
{
    int len = strlen(text);                 /* length of the text */
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r'))
    {
        text[len - 1] = '\0';               /* '\0' marks the end of a string */
        len--;
    }
}

/*
   readText
   Shows a message (prompt) and reads one line typed by the user.
   size = the maximum number of characters that the text variable can hold.
*/
void readText(char prompt[], char text[], int size)
{
    char temp[300];

    printf("%s", prompt);

    /* fgets reads a full line, including spaces. It returns NULL if there
       is no more input (for example, when the input is closed). */
    if (fgets(temp, 300, stdin) == NULL)
    {
        printf("\nInput ended. Goodbye!\n");
        exit(0);
    }

    removeNewline(temp);

    /* copy at most (size-1) characters, so that we never overflow the array */
    strncpy(text, temp, size - 1);
    text[size - 1] = '\0';
}

/*
   readNumber
   Keeps asking until the user types a whole number between low and high.
   This protects the program from wrong input like letters.
*/
int readNumber(char prompt[], int low, int high)
{
    char input[50];
    int number;

    while (1)                               /* repeat until we return */
    {
        readText(prompt, input, 50);
        number = atoi(input);               /* text -> integer ("abc" gives 0) */

        if (number >= low && number <= high)
        {
            return number;                  /* valid number: give it back */
        }
        printf("  Please enter a number between %d and %d.\n", low, high);
    }
}

/*
   shuffle   (RANDOMIZATION)
   Mixes the numbers in the list into a random order.
   Idea: go from the last position to the first. For each position i, pick a
   random position j (from 0 to i) and swap the two values.
*/
void shuffle(int list[], int n)
{
    int i, j, temp;

    for (i = n - 1; i > 0; i--)
    {
        j = rand() % (i + 1);               /* random number from 0 to i */

        temp    = list[i];                  /* swap list[i] and list[j]  */
        list[i] = list[j];
        list[j] = temp;
    }
}

/* Converts the difficulty number into a word: 1 -> "Easy" and so on. */
void getDifficultyName(int d, char name[])
{
    if (d == 1)
        strcpy(name, "Easy");
    else if (d == 2)
        strcpy(name, "Medium");
    else if (d == 3)
        strcpy(name, "Hard");
    else
        strcpy(name, "Mixed");
}

/* Puts today's date and time into text, for example "2026-10-09 14:30". */
void getCurrentDate(char text[])
{
    time_t now = time(NULL);                /* seconds since 1970      */
    struct tm *t = localtime(&now);         /* break into year, month..*/
    strftime(text, 30, "%Y-%m-%d %H:%M", t);
}


/* ==========================================================================
   PART 5 : FILE HELPER FUNCTIONS
   Opening a file:   FILE *f = fopen("name.txt", mode);
        mode "r" = read   |  "w" = write (erases old data)  |  "a" = append
   After use, ALWAYS close it:  fclose(f);
   ========================================================================== */

/*
   readFileLine
   Reads ONE line from a file into text (without the Enter at the end).
   Returns 1 if a line was read, and 0 if the file has ended.
*/
int readFileLine(FILE *f, char text[], int size)
{
    if (fgets(text, size, f) == NULL)
    {
        text[0] = '\0';                     /* make the text empty */
        return 0;
    }
    removeNewline(text);
    return 1;
}


/* ==========================================================================
   PART 6 : QUESTION BANK  (create, save, load, add)
   ========================================================================== */

/* Writes ONE question to an open file. Each value goes on its own line. */
void saveOneQuestion(FILE *f, struct Question q)
{
    int i;

    fprintf(f, "%s\n", q.subject);
    fprintf(f, "%s\n", q.topic);
    fprintf(f, "%d\n", q.difficulty);
    fprintf(f, "%s\n", q.text);
    for (i = 0; i < 4; i++)
    {
        fprintf(f, "%s\n", q.option[i]);
    }
    fprintf(f, "%d\n", q.correct);
}

/* Writes ALL questions to questions.txt (old content is replaced). */
void saveAllQuestions()
{
    int i;
    FILE *f = fopen(Q_FILE, "w");

    if (f == NULL)                          /* NULL means the file could not open */
    {
        printf("Error: cannot write %s\n", Q_FILE);
        return;
    }

    for (i = 0; i < qCount; i++)
    {
        saveOneQuestion(f, questions[i]);
    }
    fclose(f);
}

/* Adds one built-in question into the array. Used only for sample data. */
void addDefault(char subject[], char topic[], int difficulty, char text[],
                char a[], char b[], char c[], char d[], int correct)
{
    strcpy(questions[qCount].subject, subject);
    strcpy(questions[qCount].topic, topic);
    questions[qCount].difficulty = difficulty;
    strcpy(questions[qCount].text, text);
    strcpy(questions[qCount].option[0], a);
    strcpy(questions[qCount].option[1], b);
    strcpy(questions[qCount].option[2], c);
    strcpy(questions[qCount].option[3], d);
    questions[qCount].correct = correct;
    qCount++;                               /* one more question in the bank */
}

/* Creates 24 sample questions (used when questions.txt does not exist). */
void loadDefaultQuestions()
{
    qCount = 0;

    /* ----- Math : Algebra ----- */
    addDefault("Math", "Algebra", 1, "Solve: 2x + 6 = 14. What is x?",
               "2", "3", "4", "5", 3);
    addDefault("Math", "Algebra", 1, "What is the value of 3^3?",
               "6", "9", "27", "81", 3);
    addDefault("Math", "Algebra", 2, "The roots of x^2 - 5x + 6 = 0 are:",
               "1 and 6", "2 and 3", "-2 and -3", "-1 and 6", 2);
    addDefault("Math", "Algebra", 2, "Simplify: (a+b)^2 - (a-b)^2",
               "2ab", "4ab", "a^2 + b^2", "0", 2);
    addDefault("Math", "Algebra", 3, "If x + 1/x = 3, then x^2 + 1/x^2 equals:",
               "5", "7", "9", "11", 2);
    addDefault("Math", "Algebra", 3, "The sum of the roots of 2x^2 - 7x + 3 = 0 is:",
               "7/2", "-7/2", "3/2", "-3/2", 1);

    /* ----- Math : Geometry ----- */
    addDefault("Math", "Geometry", 1, "The sum of the interior angles of a triangle is:",
               "90 degrees", "180 degrees", "270 degrees", "360 degrees", 2);
    addDefault("Math", "Geometry", 1, "Area of a rectangle with length 8 and width 5:",
               "13", "26", "40", "80", 3);
    addDefault("Math", "Geometry", 2, "Hypotenuse of a right triangle with legs 6 and 8:",
               "10", "12", "14", "15", 1);
    addDefault("Math", "Geometry", 2, "Area of a circle with radius 7 (use pi = 22/7):",
               "44", "154", "308", "49", 2);
    addDefault("Math", "Geometry", 3, "The sum of the interior angles of a hexagon is:",
               "540 degrees", "720 degrees", "900 degrees", "1080 degrees", 2);
    addDefault("Math", "Geometry", 3, "Number of diagonals in a polygon with 8 sides:",
               "16", "20", "24", "28", 2);

    /* ----- Computer Science : C Basics ----- */
    addDefault("Computer Science", "C Basics", 1, "Which header file is needed for printf()?",
               "stdlib.h", "stdio.h", "string.h", "math.h", 2);
    addDefault("Computer Science", "C Basics", 1, "Which symbol ends a statement in C?",
               ":", ".", ";", ",", 3);
    addDefault("Computer Science", "C Basics", 2, "sizeof(int) on most modern systems is:",
               "1 byte", "2 bytes", "4 bytes", "8 bytes", 3);
    addDefault("Computer Science", "C Basics", 2, "Which operator gives the remainder of a division?",
               "/", "%", "*", "&", 2);
    addDefault("Computer Science", "C Basics", 3, "For 'int a[5];', accessing a[5] is:",
               "Valid (last element)", "Undefined behavior", "Always returns 0",
               "Always a compile error", 2);
    addDefault("Computer Science", "C Basics", 3, "What is the value of 7 & 3 in C?",
               "3", "7", "4", "10", 1);

    /* ----- Computer Science : Data Structures ----- */
    addDefault("Computer Science", "Data Structures", 1, "Which data structure follows LIFO?",
               "Queue", "Stack", "Array", "Tree", 2);
    addDefault("Computer Science", "Data Structures", 1, "Which data structure follows FIFO?",
               "Stack", "Queue", "Heap", "Graph", 2);
    addDefault("Computer Science", "Data Structures", 2, "Time complexity of binary search:",
               "O(n)", "O(log n)", "O(n^2)", "O(1)", 2);
    addDefault("Computer Science", "Data Structures", 2, "Which structure uses nodes linked by pointers?",
               "Array", "Linked list", "Matrix", "String", 2);
    addDefault("Computer Science", "Data Structures", 3, "Worst-case time complexity of Bubble Sort:",
               "O(n)", "O(n log n)", "O(n^2)", "O(log n)", 3);
    addDefault("Computer Science", "Data Structures", 3, "Inorder traversal of a BST visits keys in:",
               "Random order", "Sorted order", "Reverse level order", "Preorder", 2);

    saveAllQuestions();                     /* write them to questions.txt */
}

/*
   loadQuestions
   Reads the question bank from questions.txt into the questions[] array.
   If the file does not exist (first run), the sample questions are created.
*/
void loadQuestions()
{
    FILE *f = fopen(Q_FILE, "r");
    char line[300];
    struct Question q;
    int i;

    qCount = 0;

    if (f != NULL)
    {
        while (qCount < MAX_Q)
        {
            /* Read the 9 lines of one question, in the same order as saved.
               If the first line cannot be read, the file has ended. */
            if (readFileLine(f, q.subject, MAX_LABEL) == 0)
            {
                break;
            }
            readFileLine(f, q.topic, MAX_LABEL);

            readFileLine(f, line, 300);
            q.difficulty = atoi(line);              /* text -> number */

            readFileLine(f, q.text, MAX_TEXT);
            for (i = 0; i < 4; i++)
            {
                readFileLine(f, q.option[i], MAX_OPT);
            }

            readFileLine(f, line, 300);
            q.correct = atoi(line);

            /* keep the question only if the numbers are valid */
            if (q.difficulty >= 1 && q.difficulty <= 3 &&
                q.correct >= 1 && q.correct <= 4)
            {
                questions[qCount] = q;              /* copy whole structure */
                qCount++;
            }
        }
        fclose(f);
    }

    if (qCount == 0)                        /* nothing loaded: first run */
    {
        loadDefaultQuestions();
        printf("Question bank created with %d sample questions.\n", qCount);
    }
}

/* Menu option 5: the user types a new question and it is saved. */
void addQuestion()
{
    struct Question q;
    char prompt[30];
    int i;

    if (qCount >= MAX_Q)
    {
        printf("Question bank is full.\n");
        return;
    }

    printf("\n=== Add a New Question ===\n");

    readText("Subject: ", q.subject, MAX_LABEL);
    readText("Topic: ", q.topic, MAX_LABEL);

    if (strlen(q.subject) == 0 || strlen(q.topic) == 0)
    {
        printf("Subject and topic cannot be empty.\n");
        return;
    }

    q.difficulty = readNumber("Difficulty (1=Easy, 2=Medium, 3=Hard): ", 1, 3);

    readText("Question text: ", q.text, MAX_TEXT);
    if (strlen(q.text) == 0)
    {
        printf("Question cannot be empty.\n");
        return;
    }

    for (i = 0; i < 4; i++)
    {
        sprintf(prompt, "Option %c: ", 'A' + i);    /* 'A'+0 = A, 'A'+1 = B ... */
        readText(prompt, q.option[i], MAX_OPT);
    }

    q.correct = readNumber("Correct option (1=A, 2=B, 3=C, 4=D): ", 1, 4);

    questions[qCount] = q;                  /* add to the array          */
    qCount++;

    /* Append ("a" mode) the new question at the end of the file */
    FILE *f = fopen(Q_FILE, "a");
    if (f != NULL)
    {
        saveOneQuestion(f, q);
        fclose(f);
    }
    printf("Question added successfully. Total questions: %d\n", qCount);
}


/* ==========================================================================
   PART 7 : STUDENT PROFILE  (login / register)
   ========================================================================== */

/*
   findStudent
   Searches students.txt for the given ID.
   Returns 1 if found (and fills the global variable "current"), else 0.
*/
int findStudent(char id[])
{
    char fileId[MAX_ID];
    char fileName[MAX_NAME];
    FILE *f = fopen(S_FILE, "r");

    if (f == NULL)                          /* file does not exist yet */
    {
        return 0;
    }

    /* each student uses 2 lines: first the ID, then the name */
    while (readFileLine(f, fileId, MAX_ID) && readFileLine(f, fileName, MAX_NAME))
    {
        if (strcmp(fileId, id) == 0)        /* strcmp gives 0 if texts are equal */
        {
            strcpy(current.id, fileId);
            strcpy(current.name, fileName);
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}

/* Asks for an ID. Old student -> welcome back. New student -> register. */
void loginOrRegister()
{
    char id[MAX_ID];
    FILE *f;

    printf("\n=== Student Login ===\n");

    while (1)
    {
        readText("Enter your Student ID: ", id, MAX_ID);
        if (strlen(id) > 0)
        {
            break;                          /* leave the loop */
        }
        printf("  ID cannot be empty.\n");
    }

    if (findStudent(id) == 1)
    {
        printf("Welcome back, %s!\n", current.name);
        return;
    }

    /* the ID was not found, so this is a new student */
    printf("New student. Let's create your profile.\n");

    while (1)
    {
        readText("Enter your name: ", current.name, MAX_NAME);
        if (strlen(current.name) > 0)
        {
            break;
        }
        printf("  Name cannot be empty.\n");
    }
    strcpy(current.id, id);

    f = fopen(S_FILE, "a");
    if (f != NULL)
    {
        fprintf(f, "%s\n", current.id);
        fprintf(f, "%s\n", current.name);
        fclose(f);
    }
    printf("Profile created. Welcome, %s!\n", current.name);
}


/* ==========================================================================
   PART 8 : SUBJECT / TOPIC / DIFFICULTY SELECTION
   ========================================================================== */

/*
   collectSubjects
   Finds all DIFFERENT subject names in the question bank.
   "list" is an array of strings (a 2D char array). Returns how many found.
*/
int collectSubjects(char list[][MAX_LABEL])
{
    int n = 0;
    int i, j, found;

    for (i = 0; i < qCount && n < MAX_TOPICS; i++)
    {
        found = 0;
        for (j = 0; j < n; j++)             /* is this subject already listed? */
        {
            if (strcmp(list[j], questions[i].subject) == 0)
            {
                found = 1;
            }
        }
        if (found == 0)
        {
            strcpy(list[n], questions[i].subject);
            n++;
        }
    }
    return n;
}

/* Same idea, but only for the topics that belong to the chosen subject. */
int collectTopics(char subject[], char list[][MAX_LABEL])
{
    int n = 0;
    int i, j, found;

    for (i = 0; i < qCount && n < MAX_TOPICS; i++)
    {
        if (strcmp(questions[i].subject, subject) != 0)
        {
            continue;                       /* different subject: skip it */
        }

        found = 0;
        for (j = 0; j < n; j++)
        {
            if (strcmp(list[j], questions[i].topic) == 0)
            {
                found = 1;
            }
        }
        if (found == 0)
        {
            strcpy(list[n], questions[i].topic);
            n++;
        }
    }
    return n;
}

/*
   buildPool
   Looks at every question and keeps the NUMBER (index) of each question
   that matches the student's choice (subject, topic, difficulty).
   Returns how many questions matched.
*/
int buildPool()
{
    int n = 0;
    int i;

    for (i = 0; i < qCount; i++)
    {
        if (strcmp(questions[i].subject, selSubject) != 0)
        {
            continue;
        }
        /* selTopic is empty ("") when the student chose "All topics" */
        if (strlen(selTopic) > 0 && strcmp(questions[i].topic, selTopic) != 0)
        {
            continue;
        }
        /* selDifficulty is 0 when the student chose "Mixed" */
        if (selDifficulty != 0 && questions[i].difficulty != selDifficulty)
        {
            continue;
        }

        pool[n] = i;
        n++;
    }
    return n;
}

/*
   prepareSession
   Lets the student choose subject, topic and difficulty, then builds the
   list of matching questions in random order.
   Returns the number of questions found (0 means nothing matched).
*/
int prepareSession()
{
    char subjects[MAX_TOPICS][MAX_LABEL];   /* array of strings */
    char topics[MAX_TOPICS][MAX_LABEL];
    int subjectCount, topicCount, choice, i;

    /* ---- choose subject ---- */
    subjectCount = collectSubjects(subjects);
    if (subjectCount == 0)
    {
        printf("No questions available.\n");
        return 0;
    }

    printf("\nSubjects:\n");
    for (i = 0; i < subjectCount; i++)
    {
        printf("  %d. %s\n", i + 1, subjects[i]);
    }
    choice = readNumber("Choose subject: ", 1, subjectCount);
    strcpy(selSubject, subjects[choice - 1]);

    /* ---- choose topic ---- */
    topicCount = collectTopics(selSubject, topics);

    printf("\nTopics in %s:\n", selSubject);
    for (i = 0; i < topicCount; i++)
    {
        printf("  %d. %s\n", i + 1, topics[i]);
    }
    printf("  %d. All topics\n", topicCount + 1);

    choice = readNumber("Choose topic: ", 1, topicCount + 1);
    if (choice == topicCount + 1)
    {
        selTopic[0] = '\0';                 /* empty text = all topics */
    }
    else
    {
        strcpy(selTopic, topics[choice - 1]);
    }

    /* ---- choose difficulty ---- */
    printf("\nDifficulty:\n");
    printf("  1. Easy\n  2. Medium\n  3. Hard\n  4. Mixed\n");
    choice = readNumber("Choose difficulty: ", 1, 4);
    if (choice == 4)
    {
        selDifficulty = 0;                  /* 0 = mixed */
    }
    else
    {
        selDifficulty = choice;
    }

    /* ---- find the matching questions and mix them ---- */
    poolCount = buildPool();
    if (poolCount == 0)
    {
        printf("\nNo questions match that choice. Try another difficulty/topic\n");
        printf("or add questions from the main menu.\n");
        return 0;
    }

    shuffle(pool, poolCount);               /* random question order */
    return poolCount;
}


/* ==========================================================================
   PART 9 : ASKING ONE QUESTION
   ========================================================================== */

/*
   askQuestion
   Shows one question, reads the answer and checks it.
     showFeedback = 1 : tell the student right away if correct (practice)
     allowQuit    = 1 : student may type Q to stop (practice)
   Returns:  1 = correct,   0 = wrong or skipped,   -1 = student quit
*/
int askQuestion(struct Question q, int number, int total, int showFeedback, int allowQuit)
{
    char input[50];
    char letter;
    int answer = 0;         /* 1..4 for A..D, 0 for skipped */
    int i;

    /* ---- show the question ---- */
    printf("\n");
    drawLine();
    printf("Question %d/%d   [%s | %s]\n", number, total, q.topic, q.subject);
    printf("%s\n", q.text);
    for (i = 0; i < 4; i++)
    {
        printf("  %c) %s\n", 'A' + i, q.option[i]);
    }

    /* ---- read a valid answer ---- */
    while (1)
    {
        if (allowQuit == 1)
            readText("Your answer (A-D, S=skip, Q=quit): ", input, 50);
        else
            readText("Your answer (A-D, S=skip): ", input, 50);

        letter = toupper(input[0]);         /* 'a' becomes 'A' */

        /* the input must be exactly ONE character */
        if (strlen(input) == 1 && letter >= 'A' && letter <= 'D')
        {
            answer = letter - 'A' + 1;      /* A->1, B->2, C->3, D->4 */
            break;
        }
        else if (strlen(input) == 1 && letter == 'S')
        {
            answer = 0;                     /* skipped */
            break;
        }
        else if (strlen(input) == 1 && letter == 'Q' && allowQuit == 1)
        {
            return -1;                      /* tell the caller: student quit */
        }
        printf("  Invalid input.\n");
    }

    /* ---- show feedback only in practice mode ---- */
    if (showFeedback == 1)
    {
        if (answer == q.correct)
        {
            printf("  Correct!\n");
        }
        else
        {
            if (answer == 0)
                printf("  Skipped. ");
            else
                printf("  Wrong. ");
            printf("Correct answer: %c) %s\n", 'A' + q.correct - 1, q.option[q.correct - 1]);
        }
    }

    if (answer == q.correct)
        return 1;
    else
        return 0;
}


/* ==========================================================================
   PART 10 : PRACTICE MODE
   ========================================================================== */

void practiceMode()
{
    int n, i, result;
    int answered = 0;
    int right = 0;

    printf("\n=== Practice Mode ===\n");

    n = prepareSession();
    if (n == 0)
    {
        return;                             /* nothing to practice */
    }

    for (i = 0; i < n; i++)
    {
        /* pool[i] is the number of the i-th (randomly ordered) question */
        result = askQuestion(questions[pool[i]], i + 1, n, 1, 1);

        if (result == -1)
        {
            break;                          /* student typed Q */
        }
        answered++;
        right = right + result;             /* result is 1 if correct */
    }

    printf("\nPractice finished: %d correct out of %d answered.\n", right, answered);
}


/* ==========================================================================
   PART 11 : QUIZ MODE
   ========================================================================== */

/* Gives a short comment according to the percentage. */
void getRemark(double percent, char remark[])
{
    if (percent >= 80)
        strcpy(remark, "Excellent work!");
    else if (percent >= 60)
        strcpy(remark, "Good job. Keep practicing!");
    else if (percent >= 40)
        strcpy(remark, "Fair. More practice will help.");
    else
        strcpy(remark, "Needs improvement. Review the weak topics.");
}

void quizMode()
{
    int n, total, i, j, result, found;
    int score = 0;                          /* number of correct answers */
    double percent;
    char diffName[10];
    char remark[60];
    char when[30];
    FILE *f;

    /* These arrays remember the marks of each topic in THIS quiz */
    char tName[MAX_TOPICS][MAX_LABEL];      /* topic names               */
    int  tAttempted[MAX_TOPICS];            /* questions asked per topic */
    int  tCorrect[MAX_TOPICS];              /* correct answers per topic */
    int  tCount = 0;                        /* number of topics so far   */

    printf("\n=== Quiz Mode ===\n");

    n = prepareSession();
    if (n == 0)
    {
        return;
    }

    printf("\nThere are %d matching questions.\n", n);
    total = readNumber("How many questions do you want in the quiz? ", 1, n);

    /* ---- ask the questions (no feedback during the quiz) ---- */
    for (i = 0; i < total; i++)
    {
        result = askQuestion(questions[pool[i]], i + 1, total, 0, 0);
        score = score + result;

        /* find this question's topic in our topic list */
        found = -1;
        for (j = 0; j < tCount; j++)
        {
            if (strcmp(tName[j], questions[pool[i]].topic) == 0)
            {
                found = j;
            }
        }
        if (found == -1)                    /* new topic: add it to the list */
        {
            found = tCount;
            strcpy(tName[found], questions[pool[i]].topic);
            tAttempted[found] = 0;
            tCorrect[found] = 0;
            tCount++;
        }
        tAttempted[found]++;
        tCorrect[found] = tCorrect[found] + result;
    }

    /* ---- calculate percentage ---- */
    percent = (score * 100.0) / total;      /* 100.0 makes it a decimal division */
    getRemark(percent, remark);
    getDifficultyName(selDifficulty, diffName);

    /* ---- show the result ---- */
    printf("\n");
    drawLine();
    printf("QUIZ RESULT - %s (%s)\n", current.name, current.id);
    drawLine();
    printf("Subject    : %s\n", selSubject);
    if (strlen(selTopic) > 0)
        printf("Topic      : %s\n", selTopic);
    else
        printf("Topic      : All topics\n");
    printf("Difficulty : %s\n", diffName);
    printf("Score      : %d / %d\n", score, total);
    printf("Percentage : %.2f%%\n", percent);   /* %% prints a % sign */
    printf("Remark     : %s\n", remark);

    printf("\nTopic breakdown:\n");
    for (j = 0; j < tCount; j++)
    {
        printf("  %-20s %d/%d (%.0f%%)\n", tName[j], tCorrect[j], tAttempted[j],
               (tCorrect[j] * 100.0) / tAttempted[j]);
    }

    /* ---- save the quiz result to results.txt (9 lines) ---- */
    getCurrentDate(when);

    f = fopen(R_FILE, "a");
    if (f != NULL)
    {
        fprintf(f, "%s\n", current.id);
        fprintf(f, "%s\n", current.name);
        fprintf(f, "%s\n", selSubject);
        if (strlen(selTopic) > 0)
            fprintf(f, "%s\n", selTopic);
        else
            fprintf(f, "All topics\n");
        fprintf(f, "%s\n", diffName);
        fprintf(f, "%d\n", total);
        fprintf(f, "%d\n", score);
        fprintf(f, "%.2f\n", percent);
        fprintf(f, "%s\n", when);
        fclose(f);
    }

    /* ---- save topic marks to topicstats.txt (5 lines per topic) ---- */
    f = fopen(T_FILE, "a");
    if (f != NULL)
    {
        for (j = 0; j < tCount; j++)
        {
            fprintf(f, "%s\n", current.id);
            fprintf(f, "%s\n", selSubject);
            fprintf(f, "%s\n", tName[j]);
            fprintf(f, "%d\n", tAttempted[j]);
            fprintf(f, "%d\n", tCorrect[j]);
        }
        fclose(f);
    }

    printf("\nResult saved.\n");
}


/* ==========================================================================
   PART 12 : PERFORMANCE ANALYSIS  (strong and weak topics)
   Idea: read all saved topic marks of the current student, add the marks of
   the same topic together, then calculate the percentage of each topic.
   ========================================================================== */

void performanceAnalysis()
{
    struct TopicStat stats[MAX_STATS];      /* array of structures */
    int count = 0;
    int i, found, attempted, correct, best, worst;
    char id[MAX_ID], subject[MAX_LABEL], topic[MAX_LABEL], line[300];
    double percent, bestPercent, worstPercent;
    char level[10];
    FILE *f = fopen(T_FILE, "r");

    if (f == NULL)
    {
        printf("\nNo quiz data yet. Take a quiz first!\n");
        return;
    }

    /* ---- read the file: 5 lines make one record ---- */
    while (readFileLine(f, id, MAX_ID))
    {
        readFileLine(f, subject, MAX_LABEL);
        readFileLine(f, topic, MAX_LABEL);
        readFileLine(f, line, 300);
        attempted = atoi(line);
        readFileLine(f, line, 300);
        correct = atoi(line);

        if (strcmp(id, current.id) != 0)
        {
            continue;                       /* record of another student */
        }

        /* is this topic already in our stats array? */
        found = -1;
        for (i = 0; i < count; i++)
        {
            if (strcmp(stats[i].subject, subject) == 0 &&
                strcmp(stats[i].topic, topic) == 0)
            {
                found = i;
            }
        }

        if (found == -1)                    /* new topic: create a record */
        {
            if (count >= MAX_STATS)
            {
                continue;
            }
            found = count;
            strcpy(stats[found].subject, subject);
            strcpy(stats[found].topic, topic);
            stats[found].attempted = 0;
            stats[found].correct = 0;
            count++;
        }

        /* add the marks of this quiz to the topic total */
        stats[found].attempted = stats[found].attempted + attempted;
        stats[found].correct = stats[found].correct + correct;
    }
    fclose(f);

    if (count == 0)
    {
        printf("\nNo quiz data for you yet. Take a quiz first!\n");
        return;
    }

    /* ---- print the table ---- */
    printf("\n=== Topic-wise Performance: %s (%s) ===\n", current.name, current.id);
    printf("%-18s %-20s %-9s %-8s %s\n", "Subject", "Topic", "Correct", "Percent", "Level");
    drawLine();

    best = 0;                               /* index of the best topic  */
    worst = 0;                              /* index of the worst topic */
    bestPercent = 0;
    worstPercent = 100;

    for (i = 0; i < count; i++)
    {
        if (stats[i].attempted > 0)
            percent = (stats[i].correct * 100.0) / stats[i].attempted;
        else
            percent = 0;

        if (percent >= STRONG_LIMIT)
            strcpy(level, "STRONG");
        else if (percent < WEAK_LIMIT)
            strcpy(level, "WEAK");
        else
            strcpy(level, "AVERAGE");

        printf("%-18s %-20s %3d/%-5d %6.1f%%  %s\n",
               stats[i].subject, stats[i].topic,
               stats[i].correct, stats[i].attempted, percent, level);

        /* remember the best and the worst topic */
        if (percent > bestPercent || i == 0)
        {
            bestPercent = percent;
            best = i;
        }
        if (percent < worstPercent || i == 0)
        {
            worstPercent = percent;
            worst = i;
        }
    }
    drawLine();

    printf("Strongest topic : %s (%s) - %.1f%%\n",
           stats[best].topic, stats[best].subject, bestPercent);
    printf("Weakest topic   : %s (%s) - %.1f%%\n",
           stats[worst].topic, stats[worst].subject, worstPercent);
    printf("\nStrong >= %.0f%%,  Weak < %.0f%%.  Practice your weak topics more.\n",
           STRONG_LIMIT, WEAK_LIMIT);
}


/* ==========================================================================
   PART 13 : VIEW PAST RESULTS
   ========================================================================== */

void viewPastResults()
{
    char id[MAX_ID], name[MAX_NAME], subject[MAX_LABEL], topic[MAX_LABEL];
    char diff[10], when[30], line[300];
    int total, score;
    double percent;
    int found = 0;                          /* how many results we have shown */
    FILE *f = fopen(R_FILE, "r");

    if (f == NULL)
    {
        printf("\nNo results saved yet.\n");
        return;
    }

    /* ---- one result = 9 lines in the file ---- */
    while (readFileLine(f, id, MAX_ID))
    {
        readFileLine(f, name, MAX_NAME);
        readFileLine(f, subject, MAX_LABEL);
        readFileLine(f, topic, MAX_LABEL);
        readFileLine(f, diff, 10);
        readFileLine(f, line, 300);
        total = atoi(line);
        readFileLine(f, line, 300);
        score = atoi(line);
        readFileLine(f, line, 300);
        percent = atof(line);               /* text -> decimal number */
        readFileLine(f, when, 30);

        if (strcmp(id, current.id) != 0)
        {
            continue;                       /* result of another student */
        }

        if (found == 0)                     /* print the heading only once */
        {
            printf("\n=== Past Results: %s (%s) ===\n", current.name, current.id);
            printf("%-17s %-17s %-18s %-8s %-7s %s\n",
                   "Date", "Subject", "Topic", "Level", "Score", "Percent");
            drawLine();
        }
        found++;

        printf("%-17s %-17s %-18s %-8s %2d/%-4d %.1f%%\n",
               when, subject, topic, diff, score, total, percent);
    }
    fclose(f);

    if (found == 0)
    {
        printf("\nYou have no saved quiz results yet.\n");
    }
}


/* ==========================================================================
   PART 14 : MAIN MENU AND main()
   ========================================================================== */

void showMenu()
{
    printf("\n");
    printf("============================================================\n");
    printf("                       SMARTLEARN\n");
    printf("   Student: %s (%s)   Questions in bank: %d\n",
           current.name, current.id, qCount);
    printf("============================================================\n");
    printf("  1. Practice Mode\n");
    printf("  2. Quiz Mode\n");
    printf("  3. Performance Analysis (strong / weak topics)\n");
    printf("  4. View Past Results\n");
    printf("  5. Add a Question to the Bank\n");
    printf("  6. Switch Student\n");
    printf("  7. Exit\n");
}

/* Every C program starts running from main(). */
int main()
{
    int choice;
    int running = 1;                        /* 1 = keep running, 0 = stop */

    /* Give rand() a different starting point each run, otherwise the
       "random" order would be the same every time. */
    srand(time(NULL));

    printf("Welcome to SmartLearn - practice, quiz, and improve!\n");

    loadQuestions();                        /* read the question bank        */
    loginOrRegister();                      /* identify the student          */

    while (running == 1)                    /* main menu loop                */
    {
        showMenu();
        choice = readNumber("Enter your choice: ", 1, 7);

        switch (choice)
        {
            case 1: practiceMode();         break;
            case 2: quizMode();             break;
            case 3: performanceAnalysis();  break;
            case 4: viewPastResults();      break;
            case 5: addQuestion();          break;
            case 6: loginOrRegister();      break;
            case 7:
                printf("\nGoodbye, %s! Keep learning!\n", current.name);
                running = 0;                /* stops the while loop */
                break;
        }
    }

    return 0;
}
