# itc-platform
## Fundamental knowledge:
### 1. General architecture:
 
```
This is a portable intercommunication platform built as a dynamic shared library, that allows exchanging user's messages between threads, processes and even different boards.
 
Each board is one instance of a single OS and may have several processes.
 
Each thread inside a process must create a mailbox (maybe only one per thread supported for now) with a unique mailbox id across the entire universe (your whole system including all boards, devices,...).
 
To be consistent, throughout this project we will define:
	+ Thread scope: a thread inside a process: [Unit]
	+ Process scope: all threads inside a process: [Region]
	+ Host scope: all processes inside a host: [World]
	+ Network scope: all hosts inside a private network: [Universe]
 
To communicate between Worlds, we need an itc-gateway which is responsible for sending/receiving messages to/from other Worlds, helping a mailbox locate another one in other Worlds. This itc-gateway will translate itc messaging protocol to Ethernet protocol and vice versa, so that's why it's named as a "gateway". This itc-gateway is only enabled if any newly created mailbox desires to have external communication.

To coordinate all Regions within a World, we also need an itc-manager which provides, assigns "mailbox_id" to newly spawned mailboxes, and manages their identity in a hash table and support a mailbox locating another one within a World.

Both itc-gateway and itc-manager are submodules of itc-server which starts once via the first World that calls ITC system initialization API, and itc-server will run as a daemon under background.

To distinguish mailboxes in a same World, we will need mailbox_id which is managed, distributed and assigned by itc-manager to newly spawned mailboxes right after they notify itc-server on their birth and demise.
This mailbox_id is an uint32_t with format:
+ 16 lowest bits: is used to differentiate mailboxes inside a Region.
+ 12 highest bits: is used to differentiate Regions inside a World.
+ 4 gap bits (in between): reserved.
For example:
+ mailbox_id: 0x00300005
Where:
	+ region_id:	3 = 0x"003"00005
	+ reserved:   	0 = 0x003"0"0005
	+ unit_id:  	5 = 0x0030"0005"

Since multiple mailboxes in different Worlds may have exactly a same mailbox_id, so how to differentiate between them?
We will definitely need world_id which is maintained by itc-gateway and only meaningful once itc messages go outside Worlds.
This world_id is an uint32_t which is a value obtained from inet_addr("static_ip_address_string"). Since every World in Universe (device in your Ethernet-connected system) has an unique static ip address which is provided and assigned by DHCP server, we should utilise it for mailbox identification in different Worlds.
For example:
+ world_id: 0x0A01A8C0 = inet_addr("192.168.1.10")

To locate a particular mailbox, the current mailbox has to give ITC system following input:
	+ Target mailbox name: <mailbox_name>
	+ A hint: is a mailbox ID mask (3 OR bit wide), with each bit indicating where ITC system should look for the target. For example, one bit for searching outside current host, one bit for all processes inside current host, and the other for all threads in current process. If user has no idea about where to find, that means bit mask is 111, then ITC system will try to find the first found mailbox in order of Region scope -> World scope -> Universe scope.

```
 