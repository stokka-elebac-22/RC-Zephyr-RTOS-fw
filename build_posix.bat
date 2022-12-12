sudo apt install linux-libc-dev:i386
# sudo ~/zephyrproject/tools/net-tools/net-setup.sh -i zeth.1
git pull
west build -b native_posix
./build/zephyr/zephyr.exe --stop_at=30&
