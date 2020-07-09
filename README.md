# SciStream

software components for SciStream project 

# Software components

* SciStream Data Server (S2DS): software that runs on gateway nodes. It acts as a buffer-and-forward agent.
* SciStream User Client (S2UC): software that the end user and/or workflow engines/tools acting on behalf of the user interact with and provide relevant information (e.g., ID of a HPC job, ID of an experiment or data acquisition job on a scientific instrument, shared secret for secure communication with the user job (application) at the producer and consumer) to orchestrate end-to-end data streaming.
* SciStream Control Server (S2CS): a software running on one of the gateway nodes. It interacts with S2UC, data producer / consumer and S2DS (see below). 