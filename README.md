# CEEPROM

---

![text](ceeprom.webp)

---

## PRESENTATION

ceeprom is binary programmer/patcher. It is basically a TL866II clone.

---

## PREREQUISITES

* C11 compiler

---

## SUPPORTED PLATFORMS

- [X] Windows
- [X] Linux
- [X] macOS

---

## HOW TO USE

```sh
make
# this program writes the program.list instructions
#into rom.bin (rom must exist) at the specified starting
# address (if not specified, starts at 0x0000)
./ceeprom -p program.list -r rom.bin -s 0x5000
```

---

