//Lecture notes Thu 31 August 2017

/******** STRUCTS *********/
//Typedef : If used, we don't have to specify that it's a struct 
//every time we declare a new one.
//Struct with int and a string
typedef struct data {
    int n;
    char str[4];
} Data;
Data dat;
Data* ptr = &dat;   //Points to beginning of the struct
                    //Use -> operator to get stuff from inside the structure

//Segmentation fault = Problems with pointers
//Declare ptr -> Declare Pointee -> Assign ptr to Pointee: ptr = &pointee


/******* RECURSION ******/
//Useful for writing concise programs
//Should be used for lab1!
//Base case first, then what the function should do


/******* LINKED LIST ******/
//Sequence of data structures linked by pointers
struct node {
    int n;
    struct node * next;
};
//Pointer in the last element is null
//Head - pointer to first, Tail - pointer to last
//Operations - Insert, Delete, Find, Sort



