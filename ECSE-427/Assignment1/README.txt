To run this program compile using 'make rpc'

You can then run ./backend <IP> <PORT> to create a server. If one of those arguments is not provided , IP = "0.0.0.0" and PORT = 10000 
You can then run ./frontend <IP> <PORT> to connect to a server. If one of those arguments is not provided , IP = "0.0.0.0" and PORT = 10000

All required funcitonality has been implemented.

Notes: 
    - exit keyword ends only the current fronend. shutdown keyword ends the backend. Quit keyword not implemented.
    - When a shutdown occurs, the next connection will close the server and the current connection gets processed. No more connections can be made.
    - The child processes already running before shutdown will be allowed to finish and once those processes finish then the backend will shut down
    on the next connection (which will be refused).
    - More than 5 frontends can connect to the backend however only 5 at a time will be able to send the shutdown command and have it work properly.

    - RPC_Call function was not implemented since the receive and send functions from the starter code performed what was required for the task.
    Since it was mentioned that we did not even have to implement all those functions since RPC is more of a concept than a standard implementation, 
    I expect that this will be fine.
