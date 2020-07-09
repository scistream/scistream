# SciStream

software components for SciStream project 

# Software components

* SciStream Data Server (S2DS): software that runs on gateway nodes. It acts as a buffer-and-forward agent.
* SciStream User Client (S2UC): software that the end user and/or workflow engines/tools acting on behalf of the user interact with and provide relevant information (e.g., ID of a HPC job, ID of an experiment or data acquisition job on a scientific instrument, shared secret for secure communication with the user job (application) at the producer and consumer) to orchestrate end-to-end data streaming.
* SciStream Control Server (S2CS): a software running on one of the gateway nodes. It interacts with S2UC, data producer / consumer and S2DS (see below).

# A demo

compile S2DS with gcc, then run it like:

`./S2DS.out --remote-port=50000 --local-port=50001 --remote-host=192.168.0.143 --log`

* `remote-host` is the data producer's IP and `remote-port` is producer's port that it is listening on, S2DS will connect to `remote-host:remote-port`. 

* `local-port` is a port that S2DS will listen on and consumer will connect to it (instead directly to the producer) to get data. 

* Assume that the hostname of the machine for S2DS is `s2ds.local`, then data consumer need to connect to `s2ds.local:local-port`. 

* I added some logs print, so `--log` will enable printing whatever S2DS has forwarded.

* Then start both `pub.py` and `sub.py` under zeromq, the order doesnot matter. you should 

* Note that, so far, a run instance of S2DS is for one pair of producer and consumer, it exits when consumer disconnet.

# Notes

[07-09-2020] the current S2DS code was modified from an opensource project `git@github.com:vakuum/tcptunnel.git`. we may need to carefully understand its implementation, if we find it useful, we can clean up unnecesssary components. otherwise, we can remove it and start from scratch

# ToDo 

