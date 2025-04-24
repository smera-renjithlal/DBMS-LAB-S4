# DBMS-LAB-S4
This project is done as a part of the DBMS LAB course by S4 students.

# RESOURCES
https://nitcbase.github.io/

# WHAT IS NITCBASE
NITCbase is a Relational Database Management System (RDBMS) implementation project that is intended to help undergraduate students understand the design and data structures of an elementary RDBMS. This is done by implementing an RDBMS themselves with a step-by-step implementation roadmap.

# WHAT I LEARNT
* How relational databases are constructed from scratch
* Fundamentals of data storage, indexing, and query handling
* How to debug low-level memory and logic issues

# UTILITIES AND TOOLS
* Fedora - primary OS during development; essential for system-level operations
* VS Code – for writing and organizing code
* Git – for version control and stage-wise tracking
* GitHub – for backing up code and sharing progress
* GDB – for debugging logic-level issue
* Valgrind – for detecting memory leaks and segmentation faults
* Linux Terminal – essential for compiling, testing, and disk operations

# SUGGESTIONS
If you are planning to choose DBMS Lab for your elective, I highly suggest going through the documentation and getting a rough idea before you start. 
* Familiarize yourself with *git*.
* Install *Linux*, and learn how to work with it.
* Make sure you aren't trying to just memorize information. Genuinely read, picture and understand the model.
* Before starting any stage go through the design of the layer and how the code fits into it. This will give you a better idea of what to write.
* Backup. Your. Code. I kept getting stuck with bugs and would have been in a world of trouble if I hadn't pushed each of my stages into my repo once I was done with it. 
* Be consistent. It is very easy to fall behind since there are no strict deadlines for completing stages. Make sure you code everyday and keep yourself in the loop with your progress. This will also help you pace yourself and give you more time to debug when you face errors - you will need it. 
* Write your own code. Don't just copy code off of others. Genuinely sit down and write your programs with understanding. Only refer to others codes or repos when you face a roadblock.

# DEBUGGING
I used *GDB* and *Valgrind* to debug:
  - GDB for general logic issues
  - Valgrind for memory-related bugs and segmentation faults
* If you get a segmentation fault, run your program with Valgrind. It'll usually point you to what needs fixing.
* At one point, my disk got corrupted and started printing weird symbols in the XFS interface.
  - If this happens to you, do not panic. Use the 'fdisk' command in the XFS Interface to format the disk. It’ll be as good as new. You can implement fixes and test them after.
* If you get stuck and nothing works - start from scratch. Go to the previous stage and redo the whole thing.

# FINAL THOUGHTS
This course was intense, but also a massive learning opportunity. Stick with it, take breaks when you're stuck, and always push your work as you go. You'll come out of it with a deeper understanding of databases, coding discipline, and maybe even a bit of pride.
Good Luck!
