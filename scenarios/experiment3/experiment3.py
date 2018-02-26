from subprocess import call

sfactors = [0, 5]
period = [10,20,30]
nNodes = [10, 20, 30, 50, 100, 150, 200, 250, 300]
nGateways = [3, 5, 7, 10]

for sf in sfactors:
	for num in nNodes:
		for t in period:
			for nGw in nGateways:
				command = "../../waf"
				arg = "--DataRate=" + str(sf) + " " + "--nNodes=" + str(num) + " " + "--Interval=" + str(t) + " " + "--nGateways=" + str(nGw)
				tot = "\"experiment3 " + arg + "\""
				ns3call = call(command + " --run " + tot, shell="True")
