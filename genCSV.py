data = "sample data/retail.txt"
output = "tsv/retail.tsv"

f = open(data, 'r')
xactcount = f.readline()

g = open(output, 'w')
g.write(xactcount)
for line in f:
	vals = line.split()
	transaction = vals[2:]
	outline = "\t".join(transaction)+"\n";
	print outline
	g.write(outline)

g.close()
f.close()
