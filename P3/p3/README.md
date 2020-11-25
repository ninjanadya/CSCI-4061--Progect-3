## Information

* The purpose of this program is to count each unique word in a text file.

* Compile the program using the make command as follows:
$ make

Then run:
$ ./web_server port path num_dispatch num_workers dynamic_flag qlen cache_entries



* The program accepts the input as mentioned above at compilation and runs as a multithreaded webserver, waiting for requests to continue running. Once a request is received and accepted, it is sent to a dispatcher which will queue up the request that will later be handled by the worker threads. All of this occuring will also update a log file of the actions taken.

* Assumptions were made solely based on the ones in the Project 3 document.

# Contributions
* Team met via Zoom and worked on the code via a shared github repository
* Coding, testing, and debugging were performed by Sean and Nadya
* CSE Lab Machine testing and README.md writen by Nadya and Maxim

* test machine : apollo.cselabs.umn.edu
* date : 11-25-2020
* name : Maxim Zabilo, Sean Berg, Nadya Postolaki
* x500 : zabil003, berg2007, posto018
