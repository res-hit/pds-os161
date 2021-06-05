# Lab5

- In memory File Tables merged so that there is no separation between Per-Process-File-Table and System-Wide-File-Table
  - global *struct\* vnode* array which maps global(i.e. shared among processes) file descriptors  
  - uncomment spinlocks for safe access to *vn_countref*
- Further implmentation: 
  - separate descriptors and make them local for each process
  - check each process can open same file only one time(i.e. same thread of a process can not do that)
  - implement safe access(through internal locks) to same entry in system wide file table and check if there is any inheritance bind to let them access same entry
  - implement access to same entry in system wide file table in order to be thread safe (inside same process)
  - implement concurrent safe access (through internal locks) for threads accessing different system wide table entries, but referring to same *struct vnode* 
