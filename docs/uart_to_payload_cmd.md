## UART to PayloadManager command definition

UART Module must know the state of `PayloadManager` in any moment. For that purpose it will periodically polling it by means of a `TCP Socket`

### Commands
Three main commands are shared: 

* Start
* Get State
* Stop

Get State shall return any state fromn specified in states.md, and will produce a __flowgraph__ like:

0. Wait for Init (**W** state)
* `Start command issued by __UART__`
1. Initialisated (**I** state)
* `Running Experiment __G__`
2. G Experiment done (**G** state)
* `Running Experiment __O__`
3. O Experiment done (**O** state)
* `Running Experiment __A__`
4. A Experiment done (**A** state)
* `Stop command issued by __UART__`
5.  Finished (**F** state)
 
