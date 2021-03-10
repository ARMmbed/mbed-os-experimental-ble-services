FROM mbedos/mbed-os-env:latest
RUN apt-get update && apt-get install -y valgrind gcovr
RUN pip3 install virtualenv
ENTRYPOINT ["tests/unittests.sh"]
