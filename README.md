Zephyr code and board config for the leucht-ohrringe board.
---
Docker ncs commands
- menuconfig
  ``` powershell
  docker run --rm -it -v "${PWD}:/workspace/app" ncs-ci:v3.4.2-r2 -c "cd /opt/ncs && west build --build-dir /workspace/app/build-local -t menuconfig"
  ```
- build
  ``` powershell
  docker run --rm -it  -v "${PWD}:/workspace/app" ncs-ci:v3.4.2-r2 -c "rm -rf /workspace/app/build-local && cd /opt/ncs && west build --build-dir /workspace/app/build-local -b leucht-ohrringe/nrf52832 /workspace/app -- -DBOARD_ROOT=/workspace/app"
  ```
---
Manual ncs command
``` powershell
$env:PATH='C:\ncs\toolchains\5f5ef0cea0\opt\bin;' + $env:PATH; cmake --build build --parallel 4
```