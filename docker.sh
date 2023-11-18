docker build -t dknathalage/ueransim-base -t ueransim-base docker/ueransim/ueransim-base
docker build -t dknathalage/ueransim-gnb -t ueransim-gnb docker/ueransim/ueransim-gnb
docker build -t dknathalage/ueransim-ue -t ueransim-ue docker/ueransim/ueransim-ue

docker push dknathalage/ueransim-base
docker push dknathalage/ueransim-gnb
docker push dknathalage/ueransim-ue 