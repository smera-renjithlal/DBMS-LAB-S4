# NITCbase
NITCbase is a rudimentary RDBMS implementation project done by S4 students as a part of the DBMS LAB course. <br>
It is structured around an eight-layer architecture that provides basic functionality for handling standard relational queries and includes indexing through B+ trees. By the end of the project, the system supports elementary SQL operations such as CREATE, DROP, ALTER, INSERT, SELECT, PROJECT, and EQUI-JOIN. (Note that NITCbase does not support concurrency.)

# Resources
https://nitcbase.github.io/
<br><br>

NITCbase is divided into 12 stages:<br>
[Installation](https://nitcbase.github.io/docs/Roadmap/Stage00)<br>
[Stage 1](https://nitcbase.github.io/docs/Roadmap/Stage01)<br>
[Stage 2](https://nitcbase.github.io/docs/Roadmap/Stage02)<br>
[Stage 3](https://nitcbase.github.io/docs/Roadmap/Stage03)<br>
[Stage 4](https://nitcbase.github.io/docs/Roadmap/Stage04)<br>
[Stage 5](https://nitcbase.github.io/docs/Roadmap/Stage05)<br>
[Stage 6](https://nitcbase.github.io/docs/Roadmap/Stage06)<br>
[Stage 7](https://nitcbase.github.io/docs/Roadmap/Stage07)<br>
[Stage 8](https://nitcbase.github.io/docs/Roadmap/Stage08)<br>
[Stage 9](https://nitcbase.github.io/docs/Roadmap/Stage09)<br>
[Stage 10](https://nitcbase.github.io/docs/Roadmap/Stage10)<br>
[Stage 11](https://nitcbase.github.io/docs/Roadmap/Stage11)<br>
[Stage 12](https://nitcbase.github.io/docs/Roadmap/Stage12)<br>


# Learning
* Understand the design and data structures of an elementary RDBMS through implementation
* Fundamentals of data storage, indexing, and query handling
* How to debug low-level memory and logic issues


# Utilities and Tools used
* Fedora - primary OS during development; essential for system-level operations
* C/C++ compiler (gcc/g++)
* VS Code – for writing and organizing code
* Git – for version control and stage-wise tracking
* GitHub – for backing up code and sharing progress
* GDB – for debugging logic-level issue
* Valgrind – for detecting memory leaks and segmentation faults
* Linux Terminal – essential for compiling, testing, and disk operations


# Suggestions
If you are planning to choose DBMS Lab for your elective, I highly suggest going through the documentation and getting a rough idea before you start. 
* Familiarize yourself with *git*.
* Install *Linux*, and learn how to work with it.
* Make sure you aren't trying to just memorize information. Genuinely read, picture and understand the model.
* Before starting any stage go through the design of the layer and how the code fits into it. This will give you a better idea of what to write.
* Backup. Your. Code. I kept getting stuck with bugs and would have been in a world of trouble if I hadn't pushed each of my stages into my repo once I was done with it. 
* Be consistent. It is very easy to fall behind since there are no strict deadlines for completing stages. Make sure you code everyday and keep yourself in the loop with your progress. This will also help you pace yourself and give you more time to debug when you face errors - you will need it. 
* Write your own code. Don't just copy code off of others. Genuinely sit down and write your programs with understanding. Only refer to others codes or repos when you face a roadblock.


# Debugging
I used *GDB* and *Valgrind* to debug [Documentation for GDB](https://nitcbase.github.io/docs/Misc/GDB):
  - GDB for general logic issues
  - Valgrind for memory-related bugs and segmentation faults
* If you get a segmentation fault, run your program with Valgrind. It'll usually point you to what needs fixing.
* At one point, my disk got corrupted and started printing weird symbols in the XFS interface.
  - If this happens to you, do not panic. Use the 'fdisk' command in the XFS Interface to format the disk. It’ll be as good as new. You can implement fixes and test them after.
* If you get stuck and nothing works - start from scratch. Go to the previous stage and redo the whole thing.


# Final Thoughts
This course was intense, but also a massive learning opportunity. Stick with it, take breaks when you're stuck, and always push your work as you go. You'll come out of it with a deeper understanding of databases, coding discipline, and maybe even a bit of pride.

Good Luck!
