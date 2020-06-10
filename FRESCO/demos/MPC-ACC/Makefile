PROJECT=fresco-demo-mpc_acc

build: install move

build_fast: install_fast move

install_fast:
	mvn clean install -DskipTests -U

install:
	cd ../../core && mvn clean install -DskipTests -U
	cd ../../tools && mvn clean install -DskipTests -U
	cd ../../suite/spdz && mvn clean install -DskipTests -U
	cd ../common && mvn clean install -DskipTests -U
	mvn clean install -DskipTests -U

move:
	mkdir -p server1;
	mkdir -p server2;
	cp target/$(PROJECT).jar server1;
	cp target/$(PROJECT).jar server2;

clean:
	rm -rf server1
	rm -rf server2
	rm -rf target

# dummy won't work because ECCELERATE fails to decode neutralelement
run_dummy:
	cd server1 && java -jar $(PROJECT).jar -i 1 -p 1:localhost:8081 -p 2:localhost:8082 -Dspdz.preprocessingStrategy=DUMMY > log.txt 2>&1 &
	cd server2 && java -jar $(PROJECT).jar -i 2 -p 1:localhost:8081 -p 2:localhost:8082 -Dspdz.preprocessingStrategy=DUMMY 2>&1 | tee log.txt


run:
	cd server1 && java -jar $(PROJECT).jar -n 10 -i 1 -p 1:localhost:8081 -p 2:localhost:8082 -Dspdz.preprocessingStrategy=MASCOT > log.txt 2>&1 &
	cd server2 && java -jar $(PROJECT).jar -n 10 -i 2 -p 1:localhost:8081 -p 2:localhost:8082 -Dspdz.preprocessingStrategy=MASCOT 2>&1 | tee log.txt

.PHONY: clean run run_dummy
