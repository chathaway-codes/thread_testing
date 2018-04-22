FROM ubuntu

RUN apt-get update
RUN apt-get install -y build-essential

ADD . /work
WORKDIR /work
RUN make

CMD ["/work/3np1", "--threads", "1", "5"]
