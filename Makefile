NAME ?= gpsr-leo
NS3_DOCKER_TAG ?= ns3.30-gpsr-leo
# --no-cache

build-step1:
	docker build \
		--no-cache \
		--tag="$(NS3_DOCKER_TAG)" \
		.

build-step2:
	docker run --name $(NAME) $(NS3_DOCKER_TAG)
	docker cp $(NAME):/usr/ns-allinone-3.30 src
	docker stop $(NAME) | xargs docker rm

start:
	docker run -ti --rm --privileged -v ./src/ns-allinone-3.30:/usr/ns-allinone-3.30 --network=host -p 8080:8080 --name $(NAME) $(NS3_DOCKER_TAG)




# NAME ?= leo-gpsr
# NS3_DOCKER_TAG ?= ns3.29
# # --no-cache

# build-step1:
# 	docker build \
# 		--no-cache \
# 		--tag="$(NS3_DOCKER_TAG)" \
# 		.

# build-step2:
# 	docker run --name $(NAME) $(NS3_DOCKER_TAG)
# 	docker cp $(NAME):/usr/ns3.29-with-gpsr src
# 	docker stop $(NAME) | xargs docker rm

# start:
# 	docker run -ti --rm --privileged -v ./src/ns3.29-with-gpsr:/usr/ns3.29-with-gpsr --network=host -p 8080:8080 --name $(NAME) $(NS3_DOCKER_TAG)