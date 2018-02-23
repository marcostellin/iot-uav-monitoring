from subprocess import call

for i in range(0,6):
	
	command = "../../waf"
	arg = "--DataRate=" + str(i)
	tot = "\"experiment1 " + arg + "\""
	ns3call = call(command + " --run " + tot, shell="True")