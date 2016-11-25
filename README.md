shmmap
=======

A key-value memory store. It used shared memory, so multiple processes can read while a process is writing to it. The performance of reading and writing is quite good, as all the operations are in the memory! So far，the bindings of nodejs, python and lua have been developed. And the nginx C module 'ngx_shmmap' has also been developed, which can make nginx reads from shmmap.
