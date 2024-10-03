EnsembleClassifier.out: LinearClassifier.out Voter.out ensembleClassifier.cpp
	g++ ensembleClassifier.cpp -o EnsembleClassifier.out

LinearClassifier.out: linearClassifier.cpp
	g++ linearClassifier.cpp -o LinearClassifier.out
	
Voter.out: voter.cpp
	g++ voter.cpp -o Voter.out

clean: 
	rm *.out 
	rm -r Assets/named_pipes/
	mkdir Assets/named_pipes