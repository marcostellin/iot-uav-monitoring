from subprocess import call

sfactors = range(0,6)
period = [10,20,30,60,100]
nNodes = [10, 20, 30, 50, 100, 150, 200, 250, 300]

for sf in sfactors:
	for num in nNodes:
		for t in period:
			command = "../../waf"
			arg = "--DataRate=" + str(sf) + " " + "--nNodes=" + str(num) + " " + "--Interval=" + str(t)
			tot = "\"experiment2 " + arg + "\""
			ns3call = call(command + " --run " + tot, shell="True")
	
	