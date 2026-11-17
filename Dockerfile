FROM ghcr.io/nrfconnect/sdk-nrf-toolchain:v3.4.2

USER root

SHELL ["/bin/bash", "-lc"]

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    nodejs \
    npm \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /opt/ncs \
    && west init \
    --manifest-url https://github.com/nrfconnect/sdk-nrf \
    --manifest-rev v3.4.2 \
    /opt/ncs \
    && cd /opt/ncs \
    && for attempt in 1 2 3 4 5; do \
    echo "=== west update attempt ${attempt}/5 ==="; \
    if west update; then \
    break; \
    fi; \
    if [ "$attempt" -eq 5 ]; then \
    echo "west update failed after 5 attempts"; \
    west -v update; \
    exit 1; \
    fi; \
    echo "west update failed; retrying in 15 seconds"; \
    sleep 15; \
    done \
    && west zephyr-export

WORKDIR /workspace

ENTRYPOINT ["/bin/bash"]