FROM ubuntu:latest

RUN apt update && apt-get -y install git make clang llvm lld

RUN git clone https://github.com/SzilBalazs/WhiteCore.git

RUN cd WhiteCore && make EXE=WhiteCore

CMD [ "./WhiteCore/WhiteCore" ]