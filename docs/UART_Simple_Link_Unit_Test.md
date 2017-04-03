## UART and Simple Link protocol UNIT Test

This chapter aims to test the UART functionality not as a system, but as check the UART is correctly receiving/answering commands

To perform the test `socat` utility is used

```
socat -d -d pty,raw,echo=0 pty,raw,echo=0
```

This call outputs:
```
2017/03/31 10:56:50 socat[29328] N PTY is /dev/pts/22
2017/03/31 10:56:50 socat[29328] N PTY is /dev/pts/23
2017/03/31 10:56:50 socat[29328] N starting data transfer loop with FDs [5,5] and [7,7]
```

The UART program must be connected to one PTY (ex. /dev/pts/22) and the __dummy_uart__ to the other (ex. /dev/pty/23)

__dummy_uart__ connects to the PTY and sends a packets in different configurations:

Configuration 1:

 	* 50% prob. to send a packet with flipped bytes (CRC test)
	* 50% prob. to send a correct packet

For configuration 1 no special requirements have to be met, just the timeout value of the API can be set to 1 or whatever

Configuration 2:

	* 25% prob. to send a corrupted packet with BAD LENGTH BYTE (random length)
	* 25% prob. to send a shorter packet then the indicated by the length byte
	* 25% prob. to send a packet with flipped bytes (CRC test)
	* 25% prob. to send a correct packet

For configuration 2 the timeout must be set in accordance with the packet/rate, which is by default 1 packet every 20 milliseconds, thus the timeout must be a lower value (10ms for instance). In case of not respecting this statement, the API cannot handle erasures bytes in the middle of a packet. You can execute this putting bad timeouts to see the API behaviour.

Note that the Transmitter UART module answers every OK packet with a response, thus the __dummy_uart__ can count for good sent/good received packets.

