# Virtual Memory Implementation

This project uses the [Nachos operating system](https://en.wikipedia.org/wiki/Not_Another_Completely_Heuristic_Operating_System) as a starting code base. This is specifically from the `code` directory within the Nachos directory. Due to its age and lack of surrounding code, this code is **unlikely to run if downloaded**. If you have a downloaded copy of Nachos running on an old installation of Ubuntu Linux, you can run the code by navigating to `code/` and running `make all` and running whichever file needed (most likely `threadtest.cc` in `code/threads`).

For the virtual memory project, the files we were concerned with were [`addrspace.cc`](userprog/addrspace.cc), `addrspace.h`, and `progtest.cc` in `userprog/`. We added one section in `userprog/exception.cc` to notify us of page faults.

```
├── ...
├── userprog
│   ├── addrspace.cc
│   ├── addrspace.h
│   ├── exception.cc
│   ├── progtest.cc
│   └── ...
└── ...
```