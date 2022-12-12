rmdir /q /s build
cpplint --linelength=120 --filter=-whitespace/tab --recursive .\src\
west build -b particle_xenon -- -D OVERLAY_CONFIG="overlay-ot.conf"
for /f %%i in ('git rev-parse --short^=8 HEAD') do set GITHASH=%%i
@REM scp build\zephyr\zephyr.signed.bin pi@threadrouter.local:\var\otafw\10102-%GITHASH%-zephyr.bin
west flash --hex-file bootloader\mcuboot.hex -r nrfjprog
west flash -r nrfjprog
