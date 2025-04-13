## Traceroute
Allows processing Internet Control Message Protocol (ICMP) packets, essential for traceroute operations.
Organized into separate files:
  - ICMP utilities (`icmp_utils.cpp` and `icmp_utils.h`),
  - traceroute-specific utilities (`traceroute_utils.cpp` and `traceroute_utils.h`),
  - main execution logic (`main.cpp`).

### Usage
To compile and run the traceroute tool:
1. Clone the repository:
   ```bash
   git clone https://github.com/czarekmilek/Traceroute.git
   cd Traceroute
   ```
2. Build the project using the provided Makefile:
   ```bash
   make
   ```
3. Execute the compiled traceroute program:
   ```bash
   ./traceroute <destination_host>
   ```

Replace `<destination_host>` with the domain name or IP address of the target host you wish to trace.

### Technical Details
- The tool operates by sending ICMP Echo Request packets with incrementally increasing Time-To-Live (TTL) values.
- Each router along the path decrements the TTL, and when it reaches zero, the router sends back an ICMP Time Exceeded message.
- By analyzing these responses, the tool maps out the route to the destination.
