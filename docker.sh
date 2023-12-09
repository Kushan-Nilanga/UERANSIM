docker build -t dknathalage/ueransim-base -t ueransim-base docker/ueransim/ueransim-base
docker push dknathalage/ueransim-base

docker build -t dknathalage/ueransim-gnb:new -t ueransim-gnb docker/ueransim/ueransim-gnb
docker push dknathalage/ueransim-gnb:new

# docker build --no-cache -t dknathalage/ueransim-ue -t ueransim-ue docker/ueransim/ueransim-ue
# docker push dknathalage/ueransim-ue 