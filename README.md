# RPC-Scratch
A remote procedure call implementation from scratch.

Implementation Details:
Have a is_prime function that we want to distribute across computers.
We need a caller stub to: pack arguments, trasnmit the argumentsm receive the result, unpack the result
We need a callee stub to: receive the arguments, unpack the arguments, call function, pack result, transmit result
