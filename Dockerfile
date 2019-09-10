FROM ubuntu:18.04

RUN apt-get update

# Used by this Dockerfile itself
RUN apt-get install -y curl zip

# Required to run SES installer
RUN apt-get install -y libx11-6 libfreetype6 libxrender1 libfontconfig1 libxext6

WORKDIR /segger

RUN curl https://www.segger.com/downloads/embedded-studio/EmbeddedStudio_ARM_Linux_x64 -o ses.tar.gz

RUN tar -zxvf ses.tar.gz

RUN yes yes | $(find arm_segger_* -name "install_segger*") --copy-files-to /segger/ses

RUN curl https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v15.x.x/nRF5_SDK_15.3.0_59ac345.zip -o nRF5_SDK_15.3.0.zip && unzip nRF5_SDK_15.3.0.zip && mv nRF5_SDK_15.3.0_59ac345 nRF5_SDK_15.3.0 && rm nRF5_SDK_15.3.0.zip

CMD ["/segger/ses/bin/emBuild"]