#!/bin/bash          
echo SR_ctl start script
echo SR_ctl hosts list:
strings SR_ctl_hosts.txt 
echo Starting master process number 1
 mpirun -np 1 --hostfile SR_ctl_hosts.txt --host localhost ./SR_ctl # Run master locally but with other hosts list