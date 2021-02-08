FROM mbedos/mbed-os-env:latest
RUN apt-get update && apt-get install -y valgrind
ENTRYPOINT ["tests/unittests.sh"]
