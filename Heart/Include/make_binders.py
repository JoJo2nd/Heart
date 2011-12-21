# python sure is better for this kind of stuff

versions=7

f=file("template.txt")
code=f.read()


# make the version version first

version=code
version=version.replace(",$class","")
version=version.replace("$class,","")
version=version.replace("$class","")
version=version.replace("$count","0")
version=version.replace("$params","")
version=version.replace(",$paramnames","")
version=version.replace("$paramnames,","")
version=version.replace("$paramnames","")
version=version.replace("$arguments","")
version=version.replace("template<>","");
version=version.replace("<>","");

print("/////////////////////")
print("// SqBind 0 arguments ")
print("/////////////////////")

print(version)

for n in range(1,versions):

	classes=""
	params=""
	paramnames=""
	arguments=""

	for i in range(1,n+1):
		cm=""
		pt=""
		if(i>1):
			cm=", "
			pt="\t"

		classes+=cm+"class P"+str(i)
		params+=pt+"SqCParam<P"+str(i)+"> p"+str(i)+";\n"
		paramnames+=cm+"P"+str(i)
		arguments+=cm+"p"+str(i)+".getter.get(v,"+str(i+1)+")"
		
	count=str(n)		

	version=code
	version=version.replace("$class",classes)
	version=version.replace("$count",count)
	version=version.replace("$params",params)
	version=version.replace("$paramnames",paramnames)
	version=version.replace("$arguments",arguments)

	print("/////////////////////")
	print("// SqBind "+str(i)+" arguments ")
	print("/////////////////////")

	print(version)

	
