#!/usr/bin/python
import os

#execute stuff from bash
#	>pipe output to stdout
#handle stdin/stdout from python
#	>get output of arbitrary from stdin
#handle blowfish 
#make a simple shell for testing

def powerslaves_arbitrary_read(cmd, cmd_len):
	"cmd is a hexidecimal string starting with 0x, len is an \
	 integer < 4096 (multiples of 4 plz)"
	base_command = "sudo ./arbitrary "
	constr_command = base_command + cmd[2:18] + " " + str(cmd_len)
	stdout_data = os.popen(constr_command).read()
	return stdout_data

def main_loop():

	cmd = "0x0000000000000000"
	cmd_len	= 1000
	arb_ret = powerslaves_arbitrary_read(cmd, cmd_len)
	print arb_ret
	return 


if __name__ == "__main__":
	main_loop()

